#ifndef __SOURCES_H__
#define __SOURCES_H__

#include <string>

#define BINARY_START(name) _binary_##name##_start
#define BINARY_END(name) _binary_##name##_end
#define BINARY_DECLARE(name) extern char BINARY_START(name); extern char BINARY_END(name);
#define GET_RESOURCE(name) std::string(&BINARY_START(name), &BINARY_END(name))

#define SHADER(name) shader_##name##_glsl
#define SHADER_DECLARE(name) BINARY_DECLARE(SHADER(name))

#define GET_SHADER(name) GET_RESOURCE(SHADER(name))

SHADER_DECLARE(vertex);
SHADER_DECLARE(init);
SHADER_DECLARE(step);
SHADER_DECLARE(draw);
SHADER_DECLARE(final);

#endif // __SOURCES_H__
