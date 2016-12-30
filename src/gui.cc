#include "gui.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include "gui_gl.hh"

#include <iostream>
#include <algorithm>
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



GuiMouseHoverHelper::GuiMouseHoverHelper( const GuiElement &target )
: target_element( target ), is_over( false )
{
}



GuiEvent GuiMouseHoverHelper::generate_event( const GuiEvent &e )
{
	GuiEvent event;
	event.type = NO_EVENT;

	const bool was_over = is_over;
	bool is_in_area = false;

	switch( e.type )
	{
		case MOUSE_MOVE:
			is_in_area = target_element.in_area( e.mouse_move.pos );
			break;

		default:
			return event;
	}

	if( !was_over && is_in_area )
	{
		event.type = MOUSE_ENTER;
		is_over = true;
	}
	else if( was_over && !is_in_area )
	{
		event.type = MOUSE_LEAVE;
		is_over = false;
	}

	return event;
}



const GuiElementStyle::StyleRules& GuiElementStyle::get( const GuiElementStyleState state ) const
{
	switch( state )
	{
		case HOVER:
			return hover;

		case NORMAL:
		default:
			return normal;
	}
}



GuiElement::GuiElement()
: parent( nullptr ),
  hover_helper( *this ),
  style_state( NORMAL )
{
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



void GuiElement::add_child( GuiElementPtr child )
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



void GuiElement::update()
{
	for( auto& child : children )
	{
		child->update();
	}
}


void GuiElement::render() const
{
	const auto color_bg = style.get( style_state ).color_bg;
	if( color_bg.a > 0.f )
	{
		auto shader = Globals::shaders.find( "2d" );
		glUseProgram( shader->second.program );
		auto colorUniform = shader->second.get_uniform( "color" );
		glUniform4f( colorUniform, color_bg.r, color_bg.g, color_bg.b, color_bg.a );
		auto window_size = get_root()->size;
		gl::render_quad_2d( shader->second, window_size.to_gl_vec(), pos.to_gl_vec(), size.to_gl_vec() );
		gui::any_gl_errors();
	}

	for( auto& child : children )
	{
		child->render();
	}
}



void GuiElement::handle_event( const GuiEvent &e )
{
	gui::any_gl_errors();
	switch( e.type )
	{
		case MOUSE_ENTER:
			style_state = HOVER;
			break;

		case MOUSE_LEAVE:
			style_state = NORMAL;
			break;

		case MOVE:
			pos = e.move.pos;
			break;

		default:
			break;
	}

	if( e.type == RESIZE )
	{
		const auto min_size = get_minimum_size();
		size.w = max( min_size.w, e.resize.size.w );
		size.h = max( min_size.h, e.resize.size.h );
	}

	auto hover_event = hover_helper.generate_event( e );
	if( hover_event.type != NO_EVENT )
	{
		handle_event( hover_event );
	}
	gui::any_gl_errors();

	for( auto& event_listener : event_listeners )
	{
		if( event_listener.first == e.type )
		{
			event_listener.second( this, e );
		}
	}

	// Filter events that shouldn't be passed to children
	switch( e.type )
	{
		case MOUSE_ENTER:
		case MOUSE_LEAVE:
			return;
	}

	gui::any_gl_errors();
	for( auto &child : children )
	{
		child->handle_event( e );
	}
}



const GuiElement *GuiElement::get_root() const
{
	auto current = this;
	while( current->parent )
	{
		current = current->parent;
	}
	return current;
}



GuiElement *GuiElement::get_root()
{
	auto current = this;
	while( current->parent )
	{
		current = current->parent;
	}
	return current;
}


