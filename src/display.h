#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace display {

extern int width;
extern int height;

int init_display(const char *title, int width, int height);
void cleanup_display();

void (*key_callback) (int keycode, int action) = NULL;
void (*mouse_callback) (int button, int action, int mousex, int mousey) = NULL;
void (*resize_callback) () = NULL;

}

#endif