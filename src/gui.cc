#include "gui.hh"
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
	for( auto child : children )
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

	for( auto child : children )
	{
		child->handle_event( e );
	}
}

