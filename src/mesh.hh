#pragma once

#include "gui_gl.hh"

struct Mesh
{
	GLuint vertex_count;
	GLuint vao;
	GLuint vbo;
};


static Mesh create_mesh(
	const ShaderProgram &shader,
	const GLfloat *data,
	const GLuint size
)
{
	Mesh mesh;

	mesh.vao = 1;
	mesh.vertex_count = size / sizeof( GLfloat ) / 3;

	glUseProgram( shader.program );

	glGenVertexArrays( 1, &mesh.vao );
	glBindVertexArray( mesh.vao );
	glGenBuffers( 1, &mesh.vbo );
	glBindBuffer( GL_ARRAY_BUFFER, mesh.vbo );
	gui::any_gl_errors();

	glBufferData( GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW );
	gui::any_gl_errors();

	auto attribute = shader.attributes.find( "vertex" );
	if( attribute != shader.attributes.end() )
	{
		glEnableVertexAttribArray( attribute->second );
		glVertexAttribPointer( attribute->second, 3, GL_FLOAT, GL_FALSE, 0, nullptr );
		gui::any_gl_errors();
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	return mesh;
}

