#version 330

precision highp float;

layout(location=0) out vec4 fragColor;

in vec2 start_z;

void main() {
	fragColor = vec4(start_z, 0.0, 0.0);
}
