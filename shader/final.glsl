#version 330

precision highp float;

layout(location=0) out vec4 fragColor;

in vec2 start_z;
in vec2 tex_coords;

uniform sampler2D in_texture;
uniform vec2 julia_c;

const vec4 point_c_color = vec4(1.0, 1.0, 1.0, 1.0);
const float multiplier = 50000.0;

void main() {
    fragColor = texture(in_texture, vec2(tex_coords.x, 1.0 - tex_coords.y));

    float d = distance(start_z, julia_c);
    float amt = 1.0 / (multiplier * d * d + 1.0);

    fragColor = mix(fragColor, point_c_color, amt);

    //gl_FragColor = texture2D(in_texture, tex_coords);
}
