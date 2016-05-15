#pragma once

#include "gui.hh"
#include "gui_gl.hh"
#include "window.hh"
#include "typedefs.hh"

namespace gui
{
	void render_text(
		string_u8 text,
		gui::GuiVec2 position,
		const gui::Window &window,
		float scale=1.f,
		std::string font = "default"
	);
};

