#include "gui_button.hh"

using namespace std;
using namespace gui;


void GuiButton::handle_event( const GuiEvent &e )
{
	switch( e.type )
	{
		case MOUSE_ENTER:
			color_bg = color_bg_hover;
			break;

		case MOUSE_LEAVE:
			color_bg = color_bg_normal;
			break;

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
	}
	GuiElement::handle_event( e );
}



GuiVec2 GuiButton::get_minimum_size() const
{
	return size;
}

