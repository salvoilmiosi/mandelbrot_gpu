#include "sources.h"

/// VERTEX SHADER ///

const char *const SOURCE_VERTEX =
R"SOURCE_VERTEX(#version 100

precision mediump float;

attribute vec2 position;

varying vec2 point_c;
varying vec2 tex_coords;

uniform vec2 center;
uniform float scale;
uniform float ratio;

void main() {
	gl_Position = vec4(position, 0.0, 1.0);

	point_c.x = position.x * ratio * scale + center.x;
	point_c.y = position.y * scale + center.y;

	tex_coords = position * 0.5 + 0.5;
}
)SOURCE_VERTEX";

/// INIT FRAGMENT SHADER ///

const char *const SOURCE_INIT =
R"SOURCE_INIT(#version 100

precision mediump float;

varying vec2 point_c;

void main() {
	gl_FragColor = vec4(point_c, 0.0, 0.0);
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)SOURCE_INIT";

/// STEP FRAGMENT SHADER ///

const char *const SOURCE_STEP =
R"SOURCE_STEP(#version 100

precision mediump float;

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
}
)SOURCE_STEP";

/// DRAW FRAGMENT SHADER ///

const char *const SOURCE_DRAW =
R"SOURCE_DRAW(#version 100

precision mediump float;

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
		gl_FragColor = texture2D(outside_palette, vec2(0.4 + in_color.z / max_iterations, 0.0));
	}
	gl_FragColor = vec4(in_color.xyz, 1.0);
}
)SOURCE_DRAW";


