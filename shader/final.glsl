#version 100

precision highp float;

uniform sampler2D in_texture;

varying vec2 tex_coords;

void main() {
    gl_FragColor = texture2D(in_texture, vec2(tex_coords.x, 1.0 - tex_coords.y));
    //gl_FragColor = texture2D(in_texture, tex_coords);
}
