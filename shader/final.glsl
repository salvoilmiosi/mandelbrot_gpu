#version 100

precision highp float;

uniform sampler2D in_texture;

varying vec2 point_c;
uniform vec2 point_c_const;

varying vec2 tex_coords;

const vec4 point_color = vec4(1.0, 1.0, 1.0, 1.0);
const float multiplier = 50000.0;

void main() {
    gl_FragColor = texture2D(in_texture, vec2(tex_coords.x, 1.0 - tex_coords.y));

    float d = distance(point_c, point_c_const);
    float amt = 1.0 / (multiplier * d * d + 1.0);

    gl_FragColor = mix(gl_FragColor, point_color, amt);

    //gl_FragColor = texture2D(in_texture, tex_coords);
}
