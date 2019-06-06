#pragma once
#include <string>
#include <stdio.h>
#include <vector>
#include <exception>

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

		// ソートするときコンパイルに失敗する
		bool operator<(const FileInfo & info) const;

		FileInfo operator =(const FileInfo & src) const;
	};

	/**
	* ファイルのバイナリを扱う型
	*/
	class FileBinary
	{
		int64_t _size;		//!< ファイルの容量
		char * _bin;		//!< ファイルのバイナリ本体
		bool _disposable;	//!< freeできるポインタか？

	public:
		/**
		* バイナリ本体を返す
		*/
		const char * const ptr() const;

		/**
		* ファイルの容量を返す
		*/
		const int64_t size() const;

		/**
		* リソースを解放する
		*/
		void Dispose();

		/**
		* 単にアドレスを保管する
		*/
		FileBinary(char * bin, const int64_t binsize, const bool disposable);

		FileBinary();
	};

	/**
	* ファイルのバイナリのリスト
	* 明示的にDisposeしなければリソースは解放されない
	*/
	class FileBinaryList
	{
		char * _top_address;	//!< バイナリの先頭アドレス
		int64_t _length;		//!< 要素数
		int64_t _total_size;	//!< 全体の長さ
		std::vector<FileBinary> _binaries;	//!< ファイルのポインタ

	public:
		/**
		* リソースを解放する
		*/
		void Dispose();

		/**
		* 配列から要素を取ってくる
		*/
		const FileBinary & operator[](const int64_t id) const;

		/**
		* 配列の長さを返す
		*/
		const int64_t length() const;

		FileBinaryList(char * memory, const std::vector<char *> & addresses, const std::vector<FileInfo> & infos, const int64_t length, const int64_t memory_size);
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
		void * _buffer;			//!< 読み書きバッファ
		int64_t _bufsize;		//!< バッファの容量
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
		const FileInfo _InsertSingle(const char * const binary, const int64_t size);

	public:
		/**
		* @param dbpath [in] データベースのパス
		* @param allocation [in] データベースを拡張する単位, バイトなので256MB、512MBなど巨大なほうがいい
		*/
		FileSystem(const char * const dbpath, const int64_t allocation);

		FileSystem(const std::string & dbpath, const int64_t allocation);

		~FileSystem();

		/***********************************************************************
		* 読み込み周りの処理
		************************************************************************/
		/**
		* @return ファイルのバイナリ
		* @attention 明示的にDisposeを呼び出さないと領域が開放されない
		*/
		const FileBinary Query(const FileInfo& info);

		const FileBinaryList Query(const FileInfo * const infos, const int64_t size);

		/**
		* @return ファイルのバイナリの配列
		* @attention
		*	明示的にFileBinaryListのインスタンスからDisposeを呼び出さないと領域が解放されない
		*	また、個別のインスタンスのDisposeを呼び出しても領域は解放しない
		*/
		const FileBinaryList Query(const std::vector<FileInfo> & infos);

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