#pragma once

#include "gui.hh"
#include "gui_gl.hh"
#include "window.hh"
#include "typedefs.hh"

namespace gui
{
	void render_unicode(
		string_unicode text,
		gui::GuiVec2 position,
		const gui::Window &window,
		std::string font = "default",
		float scale = 1.f
	);



	struct GuiLabel : GuiElement
	{
		string_unicode content;
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};



	struct GuiTextArea : GuiElement
	{
		string_unicode content;
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};
}

