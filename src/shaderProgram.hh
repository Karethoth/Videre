#pragma once

#include "shader.hh"
#include <map>


struct ShaderProgram
{
	GLuint program;
	GLint linked;

	std::map<std::string, GLint>  uniforms;
	std::map<std::string, GLuint> attributes;

	ShaderProgram(
		const Shader &vertexShader,
		const Shader &fragmentShader,
		std::map<std::string, GLuint> attributes
	);

	~ShaderProgram();
};

