#include "display.h"

#include <cstdio>

namespace display {

int width;
int height;

GLFWwindow *window = NULL;

void _key_callback(GLFWwindow *window, int keycode, int scancode, int action, int mods) {
	if (key_callback) {
		key_callback(keycode, action);
	}
}

void _mouse_callback(GLFWwindow *window, int button, int action, int mods) {
	if (mouse_callback) {
		double mousex, mousey;
		glfwGetCursorPos(window, &mousex, &mousey);

		mouse_callback(button, action, (int)mousex, (int)mousey);
	}
}

void _resize_callback(GLFWwindow *window, int w, int h) {
	width = w;
	height = h;

	if (resize_callback) {
		resize_callback();
	}
}

int init_display(const char *title, int w, int h) {
	width = w;
	height = h;

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window) {
        fprintf(stderr, "%s\n", "Could not create window");
        return 1;
    }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetKeyCallback(window, _key_callback);
	glfwSetMouseButtonCallback(window, _mouse_callback);
	glfwSetWindowSizeCallback(window, _resize_callback);

	glewExperimental = true;
	GLenum error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "GLEW error %d: %s\n", error, glewGetErrorString(error));
		glfwTerminate();
		return 2;
	}

	return 0;
}

void cleanup_display() {
	glfwDestroyWindow(window);
}

}