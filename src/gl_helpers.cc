#pragma once

#include "gl_helpers.hh"
#include "text_helpers.hh"
#include "globals.hh"
#include "mesh.hh"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace std;

void gl::FramebufferObject::bind()
{
	if( !framebuffer_id )
	{
		return;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id );
	glViewport( 0, 0, texture_size.x, texture_size.y );
}



void gl::FramebufferObject::resize( const glm::ivec2 size )
{
	texture_size = size;

	if( framebuffer_id )
	{
		glDeleteFramebuffers( 1, &framebuffer_id );
		glDeleteTextures( 1, &texture_id );
	}

	glGenFramebuffers( 1, &framebuffer_id );
	if( !framebuffer_id )
	{
		throw runtime_error( "Couldn't create framebuffer" );
	}
	glBindFramebuffer( GL_FRAMEBUFFER, framebuffer_id );

	glGenTextures( 1, &texture_id );
	if( !texture_id )
	{
		throw runtime_error( "Couldn't create texture" );
	}
	glBindTexture( GL_TEXTURE_2D, texture_id );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	glFramebufferTexture( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0 );
	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, &draw_buffers[0] );


	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
	
}



void gl::render_line_2d(
	const ShaderProgram &shader,
	const glm::vec2 &window_size,
	glm::vec2 a,
	glm::vec2 b
)
{
	static const GLfloat line_vertex_data[] = {
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f
	};

	glUseProgram( shader.program );

	static Mesh line;
	static const float step90 = 1.57079633f;
	static const float step180 = step90 * 2;

	if( !line.vao )
	{
		line = create_mesh( shader, line_vertex_data, sizeof( line_vertex_data ) );
	}

	auto diff = b - a;

	glm::mat4 model = glm::ortho<float>( 0, window_size.x, window_size.y, 0 );
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
	glBindBuffer( GL_ARRAY_BUFFER, line.vbo );

	auto mpUniform = shader.get_uniform( "MP" );
	glUniformMatrix4fv( mpUniform, 1, GL_FALSE, &mp[0][0] );

	auto texturedUniform = shader.get_uniform( "textured" );
	glUniform1i( texturedUniform, 0 );
	
	glDrawArrays( GL_LINES, 0, line.vertex_count );
	glBindVertexArray( 0 );
}



void gl::render_quad_2d( const ShaderProgram &shader, const glm::vec2 &window_size, glm::vec2 pos, glm::vec2 size )
{
	static const GLfloat quad_vertex_data[] = {
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};

	glUseProgram( shader.program );

	static Mesh quad;

	if( !quad.vao )
	{
		quad = create_mesh( shader, &quad_vertex_data[0], sizeof( quad_vertex_data ) );
	}

	glm::mat4 model = glm::ortho<float>(0, window_size.x, 0, window_size.y);
	model = glm::translate( model, glm::vec3( pos.x, window_size.y - pos.y, 0.0f ) );
	model = glm::scale( model, glm::vec3( size.x, -size.y, 1.0f ) );
	auto mp = model;

	glBindVertexArray( quad.vao );
	glBindBuffer( GL_ARRAY_BUFFER, quad.vbo );

	auto mpUniform = shader.get_uniform( "MP" );
	glUniformMatrix4fv( mpUniform, 1, GL_FALSE, &mp[0][0] );

	auto texturedUniform = shader.get_uniform( "textured" );
	glUniform1i( texturedUniform, 0 );
	
	glDrawArrays( GL_TRIANGLES, 0, quad.vertex_count );
	glBindVertexArray( 0 );
}



size_t gl::render_text_2d(
	const ShaderProgram &shader,
	const glm::vec2 &window_size,
	const string_u8 &text,
	glm::vec2 pos,
	glm::vec2 scale,
	FT_Face face,
	size_t font_size )
{
	static GLuint vao;
	static GLuint vbo;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	gui::any_gl_errors();

	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	if( !vao )
	{
		glGenVertexArrays( 1, &vao );
		glGenBuffers( 1, &vbo );
		glBindVertexArray( vao );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, sizeof( GLfloat ) * 6 * 4, nullptr, GL_DYNAMIC_DRAW );
		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof( GLfloat ), 0 );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );
		gui::any_gl_errors();
	}

	const auto projection = glm::ortho<float>( 0, window_size.x, 0, window_size.y );
	const auto tex_uniform = shader.get_uniform( "tex" );
	const auto mp_uniform = shader.get_uniform( "MP" );
	const auto color_uniform = shader.get_uniform( "color" );
	const auto textured_uniform = shader.get_uniform( "textured" );

	glUniformMatrix4fv( mp_uniform, 1, 0, &projection[0][0] );
	glUniform4f( color_uniform, 1.0, 1.0, 1.0, 1.0 );
	glUniform1i( textured_uniform, 1 );
	glUniform1i( tex_uniform, 0 );
	gui::any_gl_errors();

	float caret_pos_x = pos.x;

	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	gui::any_gl_errors();

	glActiveTexture( GL_TEXTURE0 );
	gui::any_gl_errors();

	GlCharacter previous_character{};

	auto unicode_str = u8_to_unicode( text );
	for( const auto code_point : unicode_str )
	{
		// Find or create the character
		auto c = Globals::font_face_manager.get_character( face, code_point );

		// Get kerning
		FT_Vector kerning{0,0};
		auto has_kerning = FT_HAS_KERNING( face );
		if( has_kerning && previous_character.glyph )
		{
			FT_Get_Kerning(
				face,
				previous_character.glyph,
				c.glyph,
				FT_KERNING_DEFAULT,
				&kerning
			);
			kerning.x >>= 6;
			kerning.y >>= 6;
		}

		// Render
		const GLfloat pos_x = caret_pos_x * scale.x + kerning.x;
		const GLfloat pos_y = pos.y - (c.size.y - c.bearing.y) + kerning.y;
		const GLfloat w = c.size.x * scale.x;
		const GLfloat h = c.size.y * scale.y;

		GLfloat vertices[6][4] = {
			{ pos_x,     pos_y + h, 0.0f, 0.0f },
			{ pos_x,     pos_y,     0.0f, 1.0f },
			{ pos_x + w, pos_y,     1.0f, 1.0f },

			{ pos_x,     pos_y + h, 0.0f, 0.0f },
			{ pos_x + w, pos_y,     1.0f, 1.0f },
			{ pos_x + w, pos_y + h, 1.0f, 0.0f }
		};

		auto start = glm::vec4( vertices[1][0], vertices[1][1], 0.f, 1.f );
		auto end = glm::vec4( vertices[4][0], vertices[4][1], 0.f, 1.f );

		glBindTexture( GL_TEXTURE_2D, c.gl_texture );

		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), &vertices[0][0] );
		gui::any_gl_errors();

		glDrawArrays( GL_TRIANGLES, 0, 6 );
		
		// Bitshift by 6 to get pixels
		caret_pos_x += (c.advance >> 6) * scale.x + kerning.x;
		previous_character = c;
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindVertexArray( 0 );

	return static_cast<size_t>( caret_pos_x - pos.x );
}

