#ifndef __SHADER_H__
#define __SHADER_H__

#include <GL/glew.h>

#include <cstdlib>
#include <cstring>

class resource_data {
private:
    const char *str_start;
    const size_t len;
    char *data = NULL;

public:
    resource_data(const char *str_start, const size_t len) : str_start(str_start), len(len) {}
    ~resource_data() {
        if (data) free(data);
    }
    
    char *operator ()() {
        if (!data) {
            data = (char *) malloc(len + 1);
            memcpy(data, str_start, len);
            data[len-1] = '\0';
        }
        return data;
    }
};

#define BINARY_START(name) _binary_##name##_start
#define BINARY_END(name) _binary_##name##_end
#define BINARY_RESDATA(name) _binary_##name##_resdata

#define BINARY_DECLARE(name)                \
extern const char BINARY_START(name)[];     \
extern const char BINARY_END(name)[];       \
resource_data BINARY_RESDATA(name)(BINARY_START(name), BINARY_END(name) - BINARY_START(name));

#define GET_RESOURCE(name) BINARY_RESDATA(name)()

#define SHADER(name) shader_##name##_glsl
#define SHADER_DECLARE(name) BINARY_DECLARE(SHADER(name))
#define GET_SHADER(name) GET_RESOURCE(SHADER(name))

struct vec2 {
	float x;
	float y;
};

class shader_program {
private:
	const char *name;
	GLuint program_id = 0;
	GLuint vertex_shader = 0;
	GLuint fragment_shader = 0;

public:
	explicit shader_program(const char *name) : name(name) {}

    ~shader_program() {
        glDetachShader(program_id, vertex_shader);
        glDeleteShader(vertex_shader);
        glDetachShader(program_id, fragment_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program_id);
    }

    int create_program(const char *vertex_source, const char *fragment_source);

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
