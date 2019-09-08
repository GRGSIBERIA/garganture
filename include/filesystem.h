#pragma once
#include <string>
#include <stdio.h>
#include <vector>
#include <exception>

namespace ggtr
{
	class FileSystem;

	/**
	* ファイルの情報
	*/
	struct FileInfo
	{
	public:
		friend FileSystem;

		int64_t offset;			//!< 画像ファイルの位置
		int64_t size;			//!< 画像ファイルの容量
		void * binary;			//!< 画像ファイルのバイナリ

	private:
		int64_t id;				//!< 配列の内部的な順序
		void * mother_binary;	//!< 元リソース、Disposeで明示的に解放する

	public:
		FileInfo();
		FileInfo(const int64_t offset, const int64_t size);
		FileInfo(const FileInfo & src);
		
		FileInfo operator =(const FileInfo & src) const;

		/**
		* リソースを明示的に解放する
		* @return 解放できた場合はtrue, すでに解放されていたらfalse
		*/
		const bool Dispose();

	private:
		FileInfo(const int64_t id, const int64_t offset, const int64_t size);
		FileInfo(const int64_t id, const int64_t offset, const int64_t size, void * binary);
		FileInfo(const int64_t id, const int64_t offset, const int64_t size, void * binary, void * mother_binary);
	};

	/**
	* 無効なファイルヘッダがあるときに送出
	*/
	class InvalidFileHeaderException : public std::exception {};

	/**
	* ファイルが存在しない
	*/
	class DatabaseFileNotFoundException : public std::exception 
	{
	public:
		DatabaseFileNotFoundException(const std::string & message)
			: exception(message.c_str()) {}
	};

	/**
	* データベースを開くときにエラーが出た
	*/
	class OpenDatabaseException : public std::exception
	{
	public:
		OpenDatabaseException(const errno_t error, const char * mode)
			: exception((std::string("Can't open database: ") + mode).c_str()) {}
	};

	/**
	* ファイルデータベースシステム
	*/
	class FileSystem
	{
		std::string _dbpath;	//!< データベースのパス
		int64_t _offset;		//!< 内部データ末尾の位置
		int64_t _region;		//!< ファイル全体の容量
		int64_t _allocation;	//!< 確保サイズ

		FILE * _fp;				//!< ファイルポインタちゃん
		void * _buffer;			//!< ファイルの内部バッファ
		int64_t _bufsize;		//!< ファイルの内部バッファの容量
		void * _tempbuf;		//!< 書き込み時に複数ファイルをまとめるための一時領域
		int64_t _tempsize;		//!< 書き込み時に複数ファイルバッファの容量

	private:
		void _PreOpenDB();
		void _MoveDatabase(const char * const todbpath);
		void _FileOpen(const char * const mode);
		void _FileClose();		// ファイルを閉じるときにoffsetを書き込む

		// 書き込み周り
		void _ExpandRegion(const int64_t size);
		void _ExpandBuffer(const int64_t size);
		void _ExpandTemp(const int64_t size);
		int64_t _AllocateBuffer(void * buffer, const int64_t size);

		static bool _SortId(const FileInfo & a, const FileInfo & b);
		static bool _SortOffset(const FileInfo & a, const FileInfo & b);

	public:
		/**
		* @param[in] dbpath データベースのパス
		* @param[in] allocation データベースを拡張する単位, バイトなので256MB、512MBなど巨大なほうがいい
		*/
		FileSystem(const char * const dbpath, const int64_t allocation);

		FileSystem(const std::string & dbpath, const int64_t allocation);

		~FileSystem();

		/***********************************************************************
		* 読み込み周りの処理
		************************************************************************/
		/**
		* @param[inout] info ファイルの情報
		* @return 確保された領域
		* @attention 明示的にDisposeを呼び出さないと領域が開放されない
		*/
		void * Query(FileInfo& info);

		/**
		* 
		*/
		void * Query(const FileInfo * const infos, const int64_t size);

		/**
		* @param[inout] infos ファイルの情報
		* @return 確保された領域
		* @attention
		*	明示的にFileBinaryListのインスタンスからDisposeを呼び出さないと領域が解放されない
		*	また、個別のインスタンスのDisposeを呼び出しても領域は解放しない
		*/
		void * Query(std::vector<FileInfo> & infos);

		/***********************************************************************
		* 書き込み周りの処理
		************************************************************************/

		/**
		* 単一ファイルをデータベースに登録する
		* @return 書き込んだファイルの情報
		*/
		const FileInfo Insert(const char * const binary, const int64_t size);

		/**
		* 複数ファイルをデータベースに登録する
		* @return 書き込んだファイルの情報
		*/
		std::vector<FileInfo> Insert(const char ** const binaries, const int64_t * const sizes, const size_t numof_insertion);

		/***********************************************************************
		* 予備的な処理
		************************************************************************/

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

		/***********************************************************************
		* プロパティ関数
		************************************************************************/

		/**
		* データベースのパスを返す
		*/
		const std::string & dbpath() const { return _dbpath; }

		
	};
}