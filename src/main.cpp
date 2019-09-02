#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <math.h>

#include "sources.h"

static const char *WINDOW_TITLE = "Mandelbrot";

static int window_width = 800;
static int window_height = 600;

static int tex_width;
static int tex_height;

int current_tex = 0;
float iteration = 0.f;

struct vec2 {
	float x;
	float y;
};

vec2 center;
float scale;
bool julia = false;
vec2 point_c_const;

static void reset_mandelbrot();
static void redraw_mandelbrot();

static void error_callback(int error, const char *description) {
	fprintf(stderr, "GLFW error %d: %s\n", error, description);
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
		case GLFW_KEY_J:
			julia = !julia;
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
		}
		break;
	}
}

static vec2 mouse_pt_to_scale(double mouse_x, double mouse_y) {
	vec2 pt = center;
	double scale_len = 2 * scale / window_height;

	double dx_to_center = mouse_x - (window_width / 2);
	double dy_to_center = mouse_y - (window_height / 2);

	pt.x += (double) dx_to_center * scale_len;
	pt.y -= (double) dy_to_center * scale_len;

	return pt;
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
			center = mouse_pt_to_scale(press_x, press_y);

			if (mouse_x == press_x && mouse_y == press_y) {
				scale *= 0.25;
			} else {
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
			point_c_const = mouse_pt_to_scale(mouse_x, mouse_y);
			julia = true;
			redraw_mandelbrot();
			break;
		}
		break;
	}
}

static double get_ratio() {
	return (double) window_width / window_height;
}

GLuint vao;
GLuint vbo;

struct shader_program {
	const char *name;
	GLuint program_id;
	GLuint vertex_shader;
	GLuint fragment_shader;

	explicit shader_program(const char *name) : name(name) {}
};

shader_program program_init("init");
shader_program program_step("step");
shader_program program_draw("draw");

struct texture_io {
	GLuint tex = 0;
	GLuint fbo = 0;
	GLuint rbo = 0;
};

GLuint palette;

texture_io tex1, tex2;

int check_gl_error(const char *msg) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error %d %s\n", err, msg);
	}
	return err;
}

static int check_shader_error(const char *name, const char *type, GLuint shader) {
	GLint compiled = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (compiled != GL_TRUE) {
		int length = 0;
		int max_length = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length);

		char *info_log = new char[max_length];

		glGetShaderInfoLog(shader, max_length, &length, info_log);
		if (length > 0){
			fprintf(stderr, "%s %s Error:\n%s\n", name, type, info_log);
		}

		delete[] info_log;

		return 1;
	}
	return 0;
}

static int create_program(shader_program *program, const char *vertex_source, const char *fragment_source) {
	program->program_id = glCreateProgram();

	program->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	int vertex_source_length = strlen(vertex_source);
	glShaderSource(program->vertex_shader, 1, &vertex_source, &vertex_source_length);
	glCompileShader(program->vertex_shader);

	if (check_shader_error(program->name, "vertex", program->vertex_shader) != 0) {
		return 1;
	}

	glAttachShader(program->program_id, program->vertex_shader);

	program->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	int fragment_source_length = strlen(fragment_source);
	glShaderSource(program->fragment_shader, 1, &fragment_source, &fragment_source_length);
	glCompileShader(program->fragment_shader);

	if (check_shader_error(program->name, "frag", program->fragment_shader) != 0) {
		return 1;
	}

	glAttachShader(program->program_id, program->fragment_shader);

	glLinkProgram(program->program_id);

	return 0;
}

static int create_program(shader_program *program, const std::string &vertex_source, const std::string &fragment_source) {
	return create_program(program, vertex_source.c_str(), fragment_source.c_str());
}

static void free_program(shader_program *program) {
	glDetachShader(program->program_id, program->vertex_shader);
	glDeleteShader(program->vertex_shader);
	glDetachShader(program->program_id, program->fragment_shader);
	glDeleteShader(program->fragment_shader);
	glDeleteProgram(program->program_id);
}

static uint32_t color_rgba(int r, int g, int b, int a = 0xff) {
	return ((r & 0xff) |
			((g & 0xff) << 8) |
			((b & 0xff) << 16) |
			((a & 0xff) << 24));
}

static int create_palette(GLuint *id) {
	static const int N_COLORS = 256;

	uint32_t colors[N_COLORS];
	for (int i=0; i<N_COLORS; ++i) {
		double red   = sin(i * 0.3) * 0x80 + 0x80;
		double green = sin(i * 0.2) * 0x80 + 0x80;
		double blue  = sin(i * 0.5) * 0x80 + 0x80;
		colors[i] = color_rgba(red, green, blue);
	}

	glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, N_COLORS, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, colors);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return 0;
}

static void free_texture(texture_io *io) {
	glDeleteFramebuffers(1, &io->fbo);
	glDeleteTextures(1, &io->tex);
	glDeleteRenderbuffers(1, &io->rbo);
}

static int create_texture(texture_io *io, int width, int height) {
	if (&io->tex) {
		free_texture(io);
	}

	glGenTextures(1, &io->tex);
	glBindTexture(GL_TEXTURE_2D, io->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenRenderbuffers(1, &io->rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, io->rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);

	glGenFramebuffers(1, &io->fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, io->fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, io->tex, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, io->rbo);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "%s\n", "Framebuffer is not complete");
		return 1;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	return 0;
}

static int next_pow_2(int num) {
	//return num;
	int n = 1;
	while (n < num) {
		n *= 2;
	}
	return n;
}

static bool create_textures() {
	tex_width = next_pow_2(window_width);
	tex_height = next_pow_2(window_height);
	if (create_texture(&tex1, tex_width, tex_height) != 0) return false;
	if (create_texture(&tex2, tex_width, tex_height) != 0) return false;
	return true;
}

static void resize_callback(GLFWwindow *window, int width, int height) {
	window_width = width;
	window_height = height;
	create_textures();
	redraw_mandelbrot();
}

static int initGL() {
	if (create_program(&program_init, GET_SHADER(vertex), GET_SHADER(init)) != 0) return 1;
	if (create_program(&program_step, GET_SHADER(vertex), GET_SHADER(step)) != 0) return 2;
	if (create_program(&program_draw, GET_SHADER(vertex), GET_SHADER(draw)) != 0) return 3;

	if (create_palette(&palette) != 0) return 4;
	if (!create_textures()) return 5;

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

	GLint position = glGetAttribLocation(program_init.program_id, "position");
	glEnableVertexAttribArray(position);
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

	return 0;
}

static void set_uniform_i(GLuint program_id, const char *name, int value) {
	int location = glGetUniformLocation(program_id, name);
	glUniform1i(location, value);
}

static void set_uniform_f(GLuint program_id, const char *name, float value) {
	int location = glGetUniformLocation(program_id, name);
	glUniform1f(location, value);
}

static void set_uniform_vec2(GLuint program_id, const char *name, vec2 value) {
	int location = glGetUniformLocation(program_id, name);
	glUniform2f(location, value.x, value.y);
}

static void reset_mandelbrot() {
	center = {0.f, 0.f};
	scale = 1.5f;
}

static void redraw_mandelbrot() {
	current_tex = 0;
	iteration = 0.f;

	vec2 julia_pt = {0.0, 0.0};
	if (julia) {
		julia_pt = point_c_const;
	}

	glUseProgram(program_init.program_id);
	set_uniform_vec2(program_init.program_id, "center", center);
	set_uniform_f(program_init.program_id, "scale", scale);
	set_uniform_f(program_init.program_id, "ratio", get_ratio());

	glUseProgram(program_step.program_id);

	set_uniform_vec2(program_step.program_id, "point_c_const", julia_pt);

	set_uniform_vec2(program_step.program_id, "center", center);
	set_uniform_f(program_step.program_id, "scale", scale);
	set_uniform_f(program_step.program_id, "ratio", get_ratio());

	glUseProgram(program_draw.program_id);
	set_uniform_vec2(program_draw.program_id, "center", center);
	set_uniform_f(program_draw.program_id, "scale", scale);
	set_uniform_f(program_draw.program_id, "ratio", get_ratio());
}

static void render() {
	glViewport(0, 0, tex_width, tex_height);

	glActiveTexture(GL_TEXTURE1);
	switch (current_tex) {
	case 0:
		glBindFramebuffer(GL_FRAMEBUFFER, tex1.fbo);
		glUseProgram(program_init.program_id);
		glBindTexture(GL_TEXTURE_2D, 0);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 1;
		break;
	case 1:
		glBindFramebuffer(GL_FRAMEBUFFER, tex2.fbo);
		glUseProgram(program_step.program_id);
		set_uniform_i(program_step.program_id, "in_texture", 1);
		set_uniform_f(program_step.program_id, "iteration", iteration);
		glBindTexture(GL_TEXTURE_2D, tex1.tex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 2;
		break;
	case 2:
		glBindFramebuffer(GL_FRAMEBUFFER, tex1.fbo);
		glUseProgram(program_step.program_id);
		set_uniform_i(program_step.program_id, "in_texture", 1);
		set_uniform_f(program_step.program_id, "iteration", iteration);
		glBindTexture(GL_TEXTURE_2D, tex2.tex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		current_tex = 1;
		break;
	}

	iteration += 1.f;

	glViewport(0, 0, window_width, window_height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glUseProgram(program_draw.program_id);
	set_uniform_i(program_draw.program_id, "in_texture", 1);
	set_uniform_i(program_draw.program_id, "outside_palette", 0);
	glBindTexture(GL_TEXTURE_2D, current_tex == 1 ? tex1.tex : tex2.tex);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

static void cleanupGL() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	free_program(&program_init);
	free_program(&program_step);
	free_program(&program_draw);

	free_texture(&tex1);
	free_texture(&tex2);

	glDeleteTextures(1, &palette);
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
		fprintf(stderr, "GLEW error %d: %s\n", error, glewGetErrorString(error));
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
