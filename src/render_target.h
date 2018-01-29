#ifndef __RENDER_TARGET_H__
#define __RENDER_TARGET_H__

#include <GL/glew.h>

#include "texture.h"

class render_target {
public:
	render_target(const texture &tex);
	~render_target();

	void bind();
	static void bindScreen(int width, int height);

	const texture &tex;

private:
	GLuint fbo_id = 0;
	GLuint rbo_id = 0;
};

render_target::render_target(const texture &tex) : tex(tex) {
	glGenFramebuffers(1, &fbo_id);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

	glGenRenderbuffers(1, &rbo_id);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, tex.width, tex.height);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.texture_id, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_id);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "%s\n", "Framebuffer is not complete");
	}
}

render_target::~render_target() {
	if (fbo_id) {
		glDeleteFramebuffers(1, &fbo_id);
		fbo_id = 0;
	}
	if (rbo_id) {
		glDeleteRenderbuffers(1, &rbo_id);
		rbo_id = 0;
	}
}

void render_target::bind() {
	glViewport(0, 0, tex.width, tex.height);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
}

void render_target::bindScreen(int width, int height) {
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#endif // __RENDER_TARGET_H__