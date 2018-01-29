#version 100

precision highp float;

uniform sampler2D in_texture;
uniform sampler2D outside_palette;
uniform float log_coeff;

const vec4 color_inside = vec4(0.0, 0.0, 0.0, 1.0);
const float max_iterations = 1024.0;

varying vec2 point_c;
varying vec2 tex_coords;

void main() {
	vec4 in_color = texture2D(in_texture, tex_coords);
	vec2 point_z = in_color.xy;

	if (dot(point_z, point_z) < 4.0) {
		gl_FragColor = color_inside;
	} else {
		float color_i = 1.0 / (1.0 + log_coeff * log(1.0 + in_color.z));
		gl_FragColor = texture2D(outside_palette, vec2(color_i, 0.0));
	}

	//DEBUG
	//gl_FragColor = vec4(in_color.xyz, 1.0);
	//gl_FragColor = vec4(point_c, 0.0, 1.0);
}
