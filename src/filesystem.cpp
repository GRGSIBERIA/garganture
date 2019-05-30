#include "filesystem.h"
#include "filesystem\fileutil.h"
using namespace ggtr;

FileSystem::FileSystem(const char * const dbpath)
	: _dbpath(dbpath)
{
	
}

FileSystem::FileSystem(const std::string & dbpath)
	: _dbpath(dbpath)
{

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