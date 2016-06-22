#pragma once

#include "common_types.hh"
#include "window.hh"
#include "shaderProgram.hh"
#include "text_helpers.hh"

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

	static FontFaceManager font_face_manager;
};

