#include "gui_popup_element.hh"
#include <algorithm>

using namespace std;
using namespace gui;


void PopupElement::handle_event( const GuiEvent &e )
{
	GuiElement::handle_event( e );

	if( e.type == RESIZE )
	{
		// Match height to the minimum
		auto minimum_size = get_minimum_size();
		size.w = max( minimum_size.w, e.resize.size.w );
		size.h = max( minimum_size.h, e.resize.size.h );
	}
}



void PopupElement::render() const
{
	GuiElement::render();
}

