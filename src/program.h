#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <vector>
#include <string>

#include <GL/glew.h>

#include "shader.h"

struct vec2 {
	float x;
	float y;
};

class program {
public:
	program(const char *name);
	~program();

	void attachShader(shader &s);
	void link();
	void bind();

	template<typename T>
	void setUniform(const char *name, const T &value);

	int getAttribLoc(const char *name);

	const char *name;

private:
	std::vector<shader*> shaders;

	GLuint program_id;

	int uni_loc(const char *name);
};

program::program(const char *name) : name(name) {
	program_id = glCreateProgram();
}

program::~program() {
	for (auto &s : shaders) {
		glDetachShader(program_id, s->shader_id);
	}
	shaders.clear();
	if (program_id) {
		glDeleteProgram(program_id);
		program_id = 0;
	}
}

void program::attachShader(shader &s) {
	shaders.push_back(&s);

	glAttachShader(program_id, s.shader_id);
}

void program::link() {
	glLinkProgram(program_id);
}

void program::bind() {
	glUseProgram(program_id);
}

int program::uni_loc(const char *name) {
	return glGetUniformLocation(program_id, name);
}

template<> void program::setUniform(const char *name, const int &value) {
	bind();
	glUniform1i(uni_loc(name), value);
}

template<> void program::setUniform(const char *name, const float &value) {
	bind();
	glUniform1f(uni_loc(name), value);
}

template<> void program::setUniform(const char *name, const vec2 &value) {
	bind();
	glUniform2f(uni_loc(name), value.x, value.y);
}

int program::getAttribLoc(const char *name) {
	return glGetAttribLocation(program_id, name);
}

#endif // __PROGRAM_H__