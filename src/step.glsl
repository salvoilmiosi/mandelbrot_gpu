#version 100

precision highp float;

uniform sampler2D in_texture;

uniform float iteration;

varying vec2 point_c;
varying vec2 tex_coords;

void main() {
	vec4 in_color = texture2D(in_texture, tex_coords);

	vec2 z = in_color.xy;

	if (dot(z, z) > 4.0) {
		gl_FragColor = in_color;
	} else {
		gl_FragColor.xy = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y) + point_c;
		gl_FragColor.z = iteration;
		gl_FragColor.w = 0.0;
	}

	//DEBUG
	//gl_FragColor = in_color;
}
