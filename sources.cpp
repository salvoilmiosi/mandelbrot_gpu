#include "sources.h"

/// VERTEX SHADER ///

const char *const SOURCE_VERTEX =
"#version 330\n"

"layout(location = 0) in vec2 position;"

"out vec2 point_c;"
"out vec2 tex_coords;"

"uniform vec2 center;"
"uniform float scale;"
"uniform float ratio;"

"void main() {"
    "gl_Position = vec4(position, 0.0, 1.0);"

    "point_c.x = position.x * ratio * scale + center.x;"
    "point_c.y = position.y * scale + center.y;"

    "tex_coords = position * 0.5 + 0.5;"

"}";

/// INIT FRAGMENT SHADER ///

const char *const SOURCE_INIT =
"#version 330\n"

"in vec2 point_c;"
"out vec4 out_color;"

"void main() {"
    "out_color = vec4(point_c, 0.0, 0.0);"
"}";

/// STEP FRAGMENT SHADER ///

const char *const SOURCE_STEP =
"#version 330\n"

"uniform sampler2D in_texture;"

"uniform float iteration;"

"in vec2 point_c;"
"in vec2 tex_coords;"

"out vec4 out_color;"

"void main() {"
    "vec4 in_color = texture2D(in_texture, tex_coords);"

    "vec2 z = in_color.xy;"

    "if (dot(z, z) > 4.0) {"
        "out_color = in_color;"
    "} else {"
        "out_color.xy = vec2(z.x*z.x - z.y*z.y, 2*z.x*z.y) + point_c;"
        "out_color.z = iteration;"
        "out_color.w = 0.0;"
    "}"
"}";

/// DRAW FRAGMENT SHADER ///

const char *const SOURCE_DRAW =
"#version 330\n"

"uniform sampler2D in_texture;"
"uniform vec4 color_inside = vec4(0.0, 0.0, 0.0, 1.0);"
"uniform sampler1D outside_palette;"
"uniform float max_iterations = 256.0;"

"in vec2 point_c;"
"in vec2 tex_coords;"
"out vec4 out_color;"

"void main() {"
    "vec4 in_color = texture2D(in_texture, tex_coords);"
    "vec2 point_z = in_color.xy;"

    "if (dot(point_z, point_z) < 4.0) {"
        "out_color = color_inside;"
    "} else {"
        "out_color = texture1D(outside_palette, 0.4 + in_color.z / max_iterations);"
    "}"
"}";


