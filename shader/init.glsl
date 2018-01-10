#version 100

precision highp float;

varying vec2 point_c;

void main() {
	gl_FragColor = vec4(point_c, 0.0, 0.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
