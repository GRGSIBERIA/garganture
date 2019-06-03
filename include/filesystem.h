﻿#pragma once
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
	* ファイルのバイナリを扱う型
	*/
	class FileBinary
	{
		char * _data;	//!< ファイルのバイナリ本体

	public:
		/**
		* バイナリ本体を返す
		*/
		const char * const ptr() const;

		/**
		* リソースを解放する
		*/
		void Dispose();
	};

	/**
	* ファイルのバイナリのリスト
	* 明示的にDisposeしなければリソースは解放されない
	*/
	class FileBinaryList
	{
		int64_t _length;		//!< ファイルの長さ
		FileBinary * _binaries;	//!< ファイルのポインタ

	public:
		/**
		* リソースを解放する
		*/
		void Dispose();

		/**
		* バイナリを返す
		*/
		const char * const at(const int64_t id) const;

		/**
		* 配列の長さを返す
		*/
		const int64_t length() const;
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

		FILE * _fp;				//!< ファイルポインタちゃん
		void * _buffer;			//!< 読み書きバッファ
		int64_t _bufsize;		//!< バッファの容量
		void * _tempbuf;		//!< 複数ファイルをまとめるための一時領域
		int64_t _tempsize;		//!< 複数ファイルバッファの容量

	private:
		void _PreOpenDB(const char * const dbpath);
		void _MoveDatabase(const char * const todbpath);

		// 書き込み周り
		void _ExpandRegion(const int64_t size);
		void _ExpandBuffer(const int64_t size);
		void _ExpandTemp(const int64_t size);
		int64_t _AllocateBuffer(void * buffer, const int64_t size);
		const FileInfo _InsertSingle(const char * const binary, const int64_t size);
		std::vector<FileInfo> _InsertMulti(const char ** const binaries, const int64_t sizes, const size_t numof_insertion);

	public:
		FileSystem(const char * const dbpath, const int64_t allocation);

		FileSystem(const std::string & dbpath, const int64_t allocation);

		~FileSystem();

		/***********************************************************************
		* 読み込み周りの処理
		************************************************************************/

		//Binary Query(const FileInfo& info);

		//FileBinaryList Query(const FileInfo * const infos);

		//FileBinaryList Query(const std::vector<FileInfo> & infos);

		/***********************************************************************
		* 書き込み周りの処理
		************************************************************************/

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