#include "gui_text.hh"
#include "gui_gl.hh"
#include "text_helpers.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include "settings.hh"

#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace gui;


void gui::render_unicode(
	const ShaderProgram &shader,
	const string_unicode &text,
	const gui::GuiVec2 position,
	const gui::GuiVec2 viewport_size,
	FT_Face face,
	const glm::vec4 color,
	float scale )
{
	// OpenGL vertex array and buffer objects for text rendering
	// TODO: Figure out a better place for these
	static GLuint vao;
	static GLuint vbo;

	if( !viewport_size.x || !viewport_size.y )
	{
		return;
	}

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

	const auto projection = glm::ortho<float>(
		0, tools::int_to_float(viewport_size.w),
		0, tools::int_to_float(viewport_size.h)
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

	auto pen_pos_x = position.x;

	GlCharacter previous_character{};

	vector<pair<GlCharacter, FT_Vector>> characters{ text.size() };
	size_t max_used_height = 0;
	for( auto c : text )
	{
		auto result = Globals::font_face_manager.get_character( face, c );
		auto current_character = result.first;
		auto used_face = result.second;
		auto face_ptr = used_face.get();
		if( !face_ptr )
		{
			face_ptr = face;
		}

		FT_Vector kerning{0,0};
		auto has_kerning = FT_HAS_KERNING( face );
		if( has_kerning && previous_character.glyph )
		{
			FT_Get_Kerning(
				face_ptr,
				previous_character.glyph,
				current_character.glyph,
				FT_KERNING_DEFAULT,
				&kerning
			);
			kerning.x >>= 6;
			kerning.y >>= 6;
		}

		max_used_height = max( max_used_height, current_character.font_height );

		characters.emplace_back( current_character, kerning );
	}

	for( auto glyph_info : characters )
	{
		auto &current_character = glyph_info.first;
		auto &kerning = glyph_info.second;

		// Render
		const auto x_adjust = current_character.bearing.x + kerning.x;
		const auto y_adjust = current_character.size.y - current_character.bearing.y
		                    - kerning.y - max_used_height/4;
		const GLfloat pos_x = (pen_pos_x + x_adjust) * scale;
		const GLfloat pos_y = (position.y - y_adjust) * scale;
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

		glBindTexture( GL_TEXTURE_2D, current_character.gl_texture );

		glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), &vertices[0][0] );
		gui::any_gl_errors();

		glDrawArrays( GL_TRIANGLES, 0, 6 );
		gui::any_gl_errors();
		
		// Bitshift by 6 to get pixels
		pen_pos_x += tools::float_to_int( (current_character.advance >> 6) * scale );
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



GuiVec2 gui::get_text_bounding_box(
	FT_Face face,
	string_unicode text,
	size_t font_size
)
{
	float width = 0;
	float height = 0;


	Globals::font_face_manager.sync_font_face_sizes( font_size );

	GlCharacter previous_character{};
	for( auto c : text )
	{
		auto result = Globals::font_face_manager.get_character( face, c );
		auto current_character = result.first;
		auto used_face = result.second;
		auto face_ptr = used_face.get();
		if( !face_ptr )
		{
			face_ptr = face;
		}

		height = max( height, current_character.font_height );

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
		}

		width += kerning.x + (current_character.advance >> 6);
	}


	return { tools::float_to_int(width), tools::float_to_int(height) };
}



TextTexture::TextTexture( string_unicode text, size_t font_size, GuiVec2 texture_size )
: content(text),
  font_size(font_size),
  texture_size(texture_size.w, texture_size.h)
{
	if( !texture_size.x || !texture_size.y )
	{
		return;
	}

	reset_texture();
}



void TextTexture::set_text( const string_unicode text )
{
	content = text;
}



void TextTexture::set_font_size( const size_t size )
{
	font_size = size;
}



void TextTexture::set_texture_size( const GuiVec2 size )
{
	texture_size = { size.w, size.h };
}



void TextTexture::reset_texture()
{
	framebuffer.resize( texture_size );
	update_texture();
}



void TextTexture::update_texture()
{
	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		return;
	}

	auto font_face = Globals::font_face_manager.get_default_font_face();

	framebuffer.bind();
	render_unicode(
		shader->second,
		content,
		{ 0, 0 },
		{ texture_size.x, texture_size.y },
		font_face.get(),
		{ 1.f, 1.f, 1.f, 1.f },
		1.f
	);

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	gui::any_gl_errors();
}



void TextTexture::render( const GuiVec2 position, const GuiVec2 viewport_size, const glm::vec4 color ) const
{
	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		return;
	}

	glUseProgram( shader->second.program );

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

	const auto projection = glm::ortho<float>(
		0, tools::int_to_float(viewport_size.w),
		tools::int_to_float(viewport_size.h), 0
	);

	const auto tex_uniform = shader->second.get_uniform( "tex" );
	const auto mp_uniform = shader->second.get_uniform( "MP" );
	const auto color_uniform = shader->second.get_uniform( "color" );
	const auto textured_uniform = shader->second.get_uniform( "textured" );

	glUniformMatrix4fv( mp_uniform, 1, 0, &projection[0][0] );
	glUniform4fv( color_uniform, 1, &color[0] );
	glUniform1i( textured_uniform, 1 );
	glUniform1i( tex_uniform, 0 );
	any_gl_errors();

	glBindVertexArray( vao );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	any_gl_errors();

	const auto pos_x = tools::int_to_float( position.x );
	const auto pos_y = tools::int_to_float( viewport_size.h - position.y - texture_size.y );
	const auto w = tools::int_to_float( texture_size.x );
	const auto h = tools::int_to_float( texture_size.y );

	const GLfloat vertices[6][4] = {
		{ pos_x,     pos_y + h, 0.0, 0.0 },
		{ pos_x,     pos_y,     0.0, 1.0 },
		{ pos_x + w, pos_y,     1.0, 1.0 },

		{ pos_x,     pos_y + h, 0.0, 0.0 },
		{ pos_x + w, pos_y,     1.0, 1.0 },
		{ pos_x + w, pos_y + h, 1.0, 0.0 }
	};

	glBindTexture( GL_TEXTURE_2D, framebuffer.texture_id );

	glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), &vertices[0][0] );
	gui::any_gl_errors();

	glDrawArrays( GL_TRIANGLES, 0, 6 );
	gui::any_gl_errors();
}



GuiLabel::GuiLabel( string_unicode text, size_t size )
: content(text),
  font_size(size),
  content_size(0, 0),
  dynamic_font_size(false),
  text_texture(text, size, get_minimum_size())
{
}



GuiLabel::GuiLabel( string_u8 text, size_t size )
: content(u8_to_unicode( text )),
  font_size(size ),
  content_size(0, 0),
  text_texture(content, size, {get_minimum_size()}),
  dynamic_font_size(false)
{
}



GuiLabel::~GuiLabel()
{
}



void GuiLabel::render() const
{
	try
	{
		auto window = dynamic_cast<const Window*>( get_root() );
		if( !window )
		{
			throw runtime_error( "No window found" );
		}

		GuiElement::render();

		const auto padding = style.get( style_state ).padding;

		GuiVec2 text_position{
			tools::float_to_int( pos.x + padding.x ),
			tools::float_to_int( window->size.h - pos.y - content_size.h - padding.y )
		};

		text_texture.render(
			text_position,
			window->size,
			style.get( style_state ).color_text
		);
	}
	catch( runtime_error &e )
	{
		wcout << "GuiLabel::render failed: " << e.what() << "\n";
	}
}



void GuiLabel::handle_event( const GuiEvent &e )
{
	gui::any_gl_errors();
	GuiElement::handle_event( e );
	gui::any_gl_errors();

	if( e.type == RESIZE )
	{
		const auto padding = style.get( style_state ).padding;
		auto min_size = get_minimum_size();
		min_size.w -= tools::float_to_int( padding.x + padding.z );
		min_size.h -= tools::float_to_int( padding.y + padding.w );
		text_texture.set_texture_size( min_size );
		refresh();
	}
	else if( e.type == REFRESH_RESOURCES )
	{
		refresh();
	}
}



void GuiLabel::refresh()
{
	gui::any_gl_errors();
	const auto font_face = Globals::font_face_manager.get_default_font_face();

	used_font_size = font_size;
	if( dynamic_font_size )
	{
		lock_guard<mutex> settings_lock( settings::settings_mutex );
		auto settings_font_size = settings::core["font_size"].get<int>();
		if( settings_font_size > 0 )
		{
			used_font_size = settings_font_size;
		}
	}

	Globals::font_face_manager.sync_font_face_sizes( used_font_size );

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		throw runtime_error( "No shader found" );
	}

	glUseProgram( shader->second.program );
	gui::any_gl_errors();

	content_size = get_text_bounding_box( font_face.get(), content, used_font_size );
	text_texture.set_texture_size( content_size );
	text_texture.reset_texture();
}



GuiVec2 GuiLabel::get_minimum_size() const
{
	const auto padding = style.get( style_state ).padding;
	const auto padding_w = padding.x + padding.z;
	const auto padding_h = padding.y + padding.w;

	if( content.size() && (!content_size.x || !content_size.y) )
	{
		auto face = Globals::font_face_manager.get_default_font_face();
		const auto content_rect = get_text_bounding_box(
			face.get(),
			content,
			font_size
		);

		const auto fresh_min_size = GuiVec2(
			static_cast<int>(content_rect.w + padding_w),
			static_cast<int>(content_rect.h + padding_h)
		);

		return fresh_min_size;
	}

	const auto min_size = GuiVec2(
		static_cast<int>(content_size.w + padding_w),
		static_cast<int>(content_size.h + padding_w)
	);

	return min_size;
};



void GuiLabel::set_font_size( size_t size )
{
	font_size = size;
	refresh();
}



GuiTextField::GuiTextField()
{
	cursor.index = 0;
	cursor.is_shown = false;
}



GuiTextField::~GuiTextField()
{
}



void GuiTextField::update()
{
	auto now = chrono::steady_clock::now();
	if( now > cursor.next_step )
	{
		cursor.is_shown = !cursor.is_shown;
		cursor.next_step = now + cursor.interval;
		update_content();
	}
}



void GuiTextField::render() const
{
	try
	{
		auto window = dynamic_cast<const Window*>( get_root() );
		if( !window )
		{
			throw runtime_error( "No window found" );
		}

		auto shader = Globals::shaders.find( "2d" );
		if( shader == Globals::shaders.end() )
		{
			throw runtime_error( "No shader found" );
		}

		glUseProgram( shader->second.program );

		GuiElement::render();

		const auto color = style.get( style_state ).color_text;
		const auto padding = style.get( style_state ).padding;
		const auto font_face = Globals::font_face_manager.get_default_font_face();

		auto used_font_size = font_size;
		if( dynamic_font_size )
		{
			lock_guard<mutex> settings_lock( settings::settings_mutex );
			auto settings_font_size = settings::core["font_size"].get<int>();
			if( settings_font_size > 0 )
			{
				used_font_size = settings_font_size;
			}
		}

		Globals::font_face_manager.sync_font_face_sizes( used_font_size );

		auto text_pos = GuiVec2(
			tools::float_to_int( pos.x + padding.x ),
			tools::float_to_int( window->size.h - pos.y - padding.y - used_font_size )
		);

		render_unicode( shader->second, content, text_pos, window->size, font_face.get(), color );
		if( cursor.is_shown )
		{
			auto cursor_pos = GuiVec2(
				pos.x + padding.x + cursor.pos_x + 1,
				pos.y + size.h/4
			);

			gl::render_line_2d(
				shader->second,
				window->size.to_gl_vec(),
				{ cursor_pos.x, cursor_pos.y },
				{ cursor_pos.x, cursor_pos.y + size.h / 2 }
			);
		}
	}
	catch( runtime_error &e )
	{
		wcout << "GuiTextField::render failed: " << e.what() << "\n";
	}
}



void GuiTextField::handle_event( const GuiEvent &e )
{

	if( e.type == GuiEventType::MOUSE_BUTTON &&
	    in_area( e.mouse_button.pos ) &&
	    e.mouse_button.state == RELEASED )
	{
		is_active = true;
	}
	else
	{
		is_active = false;
	}

	if( e.type == TEXT_INPUT )
	{
		auto new_input = u8_to_unicode( e.text_input.text );
		// Strip spaces from the input here because
		// they are handled manually with the KEY events
		// - TEXT_INPUT doesn't work with spaces when IME is on
		new_input.erase(
			std::remove_if(
				new_input.begin(),
				new_input.end(),
				[]( auto c ) { return c == ' '; }
			),
			new_input.end()
		);

		if( text_input.size() >= max_characters )
		{
			return;
		}

		text_input.insert( text_input.begin()+cursor.index, new_input.begin(), new_input.end() );
		text_edit.text = {};
		cursor.index += new_input.size();

		SDL_Rect ime_rect{};
		ime_rect.x = pos.x;
		ime_rect.y = pos.y + 50;
		ime_rect.w = size.w;
		ime_rect.y = 100;
		SDL_SetTextInputRect( &ime_rect );
		text_edit.is_ime_on = false;

		update_content();
		refresh();
	}

	else if( e.type == TEXT_EDIT )
	{
		text_edit.text = u8_to_unicode( e.text_edit.text );
		text_edit.is_ime_on = true;
		update_content();
		refresh();
	}

	else if ( e.type == KEY )
	{
		if( e.key.state == PRESSED )
		{
			if( e.key.button.scancode == SDL_SCANCODE_BACKSPACE &&
				text_edit.text.size() > 0 )
			{
				text_edit.text.pop_back();
				update_content();
			}
			else if( e.key.button.scancode == SDL_SCANCODE_BACKSPACE &&
			         text_input.size() > 0 )
			{
				if( cursor.index > 0 )
				{
					text_input.erase( text_input.begin() + cursor.index-1 );
					cursor.index--;
					update_content();
				}
			}
			else if( e.key.button.scancode == SDL_SCANCODE_DELETE &&
			         text_input.size() > cursor.index )
			{
				text_input.erase( text_input.begin() + cursor.index );
				update_content();
			}
			else if( text_edit.is_ime_on &&
			         ( e.key.button.scancode == SDL_SCANCODE_RETURN ||
			           e.key.button.scancode == SDL_SCANCODE_ESCAPE ) )
			{
				text_edit.is_ime_on = false;
			}

			else if( e.key.button.scancode == SDL_SCANCODE_SPACE &&
			         !text_edit.text.size() )
			{
				if( text_input.size() < max_characters )
				{
					text_input.insert( text_input.begin() + cursor.index, ' ' );
					cursor.index++;
					update_content();
				}
			}

			else if( e.key.button.scancode == SDL_SCANCODE_LEFT &&
			         !text_edit.text.size() )
			{
				if( cursor.index > 0 )
				{
					cursor.index--;
					update_content();
				}
			}

			else if( e.key.button.scancode == SDL_SCANCODE_RIGHT &&
			         !text_edit.text.size() )
			{
				if( cursor.index < content.size() )
				{
					cursor.index++;
					update_content();
				}
			}
		}
	}

	GuiElement::handle_event( e );
}



GuiVec2 GuiTextField::get_minimum_size() const
{
	return { size.x, size.y };
}



void GuiTextField::update_content()
{
	if( text_input.size() > max_characters )
	{
		text_input.resize( max_characters );
	}

	content = text_input;

	auto used_cursor_index = cursor.index;

	if( text_edit.text.size() )
	{
		content.insert( content.begin() + used_cursor_index, text_edit.text.begin(), text_edit.text.end() );
		used_cursor_index += text_edit.text.size();
	}

	if( content.size() > max_characters )
	{
		content.resize( max_characters );
	}

	auto font_face = Globals::font_face_manager.get_default_font_face();

	if( used_cursor_index > content.size() )
	{
		used_cursor_index = content.size();
	}

	auto pretext = string_unicode{ content.begin(), content.begin() + used_cursor_index };
	auto bbox = get_text_bounding_box(
		font_face.get(),
		pretext,
		used_font_size
	);

	cursor.pos_x = bbox.w;

	refresh();
}



GuiTextArea::~GuiTextArea()
{
}



void GuiTextArea::render() const
{
	try
	{
		const auto font_face = Globals::font_face_manager.get_default_font_face();
		const auto padding = style.get( style_state ).padding;
		const auto cursor_pos = GuiVec2(
			tools::float_to_int( pos.x + padding.x ),
			tools::float_to_int( pos.y + padding.w * 2 )
		);

		const auto window = dynamic_cast<const Window*>( get_root() );
		if( !window )
		{
			throw runtime_error( "No window found" );
			return;
		}

		const auto shader = Globals::shaders.find( "2d" );
		if( shader == Globals::shaders.end() )
		{
			throw runtime_error( "No shader found" );
			return;
		}

		render_unicode( shader->second, content, cursor_pos, window->size, font_face.get() );
	}
	catch( runtime_error &e )
	{
		wcout << "GuiTextArea::render failed: " << e.what() << "\n";
	}
}



void GuiTextArea::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );
}

