#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <string>

enum files {
	SOURCE_VERTEX,
	SOURCE_INIT,
	SOURCE_STEP,
	SOURCE_DRAW
};

std::string getFile(files file);

#endif // __SOURCES_H__
