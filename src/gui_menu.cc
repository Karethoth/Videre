#pragma once
#include "gui_menu.hh"

using namespace std;
using namespace gui;


void Menu::render() const
{
	GuiElement::render();
}



void Menu::handle_event( const GuiEvent &e )
{
	if( e.type == MOVE )
	{
		pos = e.move.pos;
		fit_children();
	}
	else if( e.type == RESIZE )
	{
		size = e.resize.size;
		fit_children();
	}
	else
	{
		GuiElement::handle_event( e );
	}
}



void Menu::add_child( GuiElementPtr child )
{
	children.push_back( child );
	child->parent = this;
	fit_children();
}



void Menu::fit_children()
{
	auto child_position_y = pos.y;

	for( auto &child : children )
	{
		// TODO: Check if child fits in to the available space
		//       - Make element scrollable (when allowed) if it doesn't

		GuiEvent event;
		event.type = MOVE;
		event.move.pos = { pos.x, child_position_y };

		child->handle_event( event );

		// Here we're assuming that the child has it's height set correctly
		// and that we only have to worry about the width of it
		const auto child_current_size = child->size;
		event.type = RESIZE;
		event.resize.size = { size.w, child_current_size.h };
		child->handle_event( event );

		child_position_y += child_current_size.h;
	}
}


GuiVec2 Menu::get_minimum_size() const
{
	// Fit the menu to the children for now
	// TODO: Add a way to set the maximum height for menu, scrollable when more content than room

	int minimum_height = 0;
	for( const auto& child : children )
	{
		minimum_height += child->size.h;
	}

	return{ size.w, minimum_height };
}

