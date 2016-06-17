#include "gui_layouts.hh"
#include "gui_gl.hh"
#include "globals.hh"
#include "gl_helpers.hh"

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>

using namespace gui;
using namespace std;


GridLayout::GridLayout( int _columns, int _rows )
: columns( _columns ), rows( _rows ),
  auto_width(0), auto_height(0)
{
	col_widths = vector<GuiPixelsOrPercentage>( columns, {0, AUTO} );
	row_heights = vector<GuiPixelsOrPercentage>( rows, {0, AUTO} );
}



void GridLayout::handle_event( const GuiEvent &e )
{
	switch( e.type )
	{
		case RESIZE:
			size = e.resize.size;
			update_dimensions();
			fit_children();
			break;

		case MOVE:
			pos = e.move.pos;
			update_dimensions();
			fit_children();
			break;

		default:
			GuiElement::handle_event( e );
			return;
	}
}



void GridLayout::render() const
{
	const int grid_element_count = rows * columns;
	int children_rendered = 0;

	for( auto& child : children )
	{
		if( children_rendered++ > grid_element_count )
		{
			break;
		}

		child->render();
	}
}



void GridLayout::update_dimensions()
{
	used_width = 0;
	used_height = 0;

	int auto_width_count = 0;
	int auto_height_count = 0;

	for( auto col_width : col_widths )
	{
		switch( col_width.type )
		{
			case PIXELS:
				used_width += col_width.val;
				break;

			case PERCENTS:
				used_width += (int)(col_width.val / 100.f * size.w);
				break;

			case AUTO:
				auto_width_count++;
				break;
		}
	}

	for( auto row_height : row_heights )
	{
		switch( row_height.type )
		{
			case PIXELS:
				used_height += row_height.val;
				break;

			case PERCENTS:
				used_height += (int)(row_height.val / 100.f * size.h);
				break;

			case AUTO:
				auto_height_count++;
				break;
		}
	}

	auto_width  = auto_width_count  ? (size.w - used_width)  / auto_width_count : 0;
	auto_height = auto_height_count ? (size.h - used_height) / auto_height_count : 0;
}



void GridLayout::fit_children()
{
	int element_index_x = 0;
	int element_index_y = 0;

	GuiVec2 child_position = pos;
	GuiEvent event;

	if( children.size() > (rows * columns) )
	{
		throw runtime_error( "Too many children in GridLayout to fit in to the grid" );
	}

	for( auto& child : children )
	{
		// Move the child to correct position
		event.type = MOVE;
		event.move.pos = child_position;
		child->handle_event( event );

		// Calculate size of the child
		GuiVec2 child_size{ 0,0 };
		auto col_width = col_widths[element_index_x];
		switch( col_width.type )
		{
			case PIXELS:
				child_size.w = col_width.val;
				break;

			case AUTO:
				child_size.w = auto_width;
				break;
			case PERCENTS:
				child_size.w = (int)(col_width.val / 100.f * size.w);
				break;
		}

		auto row_height = row_heights[element_index_y];
		switch( row_height.type )
		{
			case PIXELS:
				child_size.h = row_height.val;
				break;

			case AUTO:
				child_size.h = auto_height;
				break;
			case PERCENTS:
				child_size.h = (int)(row_height.val / 100.f * size.h);
				break;
		}
		
		// Resize the child to fit
		event.type = RESIZE;
		event.resize.size = child_size;
		child->handle_event( event );

		// Move element position to right
		child_position.x += child_size.w;
		element_index_x++;

		// If that was the last element of the row, move to the next row
		if( element_index_x >= columns )
		{
			child_position.x = pos.x;
			child_position.y += child_size.h;

			element_index_y++;
			element_index_x = 0;
		}
	}
}



GuiVec2 GridLayout::get_minimum_size() const
{
	GuiVec2 minimum_size{ 0,0 };

	int current_x = 0;
	int current_y = 0;
	int current_row_min_width = 0;
	int current_row_min_height = 0;
	for( const auto& child : children )
	{
		auto child_min_size = child->get_minimum_size();

		auto col_width = col_widths[current_x];
		if( col_width.type == PIXELS )
		{
			current_row_min_width += col_width.val;
		}
		else
		{
			current_row_min_width += child_min_size.w;
		}

		if( child_min_size.h > current_row_min_height )
		{
			current_row_min_height = child_min_size.h;
		}

		current_x++;
		if( current_x >= columns )
		{
			if( current_row_min_width > minimum_size.w )
			{
				minimum_size.w = current_row_min_width;
			}

			auto row_height = row_heights[current_y];
			if( row_height.type == PIXELS )
			{
					minimum_size.h += row_height.val;
			}
			else
			{
				minimum_size.h += current_row_min_height;
			}

			current_row_min_width = 0;
			current_row_min_height = 0;

			current_y++;
			current_x = 0;
		}
	}
	return minimum_size;
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

			if( mouse_pos.x >= rect_pos.x &&
			    mouse_pos.x <= rect_pos.x + rect_size.w &&
			    mouse_pos.y >= rect_pos.y &&
			    mouse_pos.y <= rect_pos.y + rect_size.h )
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

		if( border_touched != UNDEFINED_DIRECTION &&
		    !is_layout_splitted &&
		    is_splitting_allowed )
		{
			if( (border_touched == NORTH || border_touched == SOUTH) &&
			    (allowed_axes == VERTICAL || allowed_axes == HORIZONTAL_AND_VERTICAL) )
			{
				split_bar.is_visible = true;
				split_bar.axis = VERTICAL;
				split_bar.offset = mouse_pos.x - pos.x;
			}
			else if( (border_touched == WEST || border_touched == EAST) &&
			         (allowed_axes == HORIZONTAL || allowed_axes == HORIZONTAL_AND_VERTICAL) )
			{
				split_bar.is_visible = true;
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
		split_bar.is_hilighted = false;
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
			split_bar.offset = (split_axis == VERTICAL) ?
				static_cast<int>(size.w * split_bar.ratio) :
				static_cast<int>(size.h * split_bar.ratio);

			fit_split_bar();
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
		}
	}
	else if( e.type == MOUSE_DRAG )
	{
		const auto mouse_pos = e.mouse_drag.pos_current;

		if( split_bar.is_dragged )
		{
			split_bar.offset = (split_axis == VERTICAL) ?
				mouse_pos.x - pos.x :
				mouse_pos.y - pos.y;

			fit_split_bar();

			split_bar.ratio = (split_axis == VERTICAL) ?
				(float)split_bar.offset / size.w :
				(float)split_bar.offset / size.h;

			fit_children();
		}
	}
	else if( e.type == MOUSE_DRAG_END )
	{
		split_bar.is_dragged = false;
	}

	GuiElement::handle_event( e );
}



void render_line( int x1, int y1, int x2, int y2 )
{
	auto windowSize = Globals::windows[0].size;
	auto shader = Globals::shaders.find("2d");

	glm::vec2 a = { x1, y1 };
	glm::vec2 b = { x2, y2 };

	gl::render_line_2d( shader->second, { windowSize.w, windowSize.h }, a, b );
}



void SplitLayout::render() const
{
	GuiElement::render();

	auto shader = Globals::shaders.find( "2d" );
	glUseProgram( shader->second.program );
	auto colorUniform = shader->second.get_uniform( "color" );
	auto texturedUniform = shader->second.get_uniform( "textured" );
	glUniform1i( texturedUniform, 0 );

	if( is_layout_splitted )
	{
		if( split_bar.is_hilighted )
		{
			glUniform4f( colorUniform, 0, 1.0, 0, 0.5 );
		}
		else
		{
			glUniform4f( colorUniform, 1.0, 0.5, 0, 1.0 );
		}

		if( split_bar.axis == VERTICAL )
		{
			render_line(
				pos.x + split_bar.offset,
				pos.y,
				pos.x + split_bar.offset,
				pos.y + size.h
			);
		}
		else
		{
			render_line(
				pos.x,
				pos.y + split_bar.offset,
				pos.x + size.w,
				pos.y + split_bar.offset
			);
		}
	}

	else if( split_bar.is_visible )
	{
		glUniform4f( colorUniform, 1.0, 1.0, 1.0, 0.5 );

		if( split_bar.axis == VERTICAL )
		{
			render_line(
				pos.x + split_bar.offset,
				pos.y,
				pos.x + split_bar.offset,
				pos.y + size.h
			);
		}
		else
		{
			render_line(
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

	auto children = create_children();

	add_child( children.first );
	add_child( children.second );

	// Trigger whole window resize event to make sure everything fits together
	// - This is because paddings can possibly move parent elements around
	const auto current_window_size = get_root()->size;

	GuiEvent event;
	event.type = RESIZE;
	event.resize.size = current_window_size;
	get_root()->handle_event( event );
}



void SplitLayout::split_at( SplitAxis axis, int offset )
{
	is_layout_splitted = true;

	split_axis       = axis;
	split_bar.axis   = axis;
	split_bar.offset = offset;
	split_bar.ratio  = (split_axis == VERTICAL) ?
		(float)split_bar.offset / size.w :
		(float)split_bar.offset / size.h;
	split_bar.is_locked = true;

	auto children = create_children();
	add_child( children.first );
	add_child( children.second );
	fit_children();
}



void SplitLayout::fit_split_bar()
{
	if( is_layout_splitted )
	{
		const auto a_min = children[0]->get_minimum_size();
		const auto b_min = children[1]->get_minimum_size();

		auto new_offset = split_bar.offset;

		const auto min_offset = (split_axis == VERTICAL) ?
			a_min.w :
			a_min.h;

		const auto max_offset = (split_axis == VERTICAL) ?
			size.w - b_min.w :
			size.h - b_min.h;

		if( new_offset > max_offset )
		{
			new_offset = max_offset;
		}
		if( new_offset < min_offset )
		{
			new_offset = min_offset;
		}

		split_bar.offset = new_offset;
	}
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

