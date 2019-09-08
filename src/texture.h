#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/glew.h>

#include <cstdio>

class texture {
private:
	GLuint tex = 0;

public:
	int width;
	int height;

public:
	~texture() {
		if (tex) glDeleteTextures(1, &tex);
	}

	int create_texture(int width, int height, GLint format, GLenum type, void *data = 0);

    void bind(int unit) {
		glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, tex);
    }

private:
	friend class framebuffer;
};

class framebuffer {
private:
	texture &tex;

	GLuint fbo = 0;

public:
	framebuffer(texture &tex);

	~framebuffer() {
		if (fbo) glDeleteFramebuffers(1, &fbo);
	}

    void bind() {
		glViewport(0, 0, tex.width, tex.height);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
};

#endif