#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>

#include "resource.h"

#define SHADER(name) shader_##name##_glsl
#define SHADER_DECLARE(name) BINARY_DECLARE(SHADER(name))
#define GET_SHADER(name) GET_RESOURCE(SHADER(name))

struct vec2 {
	float x;
	float y;
};

class shader_program {
private:
	GLuint program_id = 0;
	GLuint vertex_shader = 0;
	GLuint fragment_shader = 0;

public:
    ~shader_program() {
        glDetachShader(program_id, vertex_shader);
        glDeleteShader(vertex_shader);
        glDetachShader(program_id, fragment_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program_id);
    }

    int create_program(const resource &vertex_source, const resource &fragment_source);

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
