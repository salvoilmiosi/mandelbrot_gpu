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

const char *WINDOW_TITLE = "Mandelbrot";

int window_width = 800;
int window_height = 600;

int current_tex = 0;

bool vsync = false;
bool save_tex = false;

enum { NumIterations = 1 };

GLuint palette = 0; // palette texture

texture_io tex1, tex2, tex_out;

GLuint vao = 0;
GLuint vbo = 0;

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

DECLARE_SHADER(GL_VERTEX_SHADER, vertex);
DECLARE_SHADER(GL_FRAGMENT_SHADER, init);
DECLARE_SHADER(GL_FRAGMENT_SHADER, step);
DECLARE_SHADER(GL_FRAGMENT_SHADER, draw);
DECLARE_SHADER(GL_FRAGMENT_SHADER, final);

shader_program program_init(SHADER(vertex), SHADER(init));
shader_program program_step(SHADER(vertex), SHADER(step));
shader_program program_draw(SHADER(vertex), SHADER(draw));
shader_program program_final(SHADER(vertex), SHADER(final));

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

	if (!palette) glGenTextures(1, &palette);

	glBindTexture(GL_TEXTURE_2D, palette);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N_COLORS, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return 0;
}

int next_pow_2(int num) {
	int n = 1;
	while (n < num) {
		n *= 2;
	}
	return n;
}

int create_textures() {
	int tex_width = next_pow_2(window_width);
	int tex_height = next_pow_2(window_height);
	if (tex1.create_texture(tex_width, tex_height, GL_RGBA32F, GL_FLOAT) != 0) return 1;
	if (tex2.create_texture(tex_width, tex_height, GL_RGBA32F, GL_FLOAT) != 0) return 2;
	if (tex_out.create_texture(tex_width, tex_height, GL_RGBA, GL_UNSIGNED_BYTE) != 0) return 3;
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

	const float vertices[] = {
		-1.0,  1.0,
		-1.0, -1.0,
		 1.0,  1.0,
		 1.0, -1.0,
	};

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	return 0;
}

void cleanup_gl() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDeleteTextures(1, &palette);
}

inline void reset_mandelbrot() {
	center = vec2(0.f, 0.f);
	scale = 1.5f;
}

inline void redraw_mandelbrot() {
	current_tex = 0;
	iteration = 0.f;
}

int save_screenshot(const char *filename) {
	size_t bufsize = tex_out.width * tex_out.height * 3;
	GLubyte *data = (GLubyte*) malloc(bufsize);

	if (!data) {
		fprintf(stderr, "Not enough memory to save a screenshot");
		return 1;
	}

	tex_out.bind_texture();
	glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, bufsize, data);

	if (check_gl_error("Could not read texture") == 0) {
		if (save_bmp(filename, data, tex_out.width, tex_out.height) == 0) {
			return 0;
		}
		return 2;
	}
	return 3;

	free(data);
}

void render() {
	glViewport(0, 0, tex1.width, tex1.height);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, palette);

	glActiveTexture(GL_TEXTURE1);
	tex1.bind_texture();
	glActiveTexture(GL_TEXTURE2);
	tex2.bind_texture();
	glActiveTexture(GL_TEXTURE3);
	tex_out.bind_texture();

	ratio = (float) window_width / window_height;

	if (current_tex == 0) {
		tex1.bind_framebuffer();
		program_init.use_program();

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		current_tex = 1;
	}

	(current_tex == 1 ? tex2 : tex1).bind_framebuffer();

	in_texture = current_tex;

	program_step.use_program();

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	current_tex = current_tex == 1 ? 2 : 1;
	iteration.value_float += 1.f;

	tex_out.bind_framebuffer();

	program_draw.use_program();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (save_tex) {
		save_tex = false;

		const char *filename = "screenshot.data";
		
		if (save_screenshot(filename) == 0) {
			printf("Saved screenshot to %s (%d x %d)\n", filename, tex_out.width, tex_out.height);
		}
	}

	glViewport(0, 0, window_width, window_height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	in_texture = 3;
	program_final.use_program();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void move_c_polar(float x_mul, float y_mul) {
	float theta = atan2(julia_c.value_vec2.y, julia_c.value_vec2.x);
	float radius = sqrt(julia_c.value_vec2.x * julia_c.value_vec2.x + julia_c.value_vec2.y * julia_c.value_vec2.y);
	theta -= x_mul * 0.01;
	radius *= 1.0 + y_mul * 0.01;
	julia_c = vec2(cos(theta) * radius, sin(theta) * radius);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
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
			redraw_mandelbrot();
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
	float scale_len = 2 * (float)scale / window_height;

	float dx_to_center = mouse_x - (window_width / 2);
	float dy_to_center = mouse_y - (window_height / 2);

	pt.x += dx_to_center * scale_len;
	pt.y -= dy_to_center * scale_len;

	return pt;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	double mouse_x, mouse_y;

	glfwGetCursorPos(window, &mouse_x, &mouse_y);

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
				float scale_len = 2 * (float)scale / window_height;

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

void resize_callback(GLFWwindow *window, int width, int height) {
	window_width = width;
	window_height = height;
	create_textures();
	redraw_mandelbrot();
}

GLFWwindow *create_window() {
	GLFWwindow *window;

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	window = glfwCreateWindow(window_width, window_height, WINDOW_TITLE, nullptr, nullptr);
	if (window) return window;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	
	return glfwCreateWindow(window_width, window_height, WINDOW_TITLE, nullptr, nullptr);
}

int main(int argc, char**argv) {
	if (!glfwInit()) {
		return 1;
	}

	glfwSetErrorCallback(error_callback);

	GLFWwindow *window = create_window();

	if (!window) {
		glfwTerminate();
		return 3;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);

	glewExperimental = true;
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "GLEW error %d: %s\n", error, glewGetErrorString(error));
		glfwTerminate();
		return 2;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, resize_callback);

	if (init_gl() != 0) {
		glfwTerminate();
		return 4;
	}

	reset_mandelbrot();
	redraw_mandelbrot();

	while (!glfwWindowShouldClose(window)) {
		for (int i=0; i<NumIterations; ++i) {
			render();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanup_gl();

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
