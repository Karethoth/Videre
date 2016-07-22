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
		const Shader &vertex_shader,
		const Shader &fragment_shader,
		std::map<std::string, GLuint> attributes
	);

	~ShaderProgram();

	const GLint get_uniform( const std::string& uniform_name ) const;
};

