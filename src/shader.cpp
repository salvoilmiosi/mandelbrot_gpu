#include "shader.h"

#include <cstdio>

static int check_shader_error(const char *name, GLuint shader) {
	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		int length = 0;
		int max_length = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

		char *info_log = (char *) malloc(max_length);

		glGetShaderInfoLog(shader, max_length, &length, info_log);
		if (length > 0){
			fprintf(stderr, "Error in shader %s:\n%s\n", name, info_log);
		}

		free(info_log);

		return 1;
	}
	return 0;
}

int shader_program::create_program(const resource &vertex_source, const resource &fragment_source) {
	program_id = glCreateProgram();

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_source.data, &vertex_source.length);
	glCompileShader(vertex_shader);

	if (check_shader_error(vertex_source.name, vertex_shader) != 0) {
		return 1;
	}

	glAttachShader(program_id, vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_source.data, &fragment_source.length);
	glCompileShader(fragment_shader);

	if (check_shader_error(fragment_source.name, fragment_shader) != 0) {
		return 1;
	}

	glAttachShader(program_id, fragment_shader);

	glLinkProgram(program_id);

	return 0;
}
