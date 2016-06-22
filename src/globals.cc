#include "globals.hh"

using namespace std;


// Set up the Globals
atomic_bool Globals::should_quit{ false };

mutex Globals::windows_mutex{};
vector<gui::Window> Globals::windows{};
map<string, ShaderProgram> Globals::shaders{};

mutex Globals::freetype_mutex{};
FT_Library Globals::freetype;
FontFaceManager Globals::font_face_manager{};

