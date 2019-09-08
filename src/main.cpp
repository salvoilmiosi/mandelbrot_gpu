#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <math.h>

#include "shader.h"
#include "texture.h"
#include "bitmap.h"
#include "display.h"

const char *WINDOW_TITLE = "Mandelbrot";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

int current_tex = 0;

bool vsync = false;
bool save_tex = false;

enum { NumIterations = 1 };

texture tex1, tex2, tex_out, palette;
framebuffer fbo1(tex1), fbo2(tex2), fbo3(tex_out);


void error_callback(int error, const char *description) {
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

int check_gl_error(const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error %x %s\n", err, msg);
	}
	return err;
}

uint32_t color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) {
	return ((r & 0xff) |
			((g & 0xff) << 8) |
			((b & 0xff) << 16) |
			((a & 0xff) << 24));
}

uint32_t color_fade(uint32_t color_a, uint32_t color_b, float amt) {
	uint8_t ar = color_a & 0xff;
	uint8_t ag = (color_a & 0xff00) >> 8;
	uint8_t ab = (color_a & 0xff0000) >> 16;
	uint8_t aa = (color_a & 0xff000000) >> 24;

	uint8_t br = color_b & 0xff;
	uint8_t bg = (color_b & 0xff00) >> 8;
	uint8_t bb = (color_b & 0xff0000) >> 16;
	uint8_t ba = (color_b & 0xff000000) >> 24;

	uint8_t cr = (br - ar) * amt + ar;
	uint8_t cg = (bg - ag) * amt + ag;
	uint8_t cb = (bb - ab) * amt + ab;
	uint8_t ca = (ba - aa) * amt + aa;

	return color_rgba(cr, cg, cb, ca);
}

int create_palette() {
	static const uint32_t rainbow[] = {
		color_rgba(0x00,0x00,0xff),
		color_rgba(0x00,0xff,0xff),
		color_rgba(0x00,0xff,0x00),
		color_rgba(0xff,0xff,0x00),
		color_rgba(0xff,0x00,0x00),
		color_rgba(0xff,0x00,0xff)
	};
	static const int N_RAINBOW = sizeof(rainbow) / sizeof(*rainbow);
	static const int N_COLORS = 64;
	uint32_t colors[N_COLORS];

	colors[0] = color_rgba(0xff,0xff,0xff);
	colors[N_COLORS-1] = color_rgba(0xff,0xff,0xff);
	for (int i=1; i<N_COLORS-1; ++i) {
		colors[i] = color_rgba(rand()%0xff, rand()%0xff, rand()%0xff);
		colors[i] = color_fade(colors[i], rainbow[(i-1)%N_RAINBOW], 0.6);
		//colors[i] = color_fade(colors[i], color_rgba(0xff,0xff,0xff), pow(i / (float)N_COLORS, 5));
	}

	return palette.create_texture(N_COLORS, 1, GL_RGBA, GL_UNSIGNED_BYTE, colors);
}

int create_textures() {
	if (tex1.create_texture(display::width, display::height, GL_RGBA32F, GL_FLOAT) != 0) return 1;
	if (tex2.create_texture(display::width, display::height, GL_RGBA32F, GL_FLOAT) != 0) return 2;
	if (tex_out.create_texture(display::width, display::height, GL_RGBA, GL_UNSIGNED_BYTE) != 0) return 3;
	return 0;
}

int init_gl() {
	if (program_init.compile() != 0) return 1;
	if (program_step.compile() != 0) return 1;
	if (program_draw.compile() != 0) return 1;
	if (program_final.compile() != 0) return 1;

	program_init.add_uniforms(center, scale, ratio);
	program_step.add_uniforms(center, scale, ratio, in_texture, iteration, julia_c, draw_julia, z_power);
	program_draw.add_uniforms(center, scale, ratio, in_texture, outside_palette, log_multiplier, log_shift);
	program_final.add_uniforms(center, scale, ratio, in_texture, julia_c);

	srand(time(0));

	if (create_palette() != 0) return 2;
	if (create_textures() != 0) return 2;

	return 0;
}

inline void redraw_mandelbrot() {
	current_tex = 0;
	iteration = 0.f;
}

inline void reset_mandelbrot() {
	center = vec2(0.f, 0.f);
	scale = 1.5f;

	redraw_mandelbrot();
}

int save_screenshot() {
	const char *filename = "screenshot.ppm";

	size_t bufsize = display::width * display::height * 3;
	GLubyte *data = (GLubyte*) malloc(bufsize);

	if (!data) {
		fprintf(stderr, "Not enough memory to save a screenshot");
		return 1;
	}

	tex_out.bind(1);

	glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, bufsize, data);

	if (check_gl_error("Could not read texture") == 0) {
		if (save_ppm(filename, data, display::width, display::height) > 0) {
			printf("Saved screenshot to %s\n", filename);
			return 0;
		}
		return 2;
	}
	return 3;

	free(data);
}

void render() {
	palette.bind(0);

	tex1.bind(1);
	tex2.bind(2);
	tex_out.bind(3);

	ratio = (float) display::width / display::height;

	if (current_tex == 0) {
		fbo1.bind();
		program_init.bind();
		mesh1.draw();

		current_tex = 1;
	}

	(current_tex == 1 ? fbo2 : fbo1).bind();

	in_texture = current_tex;

	program_step.bind();
	mesh1.draw();

	current_tex = current_tex == 1 ? 2 : 1;
	iteration.value_float += 1.f;

	fbo_out.bind();
	program_draw.bind();
	mesh1.draw();

	if (save_tex) {
		save_tex = false;
		save_screenshot();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	in_texture = 3;
	program_final.bind();
	mesh1.draw();
}

void move_c_polar(float x_mul, float y_mul) {
	float theta = atan2(julia_c.value_vec2.y, julia_c.value_vec2.x);
	float radius = sqrt(julia_c.value_vec2.x * julia_c.value_vec2.x + julia_c.value_vec2.y * julia_c.value_vec2.y);
	theta -= x_mul * 0.01;
	radius *= 1.0 + y_mul * 0.01;
	julia_c = vec2(cos(theta) * radius, sin(theta) * radius);
}

void key_callback(int key, int action) {
	if (action == GLFW_PRESS) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			redraw_mandelbrot();
			break;
		case GLFW_KEY_J:
			draw_julia = !(bool)draw_julia;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_R:
			reset_mandelbrot();
			break;
		case GLFW_KEY_G:
			vsync = !vsync;
			glfwSwapInterval(vsync);
			break;
		case GLFW_KEY_PAGE_UP:
			scale.value_float *= 0.5f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_PAGE_DOWN:
			scale.value_float *= 2.f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_P:
			printf("center = (%f, %f)\nscale = %f\njulia_c = (%f, %f)\nz_power=%f\n",
				center.value_vec2.x, center.value_vec2.y, (float)scale, julia_c.value_vec2.x, julia_c.value_vec2.y, (float)z_power);
			break;
		case GLFW_KEY_B:
			printf("log_multipler = %f\nlog_shift = %f\n", (float) log_multiplier, (float) log_shift);
			break;
		case GLFW_KEY_N:
			create_palette();
			break;
		case GLFW_KEY_F12:
			save_tex = true;
			break;
		}
	}
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		switch(key) {
		case GLFW_KEY_Z:
			log_multiplier.value_float /= 1.02f;
			break;
		case GLFW_KEY_X:
			log_multiplier.value_float *= 1.02f;
			break;
		case GLFW_KEY_C:
			log_shift.value_float -= 0.1f;
			if ((float)log_shift < 1.0) {
				log_shift = 1.0f;
			}
			break;
		case GLFW_KEY_V:
			log_shift.value_float += 0.1f;
			break;
		case GLFW_KEY_W:
			center.value_vec2.y += 0.1 * (float)scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_A:
			center.value_vec2.x -= 0.1 * (float)scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_S:
			center.value_vec2.y -= 0.1 * (float)scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_D:
			center.value_vec2.x += 0.1 * (float)scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_K:
			z_power.value_float -= 1.0f;
			if ((float)z_power < 1.0) {
				z_power = 1.0f;
			}
			redraw_mandelbrot();
			break;
		case GLFW_KEY_L:
			z_power.value_float += 1.0f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_UP:
			move_c_polar(0.0, 1.0);
			redraw_mandelbrot();
			break;
		case GLFW_KEY_LEFT:
			move_c_polar(-1.0, 0.00);
			redraw_mandelbrot();
			break;
		case GLFW_KEY_DOWN:
			move_c_polar(0.0, -1.0);
			redraw_mandelbrot();
			break;
		case GLFW_KEY_RIGHT:
			move_c_polar(1.0, 0.0);
			redraw_mandelbrot();
			break;
		}
	}
}

vec2 mouse_pt_to_scale(float mouse_x, float mouse_y) {
	vec2 pt = center;
	float scale_len = 2 * (float)scale / display::height;

	float dx_to_center = mouse_x - (display::width / 2);
	float dy_to_center = mouse_y - (display::height / 2);

	pt.x += dx_to_center * scale_len;
	pt.y -= dy_to_center * scale_len;

	return pt;
}

void mouse_callback(int button, int action, int mouse_x, int mouse_y) {
	static float press_x, press_y;

	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
		switch (action) {
		case GLFW_PRESS:
			press_x = mouse_x;
			press_y = mouse_y;
			break;
		case GLFW_RELEASE:
		{
			center = mouse_pt_to_scale(press_x, press_y);

			if (mouse_x != press_x || mouse_y != press_y) {
				float scale_len = 2 * (float)scale / display::height;

				float dx = mouse_x - press_x;
				float dy = mouse_y - press_y;

				scale = sqrt(dx*dx + dy*dy) * scale_len;
			}

			redraw_mandelbrot();
			break;
		}
		}
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
		switch (action) {
		case GLFW_PRESS:
			julia_c = mouse_pt_to_scale(mouse_x, mouse_y);
			draw_julia = true;
			redraw_mandelbrot();
			break;
		}
		break;
	}
}

void resize_callback() {
	create_textures();
	redraw_mandelbrot();
}

int main(int argc, char**argv) {
	if (!glfwInit()) return 1;

	glfwSetErrorCallback(error_callback);

	if (display::init_display(WINDOW_TITLE, display::width, display::height) != 0) {
		glfwTerminate();
		return 3;
	}

	if (init_gl() != 0) {
		glfwTerminate();
		return 4;
	}

	reset_mandelbrot();

	while (!glfwWindowShouldClose(window)) {
		for (int i=0; i<NumIterations; ++i) {
			render();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	display::cleanup_display();

	glfwTerminate();
	return 0;
}
