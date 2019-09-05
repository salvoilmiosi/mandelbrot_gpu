#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>
#include <map>

#include "resource.h"

#define SHADER_SOURCE(name) shader_##name##_glsl
#define GET_SHADER_SOURCE(name) GET_RESOURCE(SHADER_SOURCE(name))
#define SHADER(name) _shader_##name

#define DECLARE_SHADER(type, name)      \
DECLARE_BINARY(SHADER_SOURCE(name))     \
shader SHADER(name)(#name, type, GET_SHADER_SOURCE(name));

#define DECLARE_UNIFORM(name, value) uniform name(#name, value);

struct vec2 {
	float x;
	float y;

    vec2(float x, float y) : x(x), y(y) {}
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

enum uniform_type {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VEC2
};

struct uniform {
    const char *name;
    uniform_type type;

    union {
        int value_int;
        float value_float;
        vec2 value_vec2;
    };

    template<typename T>
    uniform(const char *name, T value) : name(name) {
        *this = value;
    }

    int &operator = (const int &value) {
        type = TYPE_INT;
        return value_int = value;
    }

    float &operator = (const float &value) {
        type = TYPE_FLOAT;
        return value_float = value;
    }

    float &operator = (const double &value) {
        type = TYPE_FLOAT;
        return value_float = value;
    }

    vec2 &operator = (const vec2 &value) {
        type = TYPE_VEC2;
        return value_vec2 = value;
    }

    operator int() {
        return value_int;
    }

    operator bool() {
        return value_int;
    }

    operator float() {
        return value_float;
    }

    operator vec2() {
        return value_vec2;
    }
};

class shader_program {
private:
	GLuint program_id = 0;

	shader &vertex;
	shader &fragment;

    std::map<int, uniform *> uniforms;

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

    void add_uniform(uniform &uni) {
        int location = glGetUniformLocation(program_id, uni.name);
        uniforms[location] = &uni;
    }

    void add_uniforms() {}

    template <typename ... Ts>
    void add_uniforms(uniform &first, Ts &...unis) {
        add_uniform(first);
        add_uniforms(unis...);
    }

    void use_program() {
	    glUseProgram(program_id);
        update_uniforms();
    }

    void update_uniforms();
};

#endif
