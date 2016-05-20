#include "gui.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include <iostream>
#include <exception>

using namespace gui;
using namespace std;


GuiVec2 gui::operator+( const GuiVec2 &a, const GuiVec2 &b )
{
	return { a.x + b.x, a.y + b.y };
}



GuiVec2 gui::operator-( const GuiVec2 &a, const GuiVec2 &b )
{
	return{ a.x + b.x, a.y + b.y };
}


bool GuiElement::in_area( const GuiVec2 &_pos ) const
{
	if( _pos.x >= pos.x && _pos.x <= pos.x + size.w &&
		_pos.y >= pos.y && _pos.y <= pos.y + size.h )
	{
		return true;
	}

	return false;
}


void GuiElement::add_child( GuiElementPtr child)
{
	if( !child )
	{
		throw runtime_error( "Trying to add nullptr as GuiElement child" );
	}

	// Make sure the child isn't one of the parents
	const GuiElement *ptr_parent = parent;
	while( ptr_parent )
	{
		if( ptr_parent == child.get() )
		{
			throw runtime_error( "Trying to add a GuiElement to one of it's (grand)children" );
		}
		ptr_parent = ptr_parent->parent;
	}

	child->parent = this;
	init_child( child.get() );
	children.push_back( child );
}



GuiVec2 GuiElement::get_minimum_size() const
{
	if( children.size() <= 0 )
	{
		return { 10, 10 };
	}

	// For now, we'll assume that no children elements overlap
	GuiVec2 total_size;
	for( auto& child : children )
	{
		total_size = total_size + child->get_minimum_size();
	}

	return total_size;
}



void GuiElement::init_child( GuiElement *child )
{
	GuiEvent event;
	event.type = RESIZE;
	event.resize.size = size;
	child->handle_event( event );
}



void GuiElement::render() const
{
	if( color_bg.a > 0.f )
	{
		auto shader = Globals::shaders.find( "2d" );
		glUseProgram( shader->second.program );
		auto colorUniform = shader->second.get_uniform( "color" );
		glUniform4f( colorUniform, color_bg.r, color_bg.g, color_bg.b, color_bg.a );
		auto window_size = get_root()->size;
		gl::render_quad_2d( shader->second, window_size.to_gl_vec(), pos.to_gl_vec(), size.to_gl_vec() );
	}

	for( auto& child : children )
	{
		child->render();
	}
}



void GuiElement::handle_event( const GuiEvent &e )
{
	switch( e.type )
	{
		case MOVE:
			pos = e.move.pos;
			break;

		case RESIZE:
			size = e.resize.size;
			break;

		default:
			break;
	}

	// Cut off mouse events that are not in the area of the element
	bool not_in_area = false;
	switch( e.type )
	{
		case MOUSE_BUTTON:
			// Allow mouse button released events
			not_in_area = e.mouse_button.state != RELEASED && !in_area( e.mouse_button.pos );
			break;

		case MOUSE_DOUBLE_CLICK:
			not_in_area = !in_area( e.mouse_double_click.pos );
			break;

		case MOUSE_DRAG:
			not_in_area = !in_area( e.mouse_drag.pos_start );
			break;

		case MOUSE_MOVE:
			not_in_area = !in_area( e.mouse_move.pos );
			break;

		default:
			break;
	}

	// TODO: Logic for losing and gaining focus
	if( not_in_area )
	{
		return;
	}

	for( auto& event_listener : event_listeners )
	{
		if( event_listener.first == e.type )
		{
			event_listener.second( this, e );
		}
	}

	for( auto child : children )
	{
		child->handle_event( e );
	}
}



GuiElement *GuiElement::get_root() const
{
	auto current = const_cast<GuiElement*>( this );
	while( current->parent )
	{
		current = current->parent;
	}
	return current;
}
