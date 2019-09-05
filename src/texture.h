#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/glew.h>

#include <cstdio>

class texture_io {
private:
	GLuint tex = 0;
	GLuint fbo = 0;

public:
	int width;
	int height;

public:
	~texture_io() {
		if (tex) glDeleteTextures(1, &tex);
		if (fbo) glDeleteFramebuffers(1, &fbo);
	}
	int create_texture(int width, int height, GLint format, GLenum type);

    void bind_texture() {
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    void bind_framebuffer() {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
};

#endif