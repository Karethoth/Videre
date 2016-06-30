#pragma once

#include "common_types.hh"

#include <mutex>
#include <memory>

string_unicode u8_to_unicode( const string_u8 &str );

using FontFacePtr = std::shared_ptr<FT_FaceRec_>;
FontFacePtr create_font_face( FT_Face face );


struct FontFaceManager
{
	std::pair<GlCharacter, FontFacePtr> get_character( FT_Face face, unsigned long c );
	FontFacePtr get_default_font_face();

	void sync_font_face_sizes( size_t font_size );
	void load_font_faces();
	void clear_glyphs();


  protected:
	std::map<string_u8, FontFacePtr> freetype_faces;
	std::vector<std::pair<string_u8, FontFacePtr>> freetype_face_order;
	std::map<FontFaceIdentity, FontFaceContents> font_face_library;

	std::pair<GlCharacter, FontFacePtr> add_character( FontFacePtr face, unsigned long c );
	GlCharacter get_basic_character_info( FT_Face face, unsigned long c );
	FontFacePtr get_next_font_face( FT_Face face );
	bool character_exists( unsigned long c );


  private:
	std::mutex font_face_mutex;
};

