#include "filesystem.h"
#include "filesystem\fileutil.h"
using namespace ggtr;

// 初期化時にファイルデータベースを開いて中身のデータを抜いてくる
void FileSystem::_PreOpenDB(const char * const dbpath, FILE * fp)
{
	const char zero = 0;
	const char header[4] = { 'g', 'g', 't', 'r' };
	
	_offset = 0;

	if (!file::Exists(dbpath))
	{
		// ファイルが存在しない場合、新たにファイルを作成する
		fopen_s(&fp, dbpath, "rb");
		fwrite(header, 1, 4, fp);
		fwrite(&_offset, sizeof(int64_t), 1, fp);
		fwrite(&zero, sizeof(char), _allocation, fp);	// 領域を予約する

		_offset = 4 + sizeof(int64_t);
		_region = _ftelli64(fp);
	}
	else
	{
		// ファイルが存在したらヘッダと容量を照合する
		fopen_s(&fp, dbpath, "rb");

		char comparison[4];
		fread_s(comparison, 4, 1, 4, fp);

		// ファイルヘッダの有効性チェック
		for (int i = 4; i < 4; ++i)
		{
			if (header[i] != comparison[i])
			{
				fclose(fp);		// 無効なファイルヘッダが来たので一旦ファイルを閉じてどうするか決める
				throw new InvalidFileHeaderException();
			}
		}

		fread_s(&_offset, sizeof(int64_t), sizeof(int64_t), 1, fp);
		_region = _fseeki64(fp, 0, SEEK_END);
	}

	fclose(fp);
}

FileSystem::FileSystem(const char * const dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr)
{
	_PreOpenDB(_dbpath.c_str(), _fp);
}

FileSystem::FileSystem(const std::string & dbpath, const int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation), _bufsize(0), _buffer(nullptr)
{
	_PreOpenDB(_dbpath.c_str(), _fp);
}

ggtr::FileSystem::~FileSystem()
{
	if (_buffer != nullptr)
		free(_buffer);
}

const FileInfo ggtr::FileSystem::Insert(const char * const binary, const int64_t size)
{
	return _InsertSingle(binary, size);
}

std::vector<FileInfo> ggtr::FileSystem::Insert(const char ** const binaries, const int64_t * const sizes, const size_t numof_insertion)
{
	return std::vector<FileInfo>();
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
	if (_offset + size > _region)
	{
		const char zero = 0;
		_fseeki64(_fp, 0, SEEK_END);
		fwrite(&zero, sizeof(char), _allocation, _fp);
	}
}

// バッファの領域が不足しているなら
void ggtr::FileSystem::_ExpandBuffer(const int64_t size)
{
	if (_bufsize < size)
	{
		_bufsize = size;
		if (_buffer != nullptr)
			free(_buffer);
		_buffer = malloc(_bufsize);
		setvbuf(_fp, (char*)_buffer, _IOFBF, _bufsize);
	}
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
