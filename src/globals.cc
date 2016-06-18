#include "globals.hh";

using namespace std;


// Set up the Globals
atomic_bool Globals::should_quit{ false };

mutex Globals::windows_mutex{};
vector<gui::Window> Globals::windows{};
map<string, ShaderProgram> Globals::shaders{};

mutex Globals::freetype_mutex{};
FT_Library Globals::freetype;
map<string_u8, FT_Face> Globals::freetype_faces{};
vector<pair<string_u8, FT_Face>> Globals::freetype_face_order{};
map<FontFaceIdentity, FontFaceContents> Globals::font_face_library{};
