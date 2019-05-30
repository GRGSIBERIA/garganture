#pragma once

namespace ggtr
{
	namespace file
	{
		/**
		* ファイルの存在を確認
		*/
		bool Exists(const char * const path);

		/**
		* ファイルを強制的に移動する
		* 強制的に移動するので例外を落として失敗するケースがある
		*/
		bool Move(const char * const from, const char * const to);
	}
}