#pragma once

#include "shader.hh"
#include <map>


struct ShaderProgram
{
	GLuint program;
	GLint linked;

	mutable std::map<std::string, GLint> uniforms;
	std::map<std::string, GLuint> attributes;

	ShaderProgram(
		const Shader &vertexShader,
		const Shader &fragmentShader,
		std::map<std::string, GLuint> attributes
	);

	~ShaderProgram();

	const GLint ShaderProgram::GetUniform( const std::string& uniformName ) const;
};

