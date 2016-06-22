#pragma once
#include "gui_menu.hh"
#include "gl_helpers.hh"
#include "globals.hh"
#include <algorithm>

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
		const auto min_size = get_minimum_size();
		size.w = max( min_size.w, e.resize.size.w );
		size.y = max( min_size.h, e.resize.size.h );
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
	child->parent = static_cast<GuiElement*>( this );
	fit_children();
}



void Menu::fit_children()
{
	auto child_position_y = pos.y;

	const auto min_size = get_minimum_size();

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
		const auto child_min_size = child->get_minimum_size();
		event.type = RESIZE;
		event.resize.size = { min_size.w, child_min_size.h };
		child->handle_event( event );

		child_position_y += child_min_size.h;
	}
}



GuiVec2 Menu::get_minimum_size() const
{
	// Fit the menu to the children for now
	// TODO: Add a way to set the maximum height for menu, and
	//       make it scrollable when there's more content than room

	int minimum_height = 0;
	int minimum_width = 0;
	for( const auto& child : children )
	{
		auto child_minimum_size = child->get_minimum_size();
		minimum_height += child_minimum_size.h;
		minimum_width = max( minimum_width, child_minimum_size.w );
	}

	return{ minimum_width, minimum_height };
}



void MenuSpacer::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		size.w = e.resize.size.w;
		return;
	}

	GuiElement::handle_event( e );
}



GuiVec2 MenuSpacer::get_minimum_size() const
{
	return{ 0, size.h };
}

