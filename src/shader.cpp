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
	if (program_id) {
		return 0;
	} else {
		program_id = glCreateProgram();
	}

	for (shader *i : shaders) {
		if (i->compile() != 0) return 1;
		glAttachShader(program_id, i->shader_id);
	}

	glLinkProgram(program_id);

	return 0;
}

void shader_program::update_uniforms() {
    for (uniform_location &i : uniforms) {
		switch(i.uni->type) {
		case uniform::TYPE_INT:
			glUniform1i(i.location, i.uni->value_int);
			break;
		case uniform::TYPE_FLOAT:
			glUniform1f(i.location, i.uni->value_float);
			break;
		case uniform::TYPE_VEC2:
			glUniform2f(i.location, i.uni->value_vec2.x, i.uni->value_vec2.y);
			break;
		}
	}
}
