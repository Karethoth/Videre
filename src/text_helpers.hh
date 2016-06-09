#pragma once

#include "typedefs.hh"
#include "gl_helpers.hh"

#include <map>

struct GlCharacter
{
	GLuint gl_texture;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
	FT_UInt glyph;
};

struct FontFaceIdentity
{
	const FT_Face face_ptr;
	const size_t size;

	FontFaceIdentity( const FT_Face font_face_ptr )
		: face_ptr( font_face_ptr ), size( font_face_ptr->size->metrics.height ) {}

	bool operator< ( const FontFaceIdentity &other) const
	{
		if( face_ptr < other.face_ptr )
		{
			return true;
		}
		else if( face_ptr == other.face_ptr &&
		         size < other.size )
		{
			return true;
		}
		else return false;
	}
};


using FontFaceContents = std::map<unsigned long, GlCharacter>;
static std::map<FontFaceIdentity, FontFaceContents> font_face_library;


string_unicode u8_to_unicode( const string_u8 &str );
GlCharacter add_character( FT_Face face, unsigned long c );
