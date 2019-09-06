#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <GLFW/glfw3.h>

typedef void (* key_callback_fun) (int keycode, int action);
typedef void (* mouse_callback_fun) (int button, int action, int mousex, int mousey);
typedef void (* resize_callback_fun) (int width, int height);

class display {
public:
    display(const char *title, int width, int height);
    ~display();

    bool success() {
        return window != NULL;
    }

    const char *title;
    int width;
    int height;

public:
    key_callback_fun key_callback = NULL;
    mouse_callback_fun mouse_callback = NULL;
    resize_callback_fun resize_callback = NULL;

private:
    GLFWwindow *window = NULL;
};

#endif