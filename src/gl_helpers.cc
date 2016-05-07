#pragma once

#include "gl_helpers.hh"
#include "globals.hh"
#include "mesh.hh"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

using namespace std;

void gl::render_line_2d( const ShaderProgram &shader, const glm::vec2 &window_size, glm::vec2 a, glm::vec2 b )
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

	glm::mat4 model = glm::ortho<float>(0, window_size.x, 0, window_size.y);
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



struct GlCharacter
{
	GLuint gl_texture;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

static map<unsigned long, GlCharacter> gl_characters;


size_t get_octet_count( const char byte )
{
	if( (byte & 0b10000000) == 0 )
	{
		return 1;
	}
	else if( (byte & 0b11100000) == 0b11000000 )
	{
		return 2;
	}
	else if( (byte & 0b11110000) == 0b11100000 )
	{
		return 3;
	}
	else if( (byte & 0b11111000) == 0b11110000 )
	{
		return 4;
	}
	else return 1;
}


unsigned long utf8_to_unicode( unsigned long c )
{
	unsigned long val = 0;

	if( (c & 0xf8000000) == 0xf0000000 )
	{
		val |= (c & 0x7000000) >> 6;
		val |= (c & 0x3f0000) >> 4;
		val |= (c & 0x3f00) >> 2;
		val |= (c & 0x3f);
	}
	else if( (c & 0xf00000) == 0xe00000 )
	{
		val |= (c & 0xf0000) >> 4;
		val |= (c & 0x3f00) >> 2;
		val |= (c & 0x3f);
	}
	else if( (c & 0xe000) == 0xc000 )
	{
		val |= (c& 0x1f00) >> 2;
		val |= (c & 0x3f);
	}
	else val = c;

	return val;
}


unsigned long read_next_character( const char *str, const char * const end, size_t &bytes )
{
	unsigned long character = 0;

	if( str >= end )
	{
		bytes = 1;
		return 0;
	}

	auto octet_count = get_octet_count( *str );
	if( (str + octet_count) > end )
	{
		octet_count = end - str;
	}

	bytes = octet_count ? octet_count : 1;

	character |= str[0] & 0xff;
	str++;
	while( octet_count-- > 1 )
	{
		character <<= 8;
		character |= str++[0] & 0xff;
	}

	return utf8_to_unicode( character );
}


GlCharacter add_character( FT_Face face, unsigned long c )
{
	FT_Select_Charmap( face, FT_Encoding::FT_ENCODING_WANSUNG );
	auto glyph_index = FT_Get_Char_Index( face, c );
	if( FT_Load_Char( face, c, FT_LOAD_RENDER ) )
	{
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		return {};
	}

	// Generate texture
	GLuint texture;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	gui::any_gl_errors();

	// Set texture options
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	gui::any_gl_errors();

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);
	glBindTexture( GL_TEXTURE_2D, 0 );
	gui::any_gl_errors();

	// Now store character for later use
	GlCharacter character = {
		texture,
		glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows ),
		glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
		(GLuint)face->glyph->advance.x
	};

	gl_characters.insert( { c, character } );
	gui::any_gl_errors();
	return character;
}

void gl::render_text_2d(
	const ShaderProgram &shader,
	const glm::vec2 &window_size,
	const std::string &text,
	glm::vec2 pos,
	glm::vec2 scale,
	FT_Face face )
{
	const auto str_start = text.c_str();
	const auto str_end = str_start + text.length();

	static GLuint vao;
	static GLuint vbo;

	glUseProgram( shader.program );

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

	auto projection = glm::ortho<float>( 0, window_size.x, 0, window_size.y );
	const auto tex_uniform = shader.get_uniform( "tex" );
	const auto mp_uniform = shader.get_uniform( "MP" );
	const auto color_uniform = shader.get_uniform( "color" );
	const auto textured_uniform = shader.get_uniform( "textured" );

	glUniformMatrix4fv( mp_uniform, 1, 0, &projection[0][0] );
	glUniform4f( color_uniform, 1.0, 1.0, 1.0, 1.0 );
	glUniform1i( textured_uniform, 1 );
	glUniform1i( tex_uniform, 0 );
	gui::any_gl_errors();

	auto str_ptr = str_start;
	unsigned long current_character = 0;
	size_t bytes_read = 0;
	float caret_pos_x = pos.x;

	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	gui::any_gl_errors();

	glActiveTexture( GL_TEXTURE0 );
	gui::any_gl_errors();
	
	while( (current_character = read_next_character(str_ptr, str_end, bytes_read)) )
	{
		gui::any_gl_errors();
		str_ptr += bytes_read;

		// Find or create the character
		GlCharacter c;
		auto it = gl_characters.find( current_character );
		if( it != gl_characters.end() )
		{
			c = it->second;
		}
		else
		{
			c = add_character( face, current_character );
		}
		gui::any_gl_errors();

		// Render
		const GLfloat pos_x = caret_pos_x * scale.x;
		const GLfloat pos_y = pos.y - (c.size.y - c.bearing.y);
		const GLfloat w = c.size.x * scale.x;
		const GLfloat h = c.size.y * scale.y;

		const GLfloat vertices[6][4] = {
			{ pos_x,     pos_y + h, 0.0, 0.0 },
			{ pos_x,     pos_y,     0.0, 1.0 },
			{ pos_x + w, pos_y,     1.0, 1.0 },

			{ pos_x,     pos_y + h, 0.0, 0.0 },
			{ pos_x + w, pos_y,     1.0, 1.0 },
			{ pos_x + w, pos_y + h, 1.0, 0.0 }
		};

		auto start = glm::vec4( vertices[1][0], vertices[1][1], 0, 1 );
		auto end = glm::vec4( vertices[4][0], vertices[4][1], 0, 1 );

		glBindTexture( GL_TEXTURE_2D, c.gl_texture );

		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices );
		gui::any_gl_errors();

		glDrawArrays( GL_TRIANGLES, 0, 6 );
		caret_pos_x += (c.advance >> 6) * scale.x; // Bitshift by 6 to get pixels
	}
	//glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindVertexArray( 0 );
	glUseProgram( 0 );
	/*
	GLuint vbo;
	glCreateBuffers( 1, &vbo );
	glBindBuffer( GL_VERTEX_ARRAY, vbo );
	*/
}


glm::vec2 gl::gui_to_gl_vec(
	const gui::GuiVec2 &coord
)
{
	glm::vec2 val = {
		coord.x,
		coord.y
	};

	return val;
}

