#version 330

in vec2 point_c;

out vec4 out_color;

void main() {
    out_color = vec4(point_c, 0.0, 0.0);
}
