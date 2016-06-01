#pragma once

#include "gui.hh"

namespace gui
{

struct GuiButton : GuiElement
{
	gui::GuiEventListener::second_type on_click;

	virtual void handle_event( const GuiEvent &e ) override;
};

}

