#pragma once

#include "gl_helpers.hh"
#include "mesh.hh"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <iostream>


void gl::RenderLine2D( const ShaderProgram &shader, glm::vec2 a, glm::vec2 b )
{
	static const GLfloat lineVertexData[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	static Mesh line;
	static const float step90 = 1.57079633f;
	static const float step180 = step90 * 2;

	if( !line.vao )
	{
		line = CreateMesh( shader, lineVertexData, sizeof( lineVertexData ) );
	}

	auto diff = b - a;

	glm::mat4 model;
	model = glm::translate( model, glm::vec3( a, 0.0f ) );

	// Horizontal line
	if( diff.y == 0.0 )
	{
		if( diff.x < 0 ) model = glm::rotate( model, step180, glm::vec3{ 0.f, 0.f, 1.f } );
	}

	// Vertical line
	else if( diff.x == 0.0 )
	{
		if( diff.y > 0 ) model = glm::rotate( model, step90, glm::vec3{ 0.f, 0.f, 1.f } );
		else model = glm::rotate( model, -step90, glm::vec3{ 0.f, 0.f, 1.f } );
	}

	// Non-trivial line
	else
	{
		auto horizontal = glm::vec2( 1.f, 0.f );
		auto angle = glm::orientedAngle( horizontal, glm::normalize(diff) );
		model = glm::rotate( model, angle, glm::vec3{ 0.f, 0.f, 1.f } );
	}

	float length = glm::length( diff );
	model = glm::scale( model, { length, 1.f, 1.f } );
	auto mp = model;

	glBindVertexArray( line.vao );

	auto mpUniform = shader.GetUniform( "MP" );
	glUniformMatrix4fv( mpUniform, 1, GL_FALSE, &mp[0][0] );
	
	glDrawArrays( GL_LINES, 0, line.vertex_count );
}


glm::vec2 gl::GuiToGlVec(
	const gui::GuiVec2 &coord,
	const gui::GuiVec2 &windowSize
	)
{
	glm::vec2 val;
	val.x = (coord.x / (float)windowSize.w)*2.f - 1.0;
	val.y = (coord.y / (float)windowSize.h)*-2.f + 1.0;
	return val;
}

