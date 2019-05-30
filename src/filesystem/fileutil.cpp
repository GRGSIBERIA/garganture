#include "filesystem\fileutil.h"
#include <stdio.h>
#include <sys\stat.h>

bool ggtr::file::Exists(const char * const path)
{
	struct stat st;

	if (stat(path, &st) != 0) {
		return false;
	}

	return ((st.st_mode & S_IFMT) == S_IFREG) > 0;
}

bool ggtr::file::Move(const char * const from, const char * const to)
{
	return !rename(from, to) != 0;
}
