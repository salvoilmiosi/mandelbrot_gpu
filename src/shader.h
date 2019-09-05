#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>

#include "resource.h"

#define SHADER_SOURCE(name) shader_##name##_glsl
#define GET_SHADER_SOURCE(name) GET_RESOURCE(SHADER_SOURCE(name))
#define SHADER(name) _shader_##name
#define SHADER_DECLARE(type, name)      \
BINARY_DECLARE(SHADER_SOURCE(name))     \
shader SHADER(name)(#name, type, GET_SHADER_SOURCE(name));

struct vec2 {
	float x;
	float y;
};

class shader {
private:
    GLuint shader_id = 0;
    
    const char *name;
    const GLenum type;
    const resource &source;

public:
    shader(const char *name, const GLenum type, const resource &source) : name(name), type(type), source(source) {};

    ~shader() {
        glDeleteShader(shader_id);
    }

    int compile();

private:
    friend class shader_program;
};

class shader_program {
private:
	GLuint program_id = 0;

	shader &vertex;
	shader &fragment;

public:
    shader_program(shader &vertex, shader &fragment) : vertex(vertex), fragment(fragment) {}

    ~shader_program() {
        if (program_id) {
            glDetachShader(program_id, vertex.shader_id);
            glDetachShader(program_id, fragment.shader_id);
            glDeleteProgram(program_id);
        }
    }

    int compile();

    void use_program() {
        glUseProgram(program_id);
    }

    void set_uniform_i(const char *name, int value) {
        int location = glGetUniformLocation(program_id, name);
        glUniform1i(location, value);
    }

    void set_uniform_f(const char *name, float value) {
        int location = glGetUniformLocation(program_id, name);
        glUniform1f(location, value);
    }

    void set_uniform_vec2(const char *name, vec2 value) {
        int location = glGetUniformLocation(program_id, name);
        glUniform2f(location, value.x, value.y);
    }
};

#endif
