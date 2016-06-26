#include "text_helpers.hh"
#include "globals.hh"
#include "gui_gl.hh"
#include "settings.hh"

#include <iostream>
#include <ftlcdfil.h>

using namespace std;

namespace
{
	size_t get_unicode_octet_count( const char byte )
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



	unsigned long u8_char_to_unicode( unsigned long c )
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
			val |= (c & 0x1f00) >> 2;
			val |= (c & 0x3f);
		}
		else val = c;

		return val;
	}



	string_unicode::value_type read_next_unicode_code_point(
		const char *str,
		const char * const end,
		size_t &bytes
	)
	{
		unsigned long character = 0;

		if( str >= end )
		{
			bytes = 1;
			return 0;
		}

		auto octet_count = get_unicode_octet_count( *str );
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

		return u8_char_to_unicode( character );
	}
}



FontFacePtr create_font_face( FT_Face face )
{
	return FontFacePtr( face, []( FT_Face ptr ) {
		if( ptr ) FT_Done_Face( ptr );
	} );
}



string_unicode u8_to_unicode( const string_u8 &str )
{
	string_unicode unicode_str;
	unicode_str.reserve( str.size() );

	const char* str_ptr = str.data();
	const char* const str_end = static_cast<const char*>(str.data() + str.size());
	string_unicode::value_type code_point = 0;
	size_t bytes_read = 0;

	while( (code_point = read_next_unicode_code_point( str_ptr, str_end, bytes_read )) && bytes_read )
	{
		unicode_str.push_back( code_point );
		str_ptr += bytes_read;
	}

	unicode_str.shrink_to_fit();
	return unicode_str;
}



pair<GlCharacter, FontFacePtr> FontFaceManager::add_character( FontFacePtr face, unsigned long c )
{
	auto glyph_index = FT_Get_Char_Index( face.get(), c );

	// If the glyph wasn't found, try to use the next font face
	if( !glyph_index )
	{
		const auto next_face = get_next_font_face( face.get() );
		if( next_face )
		{
			return add_character( next_face, c );
		}
		
		// This was the last face and no glyph was found,
		// continue and render the placeholder glyph
	}

	auto face_ptr = face.get();

	auto err = FT_Load_Char( face_ptr, c, FT_LOAD_RENDER );
	if( err )
	{
		std::wcout << "ERROR::FREETYTPE: Failed to load Glyph(" << err << ")" << std::endl;
		return {};
	}


	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		throw runtime_error( "Couldn't find 2d shader" );
	}
	gui::any_gl_errors();

	glUseProgram( shader->second.program );
	gui::any_gl_errors();

	// Generate texture
	GLuint texture=0;
	glGenTextures( 1, &texture );
	if( !texture )
	{
		gui::any_gl_errors();
		return{ { 0 }, nullptr };
	}

	glBindTexture( GL_TEXTURE_2D, texture );
	gui::any_gl_errors();

	// Set texture options
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,
		face_ptr->glyph->bitmap.width,
		face_ptr->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face_ptr->glyph->bitmap.buffer
	);
	gui::any_gl_errors();

	glBindTexture( GL_TEXTURE_2D, 0 );
	gui::any_gl_errors();

	// Now store character for later use
	GlCharacter character = {
		texture,
		glm::ivec2( face_ptr->glyph->bitmap.width, face_ptr->glyph->bitmap.rows ),
		glm::ivec2( face_ptr->glyph->bitmap_left, face_ptr->glyph->bitmap_top ),
		(GLuint)face_ptr->glyph->advance.x,
		glyph_index,
		(face_ptr->size->metrics.height)/64
	};

	font_face_library[FontFaceIdentity( face_ptr )].insert( { c, character } );
	return { character, face };
}



pair<GlCharacter, FontFacePtr> FontFaceManager::get_character( FT_Face face, unsigned long c )
{
	lock_guard<mutex> font_face_library_lock( font_face_mutex );
	FontFacePtr passed_face;

	for( auto &font_face : freetype_face_order )
	{
		if( font_face.second.get() == face )
		{
			passed_face = font_face.second;
		}
	}


	if( !character_exists( c ) )
	{
		return add_character( passed_face, c );
	}

	// Try with the given font face
	const auto& font_face_contents = font_face_library[{face}];
	auto character_info = font_face_contents.find( c );
	if( character_info != font_face_contents.end() )
	{
		return { character_info->second, nullptr };
	}

	// If the given font face didn't have it, loop over all of the font faces
	for( auto &font_face : freetype_face_order )
	{
		const auto& face_contents = font_face_library[{font_face.second.get()}];

		const auto character_info = face_contents.find( c );
		if( character_info != face_contents.end() )
		{
			return { character_info->second, font_face.second };
		}
	}

	return { { 0 }, nullptr };
}



GlCharacter FontFaceManager::get_basic_character_info( FT_Face face, unsigned long c )
{
	auto glyph_index = FT_Get_Char_Index( face, c );
	if( !glyph_index )
	{
		const auto next_face = get_next_font_face( face );
		if( next_face )
		{
			return get_basic_character_info( next_face.get(), c );
		}

		// This was the last face and no glyph was found,
		// continue and render the placeholder glyph
	}

	auto err = FT_Load_Char( face, c, FT_LOAD_DEFAULT );
	if( err )
	{
		throw runtime_error( "FT_Load_Char failed for code point " + to_string( c ) );
	}

	return {
		0,
		glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows ),
		glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
		(GLuint)face->glyph->advance.x,
		glyph_index,
		1
	};
}



bool FontFaceManager::character_exists( unsigned long c )
{
	// If the given font face didn't have it, loop over all of the font faces
	for( auto &font_face : freetype_face_order )
	{
		const auto& font_face_contents = font_face_library[{font_face.second.get()}];
		const auto character_info = font_face_contents.find( c );
		if( character_info != font_face_contents.end() )
		{
			return true;
		}
	}

	return false;
}



FontFacePtr FontFaceManager::get_next_font_face( FT_Face face )
{
	auto current = freetype_face_order.begin();
	while( current != freetype_face_order.end() )
	{
		if( current->second.get() == face )
		{
			break;
		}
		else current++;
	}

	if( current     == freetype_face_order.end() ||
		current + 1 == freetype_face_order.end() )
	{
		return 0;
	}

	const auto next = current + 1;
	return next->second;
}



FontFacePtr FontFaceManager::get_default_font_face()
{
	lock_guard<mutex> font_face_library_lock( font_face_mutex );

	if( !freetype_face_order.size() )
	{
		throw runtime_error( "No fonts to choose the default font from" );
	}

	auto font = freetype_face_order.begin();
	return font->second;
}



void FontFaceManager::sync_font_face_sizes( size_t font_size )
{
	lock_guard<mutex> font_face_library_lock( font_face_mutex );
	for( auto font : freetype_face_order )
	{
		FT_Set_Pixel_Sizes( font.second.get(), 0, static_cast<FT_UInt>( font_size ) );
	}
}



void FontFaceManager::load_font_faces()
{
	clear_glyphs();

	lock_guard<mutex> font_face_library_lock( font_face_mutex );

	// Clear existing font faces
	freetype_faces.clear();
	freetype_face_order.clear();

	// Parse fonts from the settings.json to a list
	vector<std::pair<string_u8, string_u8>> font_list;

	{
		lock_guard<mutex> settings_lock( settings::settings_mutex );
		const auto fonts = settings::core["fonts"];
		for( auto &font : fonts )
		{
			font_list.push_back( { font["name"].get<string_u8>(), font["path"].get<string_u8>() } );
		}
	}

	FT_Library_SetLcdFilter( Globals::freetype, FT_LCD_FILTER_DEFAULT );

	// Try to load the fonts
	FT_Face tmp_face = nullptr;
	for( const auto &font : font_list )
	{
		if( !tools::is_file_readable( font.second ) )
		{
			wcout << "Failed to open font(" << font.first.c_str()
			      << ") " << font.second.c_str() << "\n";
			continue;
		}

		try
		{
			FT_New_Face( Globals::freetype, font.second.c_str(), 0, &tmp_face );
			auto face_ptr = create_font_face( tmp_face );
			freetype_face_order.push_back( { font.first, face_ptr } );
			freetype_faces.insert( { font.first, face_ptr } );
		}
		catch( ... )
		{
			wcout << "Failed to load font(" << font.first.c_str()
			      << ") " << font.second.c_str() << "\n";
		}
	}

	if( !freetype_faces.size() )
	{
		wcout << "Warning: No fonts loaded. Expect the unexpected behaviour on their part.\n";
	}
}



void FontFaceManager::clear_glyphs()
{
	lock_guard<mutex> font_face_library_lock( font_face_mutex );

	for( auto& font_faces : font_face_library )
	{
		for( auto& font_face : font_faces.second )
		{
			if( font_face.second.gl_texture )
			{
				glDeleteTextures( 1, &font_face.second.gl_texture );
			}
		}
	}

	font_face_library.clear();
}


