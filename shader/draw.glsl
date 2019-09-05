#version 330

precision highp float;

layout(location=0) out vec4 fragColor;

in vec2 start_z;
in vec2 tex_coords;

uniform sampler2D in_texture;
uniform sampler2D outside_palette;
uniform float log_multiplier;
uniform float log_shift;

const vec4 color_inside = vec4(0.0, 0.0, 0.0, 1.0);

void main() {
	vec4 in_color = texture(in_texture, vec2(tex_coords.x, 1.0 - tex_coords.y));
	vec2 point_z = in_color.xy;

	if (dot(point_z, point_z) < 4.0) {
		fragColor = color_inside;
	} else {
		float color_i = 1.0 - 1.0 / (1.0 + log_multiplier * log(log_shift + in_color.z));
		fragColor = texture(outside_palette, vec2(color_i, 0.0));
	}

	//DEBUG
	//gl_FragColor = vec4(in_color.xyz, 1.0);
	//gl_FragColor = vec4(start_z, 0.0, 1.0);
	//gl_FragColor = texture2D(outside_palette, tex_coords);
}
