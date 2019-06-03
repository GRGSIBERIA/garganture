#include "filesystem.h"
#include "filesystem\fileutil.h"
#include <exception>
using namespace ggtr;

// 初期化時にファイルデータベースを開いて中身のデータを抜いてくる
void FileSystem::_PreOpenDB(const char * const dbpath)
{
	const char zero = 0;
	const char header[4] = { 'g', 'g', 't', 'r' };
	
	_offset = 0;

	if (!file::Exists(dbpath))
	{
		// ファイルが存在しない場合、新たにファイルを作成する
		fopen_s(&_fp, dbpath, "rb");
		fwrite(header, 1, 4, _fp);
		fwrite(&_offset, sizeof(int64_t), 1, _fp);
		fwrite(&zero, sizeof(char), _allocation, _fp);	// 領域を予約する

		_offset = 4 + sizeof(int64_t);
		_region = _ftelli64(_fp);
	}
	else
	{
		// ファイルが存在したらヘッダと容量を照合する
		fopen_s(&_fp, dbpath, "rb");

		char comparison[4];
		fread_s(comparison, 4, 1, 4, _fp);

		// ファイルヘッダの有効性チェック
		for (int i = 4; i < 4; ++i)
		{
			if (header[i] != comparison[i])
			{
				fclose(_fp);		// 無効なファイルヘッダが来たので一旦ファイルを閉じてどうするか決める
				throw new InvalidFileHeaderException();
			}
		}

		fread_s(&_offset, sizeof(int64_t), sizeof(int64_t), 1, _fp);
		_region = _fseeki64(_fp, 0, SEEK_END);
	}

	fclose(_fp);
	_fp = nullptr;		// setvbufの判断を確実にするため、特に用がない限りはnullptrでゼロにする
}

FileSystem::FileSystem(const char * const dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr), _tempsize(0), _tempbuf(nullptr)
{
	_PreOpenDB(_dbpath.c_str());
}

FileSystem::FileSystem(const std::string & dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr), _tempsize(0), _tempbuf(nullptr)
{
	_PreOpenDB(_dbpath.c_str());
}

ggtr::FileSystem::~FileSystem()
{
	if (_buffer != nullptr)
		free(_buffer);
	if (_tempbuf != nullptr)
		free(_tempbuf);
}

const FileInfo ggtr::FileSystem::Insert(const char * const binary, const int64_t size)
{
	return _InsertSingle(binary, size);
}

std::vector<FileInfo> ggtr::FileSystem::Insert(const char ** const binaries, const int64_t * const sizes, const size_t numof_insertion)
{
	auto files = std::vector<FileInfo>();
	files.reserve(numof_insertion);

	// 合計サイズを求める
	int64_t total = 0;
	for (size_t i = 0; i < numof_insertion; ++i)
		total += sizes[i];

	// ファイルの配列を用意する
	for (size_t i = 0; i < numof_insertion; ++i)
	{
		FileInfo file(_offset, sizes[i]);
		files.emplace_back(_offset, sizes[i]);
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

	// ファイルの書き込み
	fopen_s(&_fp, _dbpath.c_str(), "ab");
	setvbuf(_fp, (char *)_buffer, _IOFBF, total);
	_fseeki64(_fp, _offset, SEEK_SET);
	fwrite(_tempbuf, total, 1, _fp);

	fclose(_fp);
	_fp = nullptr;

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

// 単一のファイルを扱うときの関数、Insertで名前を一本化するので……
const FileInfo ggtr::FileSystem::_InsertSingle(const char * const binary, const int64_t size)
{
	FileInfo file;

	fopen_s(&_fp, _dbpath.c_str(), "ab");

	// 領域が不足しそうだったら追加で領域を確保する
	_ExpandRegion(size);
	_ExpandBuffer(size);

	// ファイルに書き込み
	_fseeki64(_fp, _offset, SEEK_SET);
	fwrite(binary, size, 1, _fp);

	file.offset = _offset;	// 位置をずらす前に記録
	file.size = size;
	_offset += size;		// サイズを変更

	fclose(_fp);
	_fp = nullptr;

	return file;
}

std::vector<FileInfo> ggtr::FileSystem::_InsertMulti(const char ** const binaries, const int64_t sizes, const size_t numof_insertion)
{
	return _InsertMulti(binaries, sizes, numof_insertion);
}

ggtr::FileInfo::FileInfo()
	: offset(offset), size(size)
{
}

ggtr::FileInfo::FileInfo(const int64_t offset, const int64_t size)
	: offset(offset), size(size)
{
}

ggtr::FileInfo::FileInfo(const FileInfo & src)
	: offset(src.offset), size(src.size)
{
}

void ggtr::FileBinaryList::Dispose()
{
	if (_binaries != nullptr)
	{
		for (uint64_t i = 0; i < _length; ++i)
			_binaries[i].Dispose();
		free(_binaries);
		_binaries = nullptr;
	}
}

const char * const ggtr::FileBinaryList::at(const int64_t id) const
{
	if (id < _length)
		return _binaries[id].ptr();
	throw new std::exception("Index out of range id < _length");
}

const int64_t ggtr::FileBinaryList::length() const
{
	return _length;
}

const char * const ggtr::FileBinary::ptr() const
{
	return _data;
}

void ggtr::FileBinary::Dispose()
{
	if (_data != nullptr)
	{
		free(_data);
		_data = nullptr;
	}
}
