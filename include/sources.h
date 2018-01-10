#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <string>

enum files {
	FILE_VERTEX,
	FILE_INIT,
	FILE_STEP,
	FILE_DRAW
};

std::string getFile(files file);

#endif // __SOURCES_H__
