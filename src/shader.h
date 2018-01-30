#ifndef __SHADER_H__
#define __SHADER_H__

#include <string>

#include <GL/glew.h>

class shader {
public:
	shader(const char *name, GLenum type);
	~shader();

	const char *name;
	const GLenum type;

	int loadSource(const std::string &source);

private:
	GLuint shader_id = 0;

	int check_shader_error();

	friend class program;
};

class vertex_shader : public shader {
public:
	vertex_shader(const char *name) : shader(name, GL_VERTEX_SHADER) {}
};

class frag_shader : public shader {
public:
	frag_shader(const char *name) : shader(name, GL_FRAGMENT_SHADER) {}
};

shader::shader(const char *name, GLenum type) : name(name), type(type) {}

shader::~shader() {
	if (shader_id) {
		glDeleteShader(shader_id);
		shader_id = 0;
	}
}

int shader::loadSource(const std::string &source) {
	shader_id = glCreateShader(type);

	int vertex_source_length = source.size();
	const char *source_c_str = source.c_str();
	glShaderSource(shader_id, 1, &source_c_str, &vertex_source_length);
	glCompileShader(shader_id);

	if (check_shader_error() != 0) {
		return 1;
	}

	return 0;
}

int shader::check_shader_error() {
	GLint compiled = GL_FALSE;
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		int length = 0;
		int max_length = 0;

		glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &max_length);

		char *info_log = (char*)malloc(max_length);

		glGetShaderInfoLog(shader_id, max_length, &length, info_log);
		if (length > 0){
			fprintf(stderr, "Error in shader %s:\n%s\n", name, info_log);
		}

		free(info_log);

		return 1;
	}
	return 0;
}

#endif // __SHADER_H__