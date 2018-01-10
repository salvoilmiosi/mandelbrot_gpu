#include "sources.h"

#include <iostream>

extern char _binary_src_vertex_glsl_start;
extern char _binary_src_vertex_glsl_end;

extern char _binary_src_init_glsl_start;
extern char _binary_src_init_glsl_end;

extern char _binary_src_step_glsl_start;
extern char _binary_src_step_glsl_end;

extern char _binary_src_draw_glsl_start;
extern char _binary_src_draw_glsl_end;

std::string getResource(char &begin, char &end) {
	return std::string(&begin, &end);
}

std::string getFile(files file) {
	switch (file) {
	case FILE_VERTEX:
		return getResource(_binary_src_vertex_glsl_start, _binary_src_vertex_glsl_end);
	case FILE_INIT:
		return getResource(_binary_src_init_glsl_start, _binary_src_init_glsl_end);
	case FILE_STEP:
		return getResource(_binary_src_step_glsl_start, _binary_src_step_glsl_end);
	case FILE_DRAW:
		return getResource(_binary_src_draw_glsl_start, _binary_src_draw_glsl_end);
	default:
		return std::string();
	}
}