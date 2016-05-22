#include "gui_popup_element.hh"

using namespace std;
using namespace gui;

void PopupElement::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );

	if( e.type == RESIZE )
	{
		// Match height to the minimum
		auto minimum_size = get_minimum_size();
		size.h = minimum_size.h;
	}
}



void PopupElement::render() const
{
	GuiElement::render();
}

