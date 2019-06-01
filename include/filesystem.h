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
		int64_t offset;		//!< 画像ファイルの位置
		int64_t size;		//!< 画像ファイルの容量
	};

	/**
	* 無効なファイルヘッダがあるときに送出
	*/
	class InvalidFileHeaderException : public std::exception {};

	/**
	* ファイルデータベースシステム
	*/
	class FileSystem
	{
		std::string _dbpath;	//!< データベースのパス
		int64_t _size;			//!< 内部データのサイズ
		int64_t _total;			//!< ファイル全体の容量
		int64_t _allocation;	//!< 確保サイズ
		FILE* _fp;				//!< ファイルポインタちゃん

	private:
		void _PreOpenDB(const char * const dbpath, FILE * fp);
		void _MoveDatabase(const char * const todbpath);

	public:
		FileSystem(const char * const dbpath, int64_t allocation);

		FileSystem(const std::string & dbpath, int64_t allocation);

		const void * const Query(int64_t offset, int64_t size);

		const void * const Query(const FileInfo & fileinfo);

		const FileInfo Insert(const char * const path);

		const FileInfo Insert(const std::string & path);

		/**
		* データベースを移動する
		* 移動先にデータベースが存在する場合は.bakを付けて退避させる
		*/
		void MoveDatabase(const char * const todbpath);

		/**
		* データベースを移動する
		* 移動先にデータベースが存在する場合は.bakを付けて退避させる
		*/
		void MoveDatabase(const std::string & todbpath);

		/**
		* データベースのパスを返す
		*/
		const std::string & dbpath() const { return _dbpath; }
	};
}