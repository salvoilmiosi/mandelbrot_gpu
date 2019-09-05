#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include <cstdlib>

struct resource {
    const char *name;
    const char *data;
    const int length;
};

#define BINARY_START(name) _binary_##name##_start
#define BINARY_END(name) _binary_##name##_end
#define BINARY_RESOURCE(name) _binary_##name##_resource

#define BINARY_DECLARE(name) \
extern char BINARY_START(name)[]; extern char BINARY_END(name)[]; \
resource BINARY_RESOURCE(name) = {#name, BINARY_START(name), (int) (BINARY_END(name) - BINARY_START(name))};

#define EXTERN_BINARY_DECLARE(name) extern resource BINARY_RESOURCE(name);
#define GET_RESOURCE(name) BINARY_RESOURCE(name)

#endif