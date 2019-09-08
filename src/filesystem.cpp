#include "filesystem.h"
#include "filesystem\fileutil.h"
#include <algorithm>
#include <numeric>
using namespace ggtr;

// 初期化時にファイルデータベースを開いて中身のデータを抜いてくる
void FileSystem::_PreOpenDB()
{
	const char zero = 0;
	const char header[4] = { 'g', 'g', 't', 'r' };
	
	_offset = 0;

	if (!file::Exists(_dbpath.c_str()))
	{
		// ファイルが存在しない場合、新たにファイルを作成する
		_FileOpen("wb");

		fwrite(header, sizeof(char), 4, _fp);
		fwrite(&_offset, sizeof(int64_t), 1, _fp);
		fwrite(&zero, sizeof(char), _allocation, _fp);	// 領域を予約する

		_offset = 4 + sizeof(int64_t);
		_region = _ftelli64(_fp);
	}
	else
	{
		// ファイルが存在したらヘッダと容量を照合する
		_FileOpen("rb");

		char comparison[4];
		fread_s(comparison, 4, 1, 4, _fp);

		// ファイルヘッダの有効性チェック
		for (int i = 4; i < 4; ++i)
		{
			if (header[i] != comparison[i])
			{
				// 無効なファイルヘッダが来たので一旦ファイルを閉じてどうするか決める
				_FileClose();
				throw new InvalidFileHeaderException();
			}
		}

		fread_s(&_offset, sizeof(int64_t), sizeof(int64_t), 1, _fp);
		_region = _fseeki64(_fp, 0, SEEK_END);
	}

	// setvbufの判断を確実にするため、特に用がない限りはnullptrでゼロにする
	_FileClose();
}

FileSystem::FileSystem(const char * const dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr), _tempsize(0), _tempbuf(nullptr)
{
	_PreOpenDB();
}

FileSystem::FileSystem(const std::string & dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr), _tempsize(0), _tempbuf(nullptr)
{
	_PreOpenDB();
}

ggtr::FileSystem::~FileSystem()
{
	if (_buffer != nullptr)
		free(_buffer);
	if (_tempbuf != nullptr)
		free(_tempbuf);
}

void * ggtr::FileSystem::Query(FileInfo & info)
{
	if (file::Exists(_dbpath.c_str()))
	{
		// 開いて
		_FileOpen("rb");
		setvbuf(_fp, (char *)_buffer, _IOFBF, _bufsize);
		char * memory = (char*)malloc(info.size);	// FileBinary側でDispose呼び出さないと解放されない

		// 移動して、読み込む
		_fseeki64(_fp, info.offset, SEEK_SET);
		fread_s(memory, info.size, info.size, 1, _fp);

		_FileClose();

		//return FileBinary(memory, info.size, true);
		info.binary = memory;
		info.mother_binary = memory;
		return memory;
	}

	// ファイルが存在しない場合は例外を送る
	throw new DatabaseFileNotFoundException(_dbpath);
}

void * ggtr::FileSystem::Query(const FileInfo * const infos, const int64_t size)
{
	// 面倒なので内部的にstd::vector使おう
	std::vector<FileInfo> vinfos;
	vinfos.reserve(size);

	for (int i = 0; i < size; ++i)
		vinfos.push_back(infos[i]);

	return Query(vinfos);
}

bool FileSystem::_SortOffset(const FileInfo & a , const FileInfo & b)
{
	return a.offset < b.offset;
}

bool FileSystem::_SortId(const FileInfo & a, const FileInfo & b)
{
	return a.id < b.id;
}

void * ggtr::FileSystem::Query(std::vector<FileInfo>& infos)
{
	if (file::Exists(_dbpath.c_str()))
	{
		// infosをoffsetの昇順でソートする
		std::sort(infos.begin(), infos.end(), _SortOffset);

		// 合計サイズを求める
		int64_t total = 0;
		for (int i = 0; i < infos.size(); ++i)
			total += infos[i].size;

		// バッファの伸張
		for (int i = 0; i < infos.size(); ++i)
			_ExpandBuffer(infos[i].size);

		// ファイルを開く
		_FileOpen("rb");
		setvbuf(_fp, (char *)_buffer, _IOFBF, _bufsize);
		char * memory = (char *)malloc(total);	// この領域はFileBinaryListのDisposeが呼ばれない限り解放されない
		
		// 確保した領域にファイルを読み込んでいく
		int64_t count = 0;
		for (int i = 0; i < infos.size(); ++i)
		{
			_fseeki64(_fp, infos[i].offset, SEEK_SET);						// 移動して
			fread_s(&memory[count], total - count, infos[i].size, 1, _fp);	// 読み込む

			infos[i].id = i;
			infos[i].binary = &memory[count];
			infos[i].mother_binary = memory;

			count += infos[i].size;
		}
		
		_FileClose();

		// ファイルの順番でソート
		std::sort(infos.begin(), infos.end(), _SortId);

		return memory;
	}

	throw new DatabaseFileNotFoundException(_dbpath);
}

const FileInfo ggtr::FileSystem::Insert(const char * const binary, const int64_t size)
{
	FileInfo file;

	_FileOpen("ab");

	// 領域が不足しそうだったら追加で領域を確保する
	_ExpandRegion(size);
	_ExpandBuffer(size);

	// ファイルに書き込み
	_fseeki64(_fp, _offset, SEEK_SET);
	fwrite(binary, size, 1, _fp);

	file.offset = _offset;	// 位置をずらす前に記録
	file.size = size;
	_offset += size;		// サイズを変更

	_FileClose();

	return file;
}

std::vector<FileInfo> ggtr::FileSystem::Insert(const char ** const binaries, const int64_t * const sizes, const size_t numof_insertion)
{
	auto files = std::vector<FileInfo>();
	files.reserve(numof_insertion);

	// 合計サイズを求めつつ、ファイルの配列を用意する
	int64_t total = 0;
	for (size_t i = 0; i < numof_insertion; ++i)
	{
		files.emplace_back(i, _offset + total, sizes[i]);
		total += sizes[i];
	}

	// 領域が足りないなら確保する
	_ExpandTemp(total);
	_ExpandBuffer(total);
	_ExpandRegion(total);

	// バッファにデータを転送する
	int64_t offset = 0;

	#pragma omp parallel for reduction(+:offset)
	for (size_t i = 0; i < numof_insertion; ++i)
	{
		memcpy_s(&((char *)_tempbuf)[offset], total - offset, binaries[i], sizes[i]);
		offset += sizes[i];
	}
	_offset += total;

	// バッファからファイルへの書き込み
	_FileOpen("ab");
	setvbuf(_fp, (char *)_buffer, _IOFBF, total);
	_fseeki64(_fp, _offset, SEEK_SET);
	fwrite(_tempbuf, total, 1, _fp);

	_FileClose();

	return files;
}

void FileSystem::MoveDatabase(const std::string & todbpath)
{
	_MoveDatabase(todbpath.c_str());
}

void FileSystem::MoveDatabase(const char * const todbpath)
{
	_MoveDatabase(todbpath);
}

void FileSystem::_MoveDatabase(const char * const todbpath)
{
	if (file::Exists(todbpath))
	{
		// ファイルの移動先にファイルが存在していた場合は
		// 移動先のファイルに.bakをつける
		// .bakついたファイルがあったら、もうこれは削除と決める
		const auto str_dbpath = std::string(todbpath) + ".bak";
		if (file::Exists(str_dbpath.c_str()))
		{
			remove(str_dbpath.c_str());
		}

		file::Move(todbpath, str_dbpath.c_str());
	}

	// 移動させたのでパスを移し替える
	file::Move(_dbpath.c_str(), todbpath);
	_dbpath = std::string(todbpath);
}

void ggtr::FileSystem::_FileOpen(const char * const mode)
{
	errno_t error = fopen_s(&_fp, _dbpath.c_str(), "rb");
	if (error != 0)
		throw new OpenDatabaseException(error, "rb");
}

void ggtr::FileSystem::_FileClose()
{
	fclose(_fp);

	// ファイルのオフセットを書き込む
	_FileOpen("ab");
	_fseeki64(_fp, 4, SEEK_SET);
	fwrite(&_offset, sizeof(int64_t), 1, _fp);
	fclose(_fp);

	_fp = nullptr;
}

// 領域が不足していれば拡張する
void ggtr::FileSystem::_ExpandRegion(const int64_t size)
{
	while (_offset + size > _region)
	{
		const char zero = 0;
		_fseeki64(_fp, 0, SEEK_END);
		fwrite(&zero, sizeof(char), _allocation, _fp);
		_region += _allocation;
	}
}

// バッファの領域が不足しているなら領域を広げる
void ggtr::FileSystem::_ExpandBuffer(const int64_t size)
{
	if (_bufsize < size)
		_bufsize = _AllocateBuffer(_buffer, size);
}

void ggtr::FileSystem::_ExpandTemp(const int64_t size)
{
	if (_tempsize < size)
		_tempsize = _AllocateBuffer(_tempbuf, size);
}

int64_t ggtr::FileSystem::_AllocateBuffer(void * buffer, const int64_t size)
{
	
	if (buffer != nullptr)
		free(buffer);
	buffer = malloc(size);

	if (_fp != nullptr)
		setvbuf(_fp, (char *)buffer, _IOFBF, size);

	return size;
}

ggtr::FileInfo::FileInfo()
	: offset(offset), size(size), id(-1), binary(nullptr), mother_binary(nullptr)
{
}

ggtr::FileInfo::FileInfo(const int64_t offset, const int64_t size)
	: id(-1), offset(offset), size(size), binary(nullptr), mother_binary(nullptr)
{
}

ggtr::FileInfo::FileInfo(const int64_t id, const int64_t offset, const int64_t size)
	: id(id), offset(offset), size(size), binary(nullptr)
{
}

ggtr::FileInfo::FileInfo(const int64_t id, const int64_t offset, const int64_t size, void * binary)
	: id(id), offset(offset), size(size), binary(binary), mother_binary(binary)
{
}

ggtr::FileInfo::FileInfo(const int64_t id, const int64_t offset, const int64_t size, void * binary, void * mother_binary)
	: id(id), offset(offset), size(size), binary(binary), mother_binary(mother_binary)
{
}

ggtr::FileInfo::FileInfo(const FileInfo & src)
	: offset(src.offset), size(src.size), id(src.id), binary(src.binary), mother_binary(src.mother_binary)
{
}

FileInfo ggtr::FileInfo::operator=(const FileInfo & src) const
{
	return FileInfo(src.id, src.offset, src.size, src.binary);
}

const bool ggtr::FileInfo::Dispose()
{
	if (mother_binary != nullptr)
	{
		free(mother_binary);
		mother_binary = nullptr;
		return true;
	}
	return false;
}
