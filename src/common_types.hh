#pragma once

#include <map>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


// owner<T> pointer alias
template<typename T>
using owner = T;



// To tell UTF-8 and unicode code point strings apart
using string_u8      = std::string;
using string_unicode = std::vector<uint32_t>;



// struct to hold GL-texture and common info of an unicode code point
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

