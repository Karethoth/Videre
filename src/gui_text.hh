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
		std::string font = "default",
		float scale = 1.f
	);

	struct TextArea : GuiElement
	{
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};
}

