#version 100

precision highp float;

uniform sampler2D in_texture;

uniform float iteration;

varying vec2 start_z;
varying vec2 tex_coords;

uniform vec2 julia_c;
uniform bool draw_julia;

uniform float power;

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
	vec4 in_color = texture2D(in_texture, tex_coords);

	vec2 z = in_color.xy;

	if (dot(z, z) > 4.0) {
		gl_FragColor = in_color;
	} else {
		float a = z.x;
		float b = z.y;

		if (power == 1.0) {
			gl_FragColor.xy = z;
		} else if (power == 2.0) {
			gl_FragColor.x = a*a - b*b;
			gl_FragColor.y = 2.0 * a * b;
		} else {
			float radius = length(z);
			float theta = atan2(b, a);

			radius = pow(radius, power);

			gl_FragColor.x = radius * cos(power * theta);
			gl_FragColor.y = radius * sin(power * theta);
		}

		gl_FragColor.xy += draw_julia ? julia_c : start_z;
		gl_FragColor.z = iteration;
		gl_FragColor.w = 0.0;
	}

	// if (length(in_color.xy) > 2.0) {
	// 	gl_FragColor = in_color;
	// } else {
	// 	float a = in_color.x;
	// 	float b = in_color.y;
	// 	gl_FragColor.xy = vec2(a*a - b*b, 2.0*a*b);
	// 	//gl_FragColor.xy = vec2(a*a*a - 3.0*a*b*b, 3.0*a*a*b - b*b*b);
	// 	//gl_FragColor.xy = vec2(a*a*a*a - 6.0*a*a*b*b + b*b*b*b, 4.0*a*a*a*b - 4.0*a*b*b*b);
	// 	if (draw_julia) {
	// 		gl_FragColor.xy += julia_c;
	// 	} else {
	// 		gl_FragColor.xy += start_z;
	// 	}
	// 	gl_FragColor.z = iteration;
	// 	gl_FragColor.w = 0.0;
	// }

	//DEBUG
	//gl_FragColor = in_color;
}
