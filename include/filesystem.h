#pragma once
#include <string>
#include <stdio.h>
#include <vector>

namespace ggtr
{
	/**
	* ファイルの情報
	*/
	struct FileInfo
	{
		int64_t offset;		//!< 画像ファイルの位置
		int64_t size;		//!< 画像ファイルの容量

		FileInfo();
		FileInfo(const int64_t offset, const int64_t size);
		FileInfo(const FileInfo & src);
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
		int64_t _offset;		//!< 内部データ末尾の位置
		int64_t _region;		//!< ファイル全体の容量
		int64_t _allocation;	//!< 確保サイズ

		FILE* _fp;				//!< ファイルポインタちゃん
		void* _buffer;			//!< 読み書きバッファ
		int32_t _bufsize;		//!< バッファの容量

	private:
		void _PreOpenDB(const char * const dbpath, FILE * fp);
		void _MoveDatabase(const char * const todbpath);
		void _ExpandRegion(const int64_t size);
		void _ExpandBuffer(const int64_t size);

		const FileInfo _InsertSingle(const char * const binary, const int64_t size);

		std::vector<FileInfo> _InsertMulti(const char ** const binaries, const int64_t sizes, const size_t numof_insertion);

	public:
		FileSystem(const char * const dbpath, const int64_t allocation);

		FileSystem(const std::string & dbpath, const int64_t allocation);

		~FileSystem();

		/**
		* 単一ファイルをデータベースに登録する
		*/
		const FileInfo Insert(const char * const binary, const int64_t size);

		/**
		* 複数ファイルをデータベースに登録する
		*/
		std::vector<FileInfo> Insert(const char ** const binaries, const int64_t * const sizes, const size_t numof_insertion);

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