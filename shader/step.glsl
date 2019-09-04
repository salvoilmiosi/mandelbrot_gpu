#version 100

precision highp float;

uniform sampler2D in_texture;

uniform float iteration;

varying vec2 point_c;
varying vec2 tex_coords;

uniform vec2 point_c_const;
uniform bool draw_julia;

const float power = 2.0;

#define PI 3.1415926538

float atan2(float y, float x) {
	if (x > 0.0) {
		return atan(y/x);
	} else if (x < 0.0) {
		return y > 0.0 ? atan(y/x) + PI : atan(y/x) - PI;
	} else {
		return y > 0.0 ? PI*0.5 : -PI*0.5;
	}
}

void main() {
	vec4 in_color = texture2D(in_texture, tex_coords);

	float radius = length(point_c);
	float theta = atan2(point_c.y, point_c.x);

	if (radius > 2.0) {
		gl_FragColor = in_color;
	} else {
		gl_FragColor.x = pow(radius, power) * cos(power * theta);
		gl_FragColor.y = pow(radius, power) * sin(power * theta);
		if (draw_julia) {
			gl_FragColor.xy += point_c_const;
		} else {
			gl_FragColor.xy += point_c;
		}
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
	// 		gl_FragColor.xy += point_c_const;
	// 	} else {
	// 		gl_FragColor.xy += point_c;
	// 	}
	// 	gl_FragColor.z = iteration;
	// 	gl_FragColor.w = 0.0;
	// }

	//DEBUG
	//gl_FragColor = in_color;
}
