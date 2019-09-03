#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <cstring>

#define BINARY_START(name) _binary_##name##_start
#define BINARY_END(name) _binary_##name##_end
#define BINARY_DECLARE(name) extern const char BINARY_START(name); extern const char BINARY_END(name);

char *getResource(const char *str_start, const size_t len) {
    char *str = (char *) malloc(len + 1);
    memcpy(str, str_start, len);
    str[len-1] = '\0';
    return str;
}

#define GET_RESOURCE(name) getResource(&BINARY_START(name), &BINARY_END(name) - &BINARY_START(name))

#define SHADER(name) shader_##name##_glsl
#define SHADER_DECLARE(name) BINARY_DECLARE(SHADER(name))
#define GET_SHADER(name) GET_RESOURCE(SHADER(name))

SHADER_DECLARE(vertex)
SHADER_DECLARE(init)
SHADER_DECLARE(step)
SHADER_DECLARE(draw)
SHADER_DECLARE(final)

#endif // __SOURCES_H__
