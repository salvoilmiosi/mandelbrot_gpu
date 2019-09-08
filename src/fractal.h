#ifndef __FRACTAL_H__
#define __FRACTAL_H__

#include "shader.h"

DECLARE_SHADER_EXTERN(vertex);
DECLARE_SHADER_EXTERN(init);
DECLARE_SHADER_EXTERN(step);
DECLARE_SHADER_EXTERN(draw);
DECLARE_SHADER_EXTERN(final);

SHADER_OBJECT(GL_VERTEX_SHADER, vertex);
SHADER_OBJECT(GL_FRAGMENT_SHADER, init);
SHADER_OBJECT(GL_FRAGMENT_SHADER, step);
SHADER_OBJECT(GL_FRAGMENT_SHADER, draw);
SHADER_OBJECT(GL_FRAGMENT_SHADER, final);

class fractal {
private:

    shader_program program_init(SHADER(vertex), SHADER(init));
    shader_program program_step(SHADER(vertex), SHADER(step));
    shader_program program_draw(SHADER(vertex), SHADER(draw));
    shader_program program_final(SHADER(vertex), SHADER(final));

    DECLARE_UNIFORM(in_texture, 1);				// sampler for input texture in shaders
    DECLARE_UNIFORM(outside_palette, 0);		// sampler for palette texture
    DECLARE_UNIFORM(iteration, 0.0);			// number of iteration in algorithm
    DECLARE_UNIFORM(center, vec2(0.0, 0.0));	// center of rendering
    DECLARE_UNIFORM(scale, 1.5); 				// scale of rendering
    DECLARE_UNIFORM(ratio, 1.0);				// window width / window height
    DECLARE_UNIFORM(julia_c, vec2(-0.4, 0.6));	// C constant in Julia fractal
    DECLARE_UNIFORM(z_power, 2.0);				// Power of Z in algorithm
    DECLARE_UNIFORM(draw_julia, false);			// true:render Mandelbrot set - false:render Julia set
    DECLARE_UNIFORM(log_multiplier, 0.3);		// Shifts the "inner" colors in the draw shader
    DECLARE_UNIFORM(log_shift, 9.0);			// Shifts the "outer" colors in the draw shader

public:
    void fractal();
    ~fractal();
};

#endif