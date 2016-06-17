#pragma once

#include "common_types.hh"

string_unicode u8_to_unicode( const string_u8 &str );

// Globals::font_face_library and these should be grouped into a class
GlCharacter add_font_face_character( FT_Face face, unsigned long c );
GlCharacter get_font_face_character( FT_Face face, unsigned long c );
FT_Face get_next_font_face( FT_Face face );
FT_Face get_default_font_face();
void sync_font_face_sizes( size_t font_size );

