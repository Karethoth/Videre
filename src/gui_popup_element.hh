#pragma once

#include "gui.hh"

namespace gui
{
struct PopupElement : GuiElement
{
	bool deleted = false;
	GuiVec2 target_pos;

	// TODO: Context menus should have a context that
	//       would be used to identify them.
	//       - Creator GuiElement should be fine

	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;
};
}

