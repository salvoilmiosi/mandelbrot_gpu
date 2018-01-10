#version 100

precision highp float;

attribute vec2 position;

varying vec2 point_c;
varying vec2 tex_coords;

uniform vec2 center;
uniform float scale;
uniform float ratio;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);

	point_c.x = position.x * ratio * scale + center.x;
	point_c.y = position.y * scale + center.y;

	tex_coords = position * 0.5 + 0.5;
}
