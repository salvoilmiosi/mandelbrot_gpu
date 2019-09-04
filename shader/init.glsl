#version 100

precision highp float;

varying vec2 start_z;

void main() {
	gl_FragColor = vec4(start_z, 0.0, 0.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
