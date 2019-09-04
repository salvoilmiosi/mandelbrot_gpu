#include "shader.h"

#include <cstdio>

static int check_shader_error(const char *name, const char *type, GLuint shader) {
	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		int length = 0;
		int max_length = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

		char *info_log = new char[max_length];

		glGetShaderInfoLog(shader, max_length, &length, info_log);
		if (length > 0){
			fprintf(stderr, "%s %s Error:\n%s\n", name, type, info_log);
		}

		delete[] info_log;

		return 1;
	}
	return 0;
}

int shader_program::create_program(const char *vertex_source, const char *fragment_source) {
	program_id = glCreateProgram();

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	int vertex_source_length = strlen(vertex_source);
	glShaderSource(vertex_shader, 1, &vertex_source, &vertex_source_length);
	glCompileShader(vertex_shader);

	if (check_shader_error(name, "vertex", vertex_shader) != 0) {
		return 1;
	}

	glAttachShader(program_id, vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	int fragment_source_length = strlen(fragment_source);
	glShaderSource(fragment_shader, 1, &fragment_source, &fragment_source_length);
	glCompileShader(fragment_shader);

	if (check_shader_error(name, "frag", fragment_shader) != 0) {
		return 1;
	}

	glAttachShader(program_id, fragment_shader);

	glLinkProgram(program_id);

	return 0;
}
