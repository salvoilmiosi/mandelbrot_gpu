#version 100

precision highp float;

uniform sampler2D in_texture;
uniform sampler2D outside_palette;

const vec4 color_inside = vec4(0.0, 0.0, 0.0, 1.0);
const float max_iterations = 256.0;

varying vec2 point_c;
varying vec2 tex_coords;

void main() {
	vec4 in_color = texture2D(in_texture, tex_coords);
	vec2 point_z = in_color.xy;

	if (dot(point_z, point_z) < 4.0) {
		gl_FragColor = color_inside;
	} else {
		gl_FragColor = texture2D(outside_palette, vec2(0.2 + in_color.z / max_iterations, 0.0));
	}

	//DEBUG
	//gl_FragColor = vec4(in_color.xyz, 1.0);
	//gl_FragColor = vec4(point_c, 0.0, 1.0);
}
