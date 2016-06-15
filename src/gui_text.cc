#include "gui_text.hh"
#include "gui_gl.hh"
#include "text_helpers.hh"
#include "gl_helpers.hh"
#include "globals.hh"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace gui;

void gui::render_unicode(
	const ShaderProgram &shader,
	string_unicode text,
	gui::GuiVec2 position,
	const gui::Window &window,
	FT_Face face,
	glm::vec4 color,
	float scale )
{
	// OpenGL vertex array and buffer objects for text rendering
	// TODO: Figure out a better place for these
	static GLuint vao;
	static GLuint vbo;

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	any_gl_errors();

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

	const auto window_size = window.size;

	const auto projection = glm::ortho<float>(
		0, static_cast<float>(window_size.x),
		0, static_cast<float>(window_size.y)
	);

	const auto tex_uniform = shader.get_uniform( "tex" );
	const auto mp_uniform = shader.get_uniform( "MP" );
	const auto color_uniform = shader.get_uniform( "color" );
	const auto textured_uniform = shader.get_uniform( "textured" );

	glUniformMatrix4fv( mp_uniform, 1, 0, &projection[0][0] );
	glUniform4fv( color_uniform, 1, &color[0] );
	glUniform1i( textured_uniform, 1 );
	glUniform1i( tex_uniform, 0 );
	any_gl_errors();

	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	any_gl_errors();

	const auto& font_face_contents = Globals::font_face_library[{ face }];
	auto caret_pos_x = position.x;

	GlCharacter previous_character{};

	for( auto c : text )
	{
		GlCharacter current_character;
		auto it = font_face_contents.find( c );
		if( it != font_face_contents.end() )
		{
			current_character = it->second;
		}
		else
		{
			current_character = add_character( face, c );
		}

		any_gl_errors();

		// Get kerning
		FT_Vector kerning{0,0};
		auto has_kerning = FT_HAS_KERNING( face );
		if( has_kerning && previous_character.glyph )
		{
			FT_Get_Kerning(
				face,
				previous_character.glyph,
				current_character.glyph,
				FT_KERNING_DEFAULT,
				&kerning
			);
			kerning.x >>= 6;
			kerning.y >>= 6;
		}

		// Render
		const GLfloat pos_x = caret_pos_x * scale + kerning.x;
		const GLfloat pos_y = position.y + kerning.y;
		const GLfloat w = current_character.size.x * scale;
		const GLfloat h = current_character.size.y * scale;

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

		glBindTexture( GL_TEXTURE_2D, current_character.gl_texture );

		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices );
		gui::any_gl_errors();

		glDrawArrays( GL_TRIANGLES, 0, 6 );
		
		// Bitshift by 6 to get pixels
		caret_pos_x += static_cast<int>((current_character.advance >> 6) * scale) + kerning.x;
		previous_character = current_character;
	}
}


string_unicode gui::get_line_overflow(
	string_unicode text,
	float line_width,
	FT_Face face
)
{
	return {};
}


float gui::get_line_width(
	string_unicode text,
	FT_Face face
)
{
	const auto& font_face_contents = Globals::font_face_library[{ face }];

	float width = 0;

	GlCharacter previous_character{};
	for( auto c : text )
	{
		GlCharacter current_character;
		auto it = font_face_contents.find( c );
		if( it != font_face_contents.end() )
		{
			current_character = it->second;
		}
		else
		{
			auto glyph_index = FT_Get_Char_Index( face, c );
			auto err = FT_Load_Char( face, c, FT_LOAD_DEFAULT );
			if( err )
			{
				throw runtime_error( "FT_Load_Char failed for code point " + to_string( c ) );
			}

			current_character = {
				0,
				glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows ),
				glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
				(GLuint)face->glyph->advance.x,
				glyph_index
			};
		}

		// Get kerning
		FT_Vector kerning{0,0};
		auto has_kerning = FT_HAS_KERNING( face );
		if( has_kerning && previous_character.glyph )
		{
			FT_Get_Kerning(
				face,
				previous_character.glyph,
				current_character.glyph,
				FT_KERNING_DEFAULT,
				&kerning
			);
			kerning.x >>= 6;
			kerning.y >>= 6;
		}

		width += kerning.x + (current_character.advance >> 6);
	}

	return width;
}



GuiLabel::GuiLabel( string_unicode text, size_t size )
	: content(text), font_size(size)
{
}



GuiLabel::GuiLabel( string_u8 text, size_t size )
	: content(u8_to_unicode(text)), font_size(size)
{
}



void GuiLabel::render() const
{
	auto window = static_cast<Window*>( get_root() );
	if( !window )
	{
		throw runtime_error( "No window found" );
	}

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		throw runtime_error( "No shader found" );
	}

	auto font = Globals::freetype_faces.find( "default" );
	if( font == Globals::freetype_faces.end() )
	{
		throw runtime_error( "No default font found" );
	}


	glUseProgram( shader->second.program );

	const auto color = style.get( style_state ).color_text;
	const auto padding = style.get( style_state ).padding;

	FT_Set_Pixel_Sizes( font->second, 0, font_size );
	auto cursor_pos = GuiVec2(
		pos.x + padding.x,
		//window->size.h - pos.y - size.h+ padding.w
		window->size.h - pos.y - padding.y - font_size
	);
	render_unicode( shader->second, content, cursor_pos, *window, font->second, color );
}



void GuiLabel::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		auto min_size = get_minimum_size();
		size.w = max( min_size.w, e.resize.size.w );
		size.h = max( min_size.h, e.resize.size.h );
		return;
	}

	GuiElement::handle_event( e );
}



GuiVec2 GuiLabel::get_minimum_size() const
{
	const auto padding = style.get( style_state ).padding;

	auto font = Globals::freetype_faces.find( "default" );
	if( font == Globals::freetype_faces.end() )
	{
		throw runtime_error( "No default font found" );
	}

	FT_Set_Pixel_Sizes( font->second, 0, font_size );

	const auto min_size = GuiVec2(
		get_line_width( content, font->second ) + padding.x + padding.z,
		font_size + padding.y + padding.w
	);

	return min_size;
};



void GuiTextArea::render() const
{
	auto window = static_cast<Window*>( get_root() );
	if( !window )
	{
		throw runtime_error( "No window found" );
		return;
	}

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		throw runtime_error( "No shader found" );
		return;
	}

	auto font = Globals::freetype_faces.find( "default" );
	if( font == Globals::freetype_faces.end() )
	{
		throw runtime_error( "No default font found" );
		return;
	}

	auto padding = style.get( style_state ).padding;

	auto cursor_pos = GuiVec2( pos.x + padding.x, pos.y + padding.w * 2 );

	render_unicode( shader->second, content, cursor_pos, *window, font->second );
}



void GuiTextArea::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );
}

