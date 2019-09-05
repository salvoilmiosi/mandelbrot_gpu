#include "shader.h"

#include <cstdio>

int shader::compile() {
	if (shader_id) {
		return 0;
	} else {
		shader_id = glCreateShader(type);
	}

	glShaderSource(shader_id, 1, &source.data, &source.size);
	glCompileShader(shader_id);

	int length = 0;
	int max_length = 0;

	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);

	char *info_log = (char *) malloc(max_length);

	glGetShaderInfoLog(shader_id, max_length, &length, info_log);
	if (length > 0) {
		fprintf(stdout, "In shader %s:\n%s\n", name, info_log);
	}

	free(info_log);

	GLint compiled = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) return 1;
	return 0;
}

int shader_program::compile() {
	if (vertex.compile() != 0) return 1;
	if (fragment.compile() != 0) return 1;

	program_id = glCreateProgram();
	glAttachShader(program_id, vertex.shader_id);
	glAttachShader(program_id, fragment.shader_id);
	glLinkProgram(program_id);

	return 0;
}
