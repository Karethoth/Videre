#pragma once

#include "gui.hh"
#include <vector>

namespace gui
{

struct PopupElement : GuiElement
{
	GuiVec2 target_pos;

	// TODO: Context menus should have a context that
	//       would be used to identify them.
	//       - Creator GuiElement should be fine

	virtual void handle_event( const GuiEvent &e ) override;
	virtual void render() const override;
};



struct PopupElementHandleHelper
{
	std::vector<PopupElement*> handles;

	PopupElementHandleHelper( GuiElement& parent );
	void clear_popups();

  protected:
	GuiElement& parent;
};



// Context for elements within popups
// - Used by few buttons at the moment
struct PopupElementContext
{
	PopupElement *popup;
};

}

