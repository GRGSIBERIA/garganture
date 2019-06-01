#include "filesystem.h"
#include "filesystem\fileutil.h"
using namespace ggtr;

void FileSystem::_PreOpenDB(const char * const dbpath, FILE * fp)
{
	const int64_t zero = 0;
	const char header[4] = { 'g', 'g', 't', 'r' };
	
	_size = 0;

	if (!file::Exists(dbpath))
	{
		// ファイルが存在しない場合、新たにファイルを作成する
		fopen_s(&fp, dbpath, "rb");
		fwrite(header, 1, 4, fp);
		fwrite(&_size, sizeof(int64_t), 1, fp);
		fwrite(&zero, sizeof(int64_t), _allocation, fp);	// 領域を予約する

		_size = 4 + sizeof(int64_t);
		_total = _ftelli64(fp);
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

		fread_s(&_size, sizeof(int64_t), sizeof(int64_t), 1, fp);
		_total = _fseeki64(fp, 0, SEEK_END);
	}

	fclose(fp);
}

FileSystem::FileSystem(const char * const dbpath, int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation)
{
	_PreOpenDB(_dbpath.c_str(), _fp);
}

FileSystem::FileSystem(const std::string & dbpath, int64_t allocation)
	: _dbpath(dbpath), _allocation(allocation)
{
	_PreOpenDB(_dbpath.c_str(), _fp);
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