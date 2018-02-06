#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <vector>
#include <cmath>

#include "sources.h"
#include "program.h"
#include "render_target.h"

static const char *WINDOW_TITLE = "Mandelbrot";
static const int N_COLORS = 256;

static int window_width = 800;
static int window_height = 600;

GLuint vao;
GLuint vbo;

vertex_shader shader_vertex("vertex");
frag_shader shader_init("init");
frag_shader shader_step("step");
frag_shader shader_draw("draw");

program program_init("init");
program program_step("step");
program program_draw("draw");

texture palette(N_COLORS, 1);

std::vector<texture> textures;
std::vector<render_target> targets;

int current_tex = 0;
float iteration = 0.f;

vec2 center;
float scale;
float log_coeff = 0.03f;

static uint32_t color_rgba(int r, int g, int b, int a = 0xff) {
	return ((r & 0xff) |
		   ((g & 0xff) << 8) |
		   ((b & 0xff) << 16) |
		   ((a & 0xff) << 24));
}

static void create_palette() {
	uint32_t colors[N_COLORS];
	for (int i=0; i<N_COLORS; ++i) {
		int red   = sin(i * 0.3) * 0x80 + 0x80;
		int green = sin(i * 0.2) * 0x80 + 0x80;
		int blue  = sin(i * 0.5) * 0x80 + 0x80;
		colors[i] = color_rgba(red, green, blue);
	}

	palette.attachPixels(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, colors);
	palette.setFilter(GL_LINEAR);
}

static int next_pow_2(int num) {
	int n = 1;
	while (n < num) {
		n *= 2;
	}
	return n;
}

static void create_textures() {
	textures.clear();
	targets.clear();

	int tex_width = next_pow_2(window_width);
	int tex_height = next_pow_2(window_height);
	for (int i=0; i<2; ++i) {
		texture &tex = textures.emplace_back(tex_width, tex_height);
		tex.attachPixels(GL_RGBA32F, GL_RGBA, GL_FLOAT, nullptr);
		targets.emplace_back(tex);
	}
}

static int initGL() {
	if (shader_vertex.loadSource(GET_SHADER(vertex)) != 0) return 1;
	if (shader_init.loadSource(GET_SHADER(init)) != 0) return 1;
	if (shader_step.loadSource(GET_SHADER(step)) != 0) return 1;
	if (shader_draw.loadSource(GET_SHADER(draw)) != 0) return 1;

	program_init.attachAndLink(shader_vertex, shader_init);
	program_step.attachAndLink(shader_vertex, shader_step);
	program_draw.attachAndLink(shader_vertex, shader_draw);

	create_palette();
	create_textures();

	palette.bindToSampler(0);

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

	GLint position = program_init.getAttribLoc("position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	return 0;
}

static void reset_mandelbrot() {
	center = {-0.f, 0.f};
	scale = 1.2f;
}

static void redraw_mandelbrot() {
	current_tex = 0;
	iteration = 0.f;
	float ratio = (float) window_width / window_height;

	program_init.setUniform("center", center);
	program_init.setUniform("scale", scale);
	program_init.setUniform("ratio", ratio);

	program_step.setUniform("center", center);
	program_step.setUniform("scale", scale);
	program_step.setUniform("ratio", ratio);

	program_draw.setUniform("center", center);
	program_draw.setUniform("scale", scale);
	program_draw.setUniform("ratio", ratio);
}

static void error_callback(int error, const char *description) {
	std::cerr << "GLFW error " << error << " : " << description << std::endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
	switch (action) {
	case GLFW_PRESS:
		switch (key) {
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			redraw_mandelbrot();
			break;
		case GLFW_KEY_R:
			reset_mandelbrot();
			redraw_mandelbrot();
			break;
		case GLFW_KEY_PAGE_UP:
			scale *= 0.5f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_PAGE_DOWN:
			scale *= 2.f;
			redraw_mandelbrot();
			break;
		case GLFW_KEY_UP:
			log_coeff += 0.01f;
			break;
		case GLFW_KEY_DOWN:
			log_coeff -= 0.01f;
			break;
		default:
			break;
		}
		break;
	}
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
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
			double scale_len = 2 * scale / window_height;

			double dx_to_center = press_x - (window_width / 2);
			double dy_to_center = press_y - (window_height / 2);

			center.x += (double) dx_to_center * scale_len;
			center.y -= (double) dy_to_center * scale_len;

			double dx_to_start = press_x < mouse_x ? mouse_x - press_x : press_x - mouse_x;
			double dy_to_start = press_y < mouse_y ? mouse_y - press_y : press_y - mouse_y;

			scale = dx_to_start > dy_to_start ? dx_to_start * scale_len : dy_to_start * scale_len;

			redraw_mandelbrot();
			break;
		}
		default:
			break;
		}
		break;
	}
}

static void resize_callback(GLFWwindow *window, int width, int height) {
	window_width = width;
	window_height = height;
	create_textures();
	redraw_mandelbrot();
}

static void render() {
	switch (current_tex) {
	case 0:
		program_init.bind();
		targets[0].bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 1;
		break;
	case 1:
		program_step.bind();
		program_step.setUniform("in_texture", 1);
		program_step.setUniform("iteration", iteration);
		textures[0].bindToSampler(1);
		targets[1].bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 2;
		break;
	case 2:
		program_step.bind();
		program_step.setUniform("in_texture", 1);
		program_step.setUniform("iteration", iteration);
		textures[1].bindToSampler(1);
		targets[0].bind();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 1;
		break;
	}

	iteration += 1.f;

	program_draw.bind();
	program_draw.setUniform("in_texture", 1);
	program_draw.setUniform("outside_palette", 0);
	program_draw.setUniform("log_coeff", log_coeff);
	//textures[current_tex - 1].bind();

	render_target::bindScreen(window_width, window_height);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void cleanupGL() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

GLFWwindow *createTheWindow() {
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

	GLFWwindow *window = createTheWindow();

	if (!window) {
		glfwTerminate();
		return 3;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(0);

	glewExperimental = true;
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		std::cerr << "GLEW error " << error << " : " << glewGetErrorString(error) << std::endl;
		glfwTerminate();
		return 2;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetWindowSizeCallback(window, resize_callback);

	if (initGL() != 0) {
		glfwTerminate();
		return 4;
	}

	reset_mandelbrot();
	redraw_mandelbrot();

	while (!glfwWindowShouldClose(window)) {
		render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanupGL();

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}
