#include "display.h"

#include <cstdio>

static display *context; // UGLY HACK

static void _key_callback(GLFWwindow *window, int keycode, int scancode, int action, int mods) {
	if (context->key_callback) {
		context->key_callback(keycode, action);
	}
}

static void _mouse_callback(GLFWwindow *window, int button, int action, int mods) {
	if (context->mouse_callback) {
		double mousex, mousey;
		glfwGetCursorPos(window, &mousex, &mousey);

		context->mouse_callback(button, action, (int)mousex, (int)mousey);
	}
}

static void _resize_callback(GLFWwindow *window, int width, int height) {
	context->width = width;
	context->height = height;

	if (context->resize_callback) {
		context->resize_callback(width, height);
	}
}

display::display(const char *title, int width, int height) : title(title), width(width), height(height) {
	context = this;

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "%s\n", "Could not create window");
        return;
    }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetKeyCallback(window, _key_callback);
	glfwSetMouseButtonCallback(window, _mouse_callback);
	glfwSetWindowSizeCallback(window, _resize_callback);
}

display::~display() {
	glfwDestroyWindow(window);
}