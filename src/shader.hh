#pragma once

#include <GL/glew.h>
#include <string>


struct Shader
{
	GLenum type;
	GLuint shader;
	GLint  compiled;

	Shader( GLenum type, const std::string &filepath );
	~Shader();
};

