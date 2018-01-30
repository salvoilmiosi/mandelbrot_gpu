#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <GL/glew.h>

class texture {
public:
	texture(int width, int height);
	~texture();

	const int width;
	const int height;

	void bind();
	void attachPixels(GLint internalFormat, GLenum format, GLenum type, const GLvoid *data);
	void setFilter(GLenum value);
	void setWrap(GLenum value);

private:
	GLuint texture_id = 0;

	friend class render_target;
};

texture::texture(int width, int height) : width(width), height(height) {}

texture::~texture() {
	if (texture_id) {
		glDeleteTextures(1, &texture_id);
		texture_id = 0;
	}
}

void texture::bind() {
	glBindTexture(GL_TEXTURE_2D, texture_id);
}

void texture::attachPixels(GLint internalFormat, GLenum format, GLenum type, const GLvoid *data) {
	glGenTextures(1, &texture_id);

	bind();
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);
	setFilter(GL_NEAREST);
	setWrap(GL_CLAMP_TO_EDGE);
}

void texture::setFilter(GLenum value) {
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, value);
}

void texture::setWrap(GLenum value) {
	bind();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, value);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, value);
}

#endif // __TEXTURE_H__