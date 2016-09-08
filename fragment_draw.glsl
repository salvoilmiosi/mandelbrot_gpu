#version 330

uniform sampler2D in_texture;

uniform vec4 color_inside = vec4(0.0, 0.0, 0.0, 1.0);

uniform sampler1D outside_palette;

uniform float max_iterations = 256.0;

uniform float base = 0.99;

in vec2 point_c;

in vec2 tex_coords;

out vec4 out_color;

void main() {
    vec4 in_color = texture2D(in_texture, tex_coords);
    vec2 point_z = in_color.xy;

    if (dot(point_z, point_z) < 4.0) {
        out_color = color_inside;
    } else {
        out_color = texture1D(outside_palette, 0.4 + in_color.z / max_iterations);
    }
}
