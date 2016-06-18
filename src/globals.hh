#pragma once

#include "common_types.hh"
#include "window.hh"
#include "shaderProgram.hh"

#include <map>
#include <mutex>
#include <atomic>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H


struct Globals
{
	static std::atomic_bool         should_quit;

	static std::mutex               windows_mutex;
	static std::vector<gui::Window> windows;

	static std::map<std::string, ShaderProgram> shaders;

	static std::mutex freetype_mutex;
	static FT_Library freetype;
	static std::map<string_u8, FT_Face> freetype_faces;
	static std::vector <std::pair<string_u8, FT_Face>> freetype_face_order;
	static std::map<FontFaceIdentity, FontFaceContents> font_face_library;
};

