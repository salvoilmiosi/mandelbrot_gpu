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

int tex_width;
int tex_height;

int current_tex = 0;
float iteration = 0.f;

const int NUM_ITERATIONS = 1;

vec2 center;
float scale;
float alg_power = 2.0;
bool draw_julia = false;
bool vsync = false;
bool save_tex = false;

vec2 julia_c = {-0.4, 0.6};

float log_multiplier = 0.3;
float log_shift = 9.0;

GLuint vao;
GLuint vbo;

shader_program program_init("init");
shader_program program_step("step");
shader_program program_draw("draw");
shader_program program_final("final");

GLuint palette = 0; // palette texture

texture_io tex1, tex2, tex_out;

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

void cleanup_gl() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	glDeleteTextures(1, &palette);
}

inline double get_ratio() {
	return (double) window_width / window_height;
}

static uint32_t color_rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) {
	return ((r & 0xff) |
			((g & 0xff) << 8) |
			((b & 0xff) << 16) |
			((a & 0xff) << 24));
}

static uint32_t color_fade(uint32_t color_a, uint32_t color_b, float amt) {
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

static int create_palette() {
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

	if (!palette) {
		glGenTextures(1, &palette);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, palette);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, palette);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N_COLORS, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);

	return 0;
}

static int next_pow_2(int num) {
	int n = 1;
	while (n < num) {
		n *= 2;
	}
	return n;
}

static bool create_textures() {
	tex_width = next_pow_2(window_width);
	tex_height = next_pow_2(window_height);
	if (tex1.create_texture(tex_width, tex_height, GL_RGBA32F, GL_FLOAT) != 0) return false;
	if (tex2.create_texture(tex_width, tex_height, GL_RGBA32F, GL_FLOAT) != 0) return false;
	if (tex_out.create_texture(tex_width, tex_height, GL_RGBA, GL_UNSIGNED_BYTE) != 0) return false;
	return true;
}

int init_gl() {
	if (program_init.create_program(GET_SHADER(vertex), GET_SHADER(init)) != 0) return 1;
	if (program_step.create_program(GET_SHADER(vertex), GET_SHADER(step)) != 0) return 1;
	if (program_draw.create_program(GET_SHADER(vertex), GET_SHADER(draw)) != 0) return 1;
	if (program_final.create_program(GET_SHADER(vertex), GET_SHADER(final)) != 0) return 1;

	srand(time(0));

	if (create_palette() != 0) return 2;
	if (!create_textures()) return 2;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, palette);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	float vertices[] = {
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

static void reset_mandelbrot() {
	center = {0.f, 0.f};
	scale = 1.5f;
}

static void redraw_mandelbrot() {
	current_tex = 0;
	iteration = 0.f;
}

static int save_screenshot(const char *filename) {
	size_t bufsize = tex_width * tex_height * 3;
	GLubyte *data = (GLubyte*) malloc(bufsize);

	if (!data) {
		fprintf(stderr, "Not enough memory to save a screenshot");
		return 1;
	}

	tex_out.bind_texture();
	glGetnTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, bufsize, data);

	if (check_gl_error("Could not read texture") == 0) {
		if (save_bmp(filename, data, tex_width, tex_height) == 0) {
			return 0;
		}
		return 2;
	}
	return 3;

	free(data);
}

static void render() {
	glViewport(0, 0, tex_width, tex_height);

	glActiveTexture(GL_TEXTURE1);

	if (current_tex==0) {
		tex1.bind_framebuffer();
		program_init.use_program();
		program_init.set_uniform_vec2("center", center);
		program_init.set_uniform_f("scale", scale);
		program_init.set_uniform_f("ratio", get_ratio());

		glBindTexture(GL_TEXTURE_2D, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		current_tex = 1;
	}

	(current_tex == 1 ? tex1 : tex2).bind_texture();
	(current_tex == 1 ? tex2 : tex1).bind_framebuffer();

	program_step.use_program();
	program_step.set_uniform_vec2("julia_c", julia_c);
	program_step.set_uniform_vec2("center", center);
	program_step.set_uniform_f("iteration", iteration);
	program_step.set_uniform_f("scale", scale);
	program_step.set_uniform_f("ratio", get_ratio());
	program_step.set_uniform_f("power", alg_power);
	program_step.set_uniform_i("draw_julia", draw_julia);
	program_step.set_uniform_i("in_texture", 1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	current_tex = current_tex == 1 ? 2 : 1;
	iteration += 1.f;

	tex_out.bind_framebuffer();
	(current_tex == 1 ? tex1 : tex2).bind_texture();

	program_draw.use_program();
	program_draw.set_uniform_vec2("center", center);
	program_draw.set_uniform_f("log_multiplier", log_multiplier);
	program_draw.set_uniform_f("log_shift", log_shift);
	program_draw.set_uniform_f("scale", scale);
	program_draw.set_uniform_f("ratio", get_ratio());
	program_draw.set_uniform_i("in_texture", 1);
	program_draw.set_uniform_i("outside_palette", 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if (save_tex) {
		save_tex = false;

		const char *filename = "screenshot.data";
		
		if (save_screenshot(filename) == 0) {
			printf("Saved screenshot to %s (%d x %d)\n", filename, tex_width, tex_height);
		}
	}

	glViewport(0, 0, window_width, window_height);

	tex_out.bind_texture();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	program_final.use_program();
	program_final.set_uniform_vec2("julia_c", julia_c);
	program_final.set_uniform_vec2("center", center);
	program_final.set_uniform_f("scale", scale);
	program_final.set_uniform_f("ratio", get_ratio());
	program_final.set_uniform_i("in_texture", 1);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void move_c_polar(float x_mul, float y_mul) {
	double theta = atan2(julia_c.y, julia_c.x);
	double modulo = sqrt(julia_c.x * julia_c.x + julia_c.y * julia_c.y);
	theta -= x_mul * 0.01;
	modulo *= 1.0 + y_mul * 0.01;
	julia_c.x = (float)(cos(theta) * modulo);
	julia_c.y = (float)(sin(theta) * modulo);
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
			draw_julia = !draw_julia;
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
			scale *= 0.5f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_PAGE_DOWN:
			scale *= 2.f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_P:
			printf("center = (%f, %f)\nscale = %f\njulia_c = (%f, %f)\n", center.x, center.y, scale, julia_c.x, julia_c.y);
			break;
		case GLFW_KEY_B:
			printf("log_multipler = %f\nlog_shift = %f\n", log_multiplier, log_shift);
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
			log_multiplier /= 1.1;
			break;
		case GLFW_KEY_X:
			log_multiplier *= 1.1;
			break;
		case GLFW_KEY_C:
			log_shift -= 0.1;
			if (log_shift < 1.0) {
				log_shift = 1.0;
			}
			break;
		case GLFW_KEY_V:
			log_shift += 0.1;
			break;
		case GLFW_KEY_W:
			center.y += 0.1 * scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_A:
			center.x -= 0.1 * scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_S:
			center.y -= 0.1 * scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_D:
			center.x += 0.1 * scale;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_K:
			alg_power -= 1.0;
			if (alg_power < 1.0) {
				alg_power = 1.0;
			}
			redraw_mandelbrot();
			break;
		case GLFW_KEY_L:
			alg_power += 1.0;
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

vec2 mouse_pt_to_scale(double mouse_x, double mouse_y) {
	vec2 pt = center;
	double scale_len = 2 * scale / window_height;

	double dx_to_center = mouse_x - (window_width / 2);
	double dy_to_center = mouse_y - (window_height / 2);

	pt.x += (double) dx_to_center * scale_len;
	pt.y -= (double) dy_to_center * scale_len;

	return pt;
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
	double mouse_x, mouse_y;

	glfwGetCursorPos(window, &mouse_x, &mouse_y);

	static double press_x, press_y;

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
				double scale_len = 2 * scale / window_height;

				double dx_to_start = press_x < mouse_x ? mouse_x - press_x : press_x - mouse_x;
				double dy_to_start = press_y < mouse_y ? mouse_y - press_y : press_y - mouse_y;

				scale = sqrt(dx_to_start*dx_to_start + dy_to_start * dy_to_start) * scale_len;
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
		for (int i=0; i<NUM_ITERATIONS; ++i) {
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
