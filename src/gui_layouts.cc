#include "gui_layuts.hh"
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>

using namespace gui;
using namespace std;


GridLayout::GridLayout( int _rows, int _columns )
: rows( _rows ), columns( _columns )
{
}



void GridLayout::handle_event( const GuiEvent &e )
{
	if( e.type != RESIZE && e.type != MOVE )
	{
		GuiElement::handle_event( e );
		return;
	}
}



void SplitLayout::handle_event( const GuiEvent &e )
{
	if( e.type == MOUSE_MOVE )
	{
		const auto mouse_pos = e.mouse_move.pos;
		const int top = pos.y;
		const int bottom = pos.y + size.h;
		const int left = pos.x;
		const int right = pos.x + size.w;

		// Check if mouse is over the bar
		if( is_layout_splitted && !split_bar.is_locked )
		{
			auto rect_pos = (split_bar.axis==VERTICAL) ?
				GuiVec2{ pos.x + split_bar.offset - border_drag_width, pos.y } :
				GuiVec2{ pos.x, pos.y + split_bar.offset - border_drag_width };

			auto rect_size = (split_bar.axis==VERTICAL) ?
				GuiVec2{ border_drag_width*2, size.h } :
				GuiVec2{ size.w, border_drag_width*2 };

			if( mouse_pos.x >= rect_pos.x && mouse_pos.x <= rect_pos.x + rect_size.w &&
				mouse_pos.y >= rect_pos.y && mouse_pos.y <= rect_pos.y + rect_size.h )
			{
				split_bar.is_hilighted = true;
				GuiEvent event;
				event.type = WINDOW_BLUR;
				for( auto &child : children )
				{
					child->handle_event( event );
				}
				return;
			}
			else
			{
				split_bar.is_hilighted = false;
			}
		}

		if( !in_area( mouse_pos ) )
		{
			split_bar.is_visible = false;
			GuiElement::handle_event( e );
			return;
		}

		GuiDirection border_touched = GuiDirection::UNDEFINED_DIRECTION;

		if( mouse_pos.y >= top &&
			mouse_pos.y < top + border_touch_width &&
		    split_axis != HORIZONTAL )
		{
			border_touched = NORTH;
		}
		else if( mouse_pos.y >= bottom - border_touch_width &&
		         mouse_pos.y < bottom &&
		         split_axis != HORIZONTAL )
		{
			border_touched = SOUTH;
		}
		else if( mouse_pos.x >= left &&
		         mouse_pos.x < left + border_touch_width &&
		         split_axis != VERTICAL )
		{
			border_touched = WEST;
		}
		else if( mouse_pos.x >= right - border_touch_width &&
		         mouse_pos.x < right &&
		         split_axis != VERTICAL )
		{
			border_touched = EAST;
		}

		if( border_touched != UNDEFINED_DIRECTION && !is_layout_splitted && is_splitting_allowed )
		{
			split_bar.is_visible = true;
			if( border_touched == NORTH || border_touched == SOUTH )
			{
				split_bar.axis = VERTICAL;
				split_bar.offset = mouse_pos.x - pos.x;
			}
			else
			{
				split_bar.axis = HORIZONTAL;
				split_bar.offset = mouse_pos.y - pos.y;
			}

			// Hide mouse hover related stuff in child elements
			GuiEvent event;
			event.type = WINDOW_BLUR;
			for( auto& child : children )
			{
				child->handle_event( event );
			}
			return;
		}
		else if( !is_layout_splitted )
		{
			split_bar.is_visible = false;
		}
	}

	else if( e.type == WINDOW_BLUR )
	{
		split_bar.is_visible = false;
	}

	else if( e.type == MOUSE_DOUBLE_CLICK )
	{
		if( !is_layout_splitted && split_bar.is_visible )
		{
			split_layout();
			return;
		}
	}

	else if( e.type == RESIZE )
	{
		const auto old_size = size;
		size = e.resize.size;

		if( is_layout_splitted )
		{
			int new_offset = (split_axis == VERTICAL) ?
				split_bar.offset = static_cast<int>(size.w * split_bar.ratio) :
				split_bar.offset = static_cast<int>(size.h * split_bar.ratio);

			const auto a_min = children[0]->get_minimum_size();
			const auto b_min = children[1]->get_minimum_size();

			const auto min_offset = (split_axis == VERTICAL) ? a_min.w : a_min.h;
			const auto max_offset = (split_axis == VERTICAL) ? size.w - b_min.w : size.h - b_min.h;

			if( new_offset > max_offset )
			{
				new_offset = max_offset;
			}
			if( new_offset < min_offset )
			{
				new_offset = min_offset;
			}

			split_bar.offset = new_offset;

			fit_children();

			return;
		}
	}
	else if( e.type == MOUSE_BUTTON )
	{
		const auto mouse_pos = e.mouse_button.pos;

		if( e.mouse_button.state == RELEASED )
		{
			split_bar.is_dragged = false;
		}
		else if( split_bar.is_hilighted )
		{
			split_bar.is_dragged = true;
			return;
		}
	}
	else if( e.type == MOUSE_DRAG )
	{
		const auto mouse_pos = e.mouse_drag.pos_current;

		if( split_bar.is_dragged )
		{
			int new_offset = (split_axis == VERTICAL) ?
				mouse_pos.x - pos.x :
				mouse_pos.y - pos.y;

			const auto a_min = children[0]->get_minimum_size();
			const auto b_min = children[1]->get_minimum_size();

			const auto min_offset = (split_axis == VERTICAL) ? a_min.w : a_min.h;
			const auto max_offset = (split_axis == VERTICAL) ? size.w - b_min.w : size.h - b_min.h;

			if( new_offset > max_offset )
			{
				new_offset = max_offset;
			}
			if( new_offset < min_offset )
			{
				new_offset = min_offset;
			}

			split_bar.offset = new_offset;
			split_bar.ratio = (split_axis == VERTICAL) ?
				(float)split_bar.offset / size.w :
				(float)split_bar.offset / size.h;

			fit_children();
		}
	}

	GuiElement::handle_event( e );
}



void SplitLayout::render( SDL_Renderer *renderer ) const
{
	GuiElement::render( renderer );

	if( is_layout_splitted )
	{
		if( split_bar.is_hilighted )
		{
			SDL_SetRenderDrawColor( renderer, 0, 255, 0, 127 );
		}
		else
		{
			SDL_SetRenderDrawColor( renderer, 255, 127, 0, 255 );
		}


		if( split_bar.axis == VERTICAL )
		{
			SDL_RenderDrawLine(
				renderer,
				pos.x + split_bar.offset,
				pos.y,
				pos.x + split_bar.offset,
				pos.y + size.h
			);
		}
		else
		{
			SDL_RenderDrawLine(
				renderer,
				pos.x,
				pos.y + split_bar.offset,
				pos.x + size.w,
				pos.y + split_bar.offset
			);
		}
	}

	else if( split_bar.is_visible )
	{
		SDL_SetRenderDrawColor( renderer, 255, 0, 0, 127 );

		if( split_bar.axis == VERTICAL )
		{
			SDL_RenderDrawLine(
				renderer,
				pos.x + split_bar.offset,
				pos.y,
				pos.x + split_bar.offset,
				pos.y + size.h
			);
		}
		else
		{
			SDL_RenderDrawLine(
				renderer,
				pos.x,
				pos.y + split_bar.offset,
				pos.x + size.w,
				pos.y + split_bar.offset
			);
		}
	}
}



GuiVec2 SplitLayout::get_minimum_size() const
{
	if( is_layout_splitted )
	{
		const auto a_min = children[0]->get_minimum_size();
		const auto b_min = children[1]->get_minimum_size();

		if( split_axis == VERTICAL )
		{
			return { a_min.w + b_min.w, max( a_min.h, b_min.h ) };
		}
		else
		{
			return { max( a_min.w, b_min.w ), a_min.h + b_min.h };
		}
	}

	return { minimum_window_size, minimum_window_size };
}



void SplitLayout::split_layout()
{
	if( is_layout_splitted )
	{
		wcout << "Trying to split already splitted layout." << endl;
		return;
	}

	if( !is_splitting_allowed )
	{
		wcout << "Trying to split unsplittable layout." << endl;
		return;
	}

	is_layout_splitted = true;
	split_axis = split_bar.axis;
	split_bar.ratio = (split_axis == VERTICAL) ?
		(float)split_bar.offset / size.w :
		(float)split_bar.offset / size.h;

	auto a = make_shared<gui::SplitLayout>();
	auto b = make_shared<gui::SplitLayout>();

	add_child( a );
	add_child( b );

	fit_children();
}



void SplitLayout::fit_children()
{
	if( !is_layout_splitted || children.size() < 2 )
	{
		return;
	}

	GuiEvent event;


	// Move children to correct positions
	event.type = MOVE;
	event.move.pos = { pos.x, pos.y };
	children[0]->handle_event( event );

	event.move.pos = (split_axis == VERTICAL) ?
		GuiVec2{ pos.x + split_bar.offset, pos.y } :
		GuiVec2{ pos.x, pos.y + split_bar.offset };

	children[1]->handle_event( event );

	// Resize children
	event.type = RESIZE;
	event.resize.size = (split_axis == VERTICAL) ?
		GuiVec2{ split_bar.offset, size.h } :
		GuiVec2{ size.w, split_bar.offset };

	children[0]->handle_event( event );

	event.resize.size = (split_axis == VERTICAL) ?
		GuiVec2{ size.w - split_bar.offset, size.h } :
		GuiVec2{ size.w, size.h - split_bar.offset };

	children[1]->handle_event( event );
}



void SplitLayout::init_child( GuiElement *child )
{
}

