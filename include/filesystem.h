#pragma once
#include <string>
#include <stdio.h>

namespace ggtr
{
	/**
	* ファイルの情報
	*/
	struct FileInfo
	{
		uint64_t offset;
		uint64_t size;
	};

	/**
	* ファイルデータベースシステム
	*/
	class FileSystem
	{
		std::string _dbpath;
		FILE* _fp;

	private:
		void _MoveDatabase(const char * const todbpath);

	public:
		FileSystem(const char * const dbpath);

		FileSystem(const std::string & dbpath);

		const void * const Query(uint64_t offset, uint64_t size);

		const void * const Query(const FileInfo & fileinfo);

		const FileInfo Insert(const char * const path);

		const FileInfo Insert(const std::string & path);

		/**
		* データベースを移動する
		*/
		void MoveDatabase(const char * const todbpath);

		/**
		* データベースを移動する
		*/
		void MoveDatabase(const std::string & todbpath);

		/**
		* データベースのパスを返す
		*/
		const std::string & dbpath() const { return _dbpath; }
	};
}