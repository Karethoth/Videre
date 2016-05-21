#include "gui_popup_element.hh"

using namespace std;
using namespace gui;

void PopupElement::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		return;
	}

	GuiElement::handle_event( e );
}



void PopupElement::render() const
{
	auto x = this;
	GuiElement::render();
}

