#version 330

precision highp float;

layout(location=0) out vec4 fragColor;

in vec2 start_z;
in vec2 tex_coords;

uniform sampler2D in_texture;
uniform float iteration;
uniform vec2 julia_c;
uniform bool draw_julia;
uniform float z_power;

#define PI 3.1415926538

float atan2(float y, float x) {
	if (x > 0.0) {
		return atan(y/x);
	} else if (x < 0.0) {
		return atan(y/x) + PI;
	} else {
		return y > 0.0 ? PI*0.5 : -PI*0.5;
	}
}

void main() {
	vec4 in_color = texture(in_texture, tex_coords);

	vec2 z = in_color.xy;

	if (dot(z, z) > 4.0) {
		fragColor = in_color;
	} else {
		float a = z.x;
		float b = z.y;

		if (z_power == 1.0) {
			fragColor.xy = z;
		} else if (z_power == 2.0) {
			fragColor.x = a*a - b*b;
			fragColor.y = 2.0 * a * b;
		} else if (z_power == 3.0) {
			fragColor.x = a*a*a - 3.0 * a*b*b;
			fragColor.y = 3.0 * a*a*b - b*b*b;
		} else {
			float radius = length(z);
			float theta = atan2(b, a);

			radius = pow(radius, z_power);

			fragColor.x = radius * cos(z_power * theta);
			fragColor.y = radius * sin(z_power * theta);
		}

		fragColor.xy += draw_julia ? julia_c : start_z;
		fragColor.z = iteration;
		fragColor.w = 0.0;
	}

	//DEBUG
	//gl_FragColor = in_color;
}
