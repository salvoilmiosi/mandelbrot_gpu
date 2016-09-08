#version 330

uniform sampler2D in_texture;

uniform float iteration;

in vec2 point_c;
in vec2 tex_coords;

out vec4 out_color;

void main() {
    vec4 in_color = texture2D(in_texture, tex_coords);

    vec2 z = in_color.xy;

    if (dot(z, z) > 4.0) {
        out_color = in_color;
    } else {
        out_color.xy = vec2(z.x*z.x - z.y*z.y, 2*z.x*z.y) + point_c;
        out_color.z = iteration;
        out_color.w = 0.0;
    }
}
