#ifndef __MESH_H__
#define __MESH_H__

#include <GL/glew.h>

class mesh {
public:
    mesh();
    ~mesh();

    void draw();

private:
    GLuint vao = 0;
    GLuint vbo = 0;
};

#endif