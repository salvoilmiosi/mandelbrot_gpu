#version 330

precision highp float;

layout(location=0) in vec2 position;

out vec2 start_z;
out vec2 tex_coords;

uniform vec2 center;
uniform float scale;
uniform float ratio;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);

	start_z.x = position.x * ratio * scale + center.x;
	start_z.y = position.y * scale + center.y;

	tex_coords = position * 0.5 + 0.5;
}
