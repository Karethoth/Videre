#pragma once

#include "common_types.hh"

string_unicode u8_to_unicode( const string_u8 &str );

// Globals::font_face_library and this should be grouped into a class
GlCharacter add_character( FT_Face face, unsigned long c );
