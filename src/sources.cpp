#include "sources.h"

#include <iostream>

extern char _binary_shader_vertex_glsl_start;
extern char _binary_shader_vertex_glsl_end;

extern char _binary_shader_init_glsl_start;
extern char _binary_shader_init_glsl_end;

extern char _binary_shader_step_glsl_start;
extern char _binary_shader_step_glsl_end;

extern char _binary_shader_draw_glsl_start;
extern char _binary_shader_draw_glsl_end;

std::string getResource(char &begin, char &end) {
	return std::string(&begin, &end);
}

std::string getFile(files file) {
	switch (file) {
	case SOURCE_VERTEX:
		return getResource(_binary_shader_vertex_glsl_start, _binary_shader_vertex_glsl_end);
	case SOURCE_INIT:
		return getResource(_binary_shader_init_glsl_start, _binary_shader_init_glsl_end);
	case SOURCE_STEP:
		return getResource(_binary_shader_step_glsl_start, _binary_shader_step_glsl_end);
	case SOURCE_DRAW:
		return getResource(_binary_shader_draw_glsl_start, _binary_shader_draw_glsl_end);
	default:
		return std::string();
	}
}