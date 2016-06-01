#pragma once

#include "gui.hh"

namespace gui
{
struct PopupElement : GuiElement
{
	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;
	bool deleted = false;
	GuiVec2 target_pos;
};
}

