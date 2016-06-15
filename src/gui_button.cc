#include "gui_button.hh"
#include <algorithm>
using namespace std;
using namespace gui;


void GuiButton::handle_event( const GuiEvent &e )
{
	GuiVec2 min_size;

	switch( e.type )
	{
		case MOUSE_BUTTON:
			if( on_click )
			{
				on_click( this, e );
			}
			break;

		case MOUSE_DRAG_END:
			if( in_area( e.mouse_drag_end.pos_start ) &&
				in_area( e.mouse_drag_end.pos_end ) )
			{
				// If mouse drag starts and ends within this element
				// act as if it's just a mouse click
				GuiEvent event;
				event.type = MOUSE_BUTTON;
				event.mouse_button.state = RELEASED;
				event.mouse_button.pos = e.mouse_drag_end.pos_end;
				event.mouse_button.button = e.mouse_drag_end.button;
				handle_event( event );
			}
			break;

		case RESIZE:
			min_size = get_minimum_size();
			size.w = max(min_size.w, e.resize.size.w);
			size.h = max(min_size.h, e.resize.size.h);
			return;
			break;

		case MOUSE_ENTER:
		case MOUSE_LEAVE:
			for( auto child : children )
			{
				child->handle_event( e );
			}
			break;
	}

	GuiElement::handle_event( e );
}

