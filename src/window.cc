#include "window.hh"
#include "gui_gl.hh"
#include "globals.hh"
#include "mesh.hh"
#include "gl_helpers.hh"

#include <math.h>
#include <chrono>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>


#pragma execution_character_set("utf-8")

using namespace std;
using namespace gui;


Window::Window()
: closed(false), sdl_id(0), active_element(nullptr)
{
	pos = { 0, 0 };
	size = { 600, 400 };

	auto window_ptr = SDL_CreateWindow(
		"Videre",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		size.w, size.h,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
	);

	if( window_ptr )
	{
		sdl_id = SDL_GetWindowID( window_ptr );
		std::wcout << "Created window with id " << sdl_id << std::endl;
		window = sdl2::WindowPtr( window_ptr );
	}
	else
	{
		throw runtime_error{ string{ "Window::Window() - SDL_CreateWindow() failed: " }.append( SDL_GetError() ) };
	}

	auto createContext = SDL_GL_CreateContext( window.get() );
	if( !createContext )
	{
		wcout << SDL_GetError() << endl;
		throw runtime_error{ string{ "Window::Window() - SDL_GL_CreateContext failed: " }.append( SDL_GetError() ) };
	}

}



Window::Window( Window&& other )
{
	using std::swap;
	swap( window,   other.window );
	swap( sdl_id,   other.sdl_id );
	swap( closed,   other.closed );
}



Window& Window::operator=( Window&& other )
{
	using std::swap;
	swap( window,   other.window );
	swap( sdl_id,   other.sdl_id );
	swap( closed,   other.closed );

	return *this;
}



Window::~Window()
{
	wcout << "Closed window" << endl;
}



bool Window::is_initialized() const
{
	return !closed && !!window;
}



void Window::handle_sdl_event( const SDL_Event &e )
{
	GuiEvent gui_event;
	GuiVec2  min_size;

	// TODO: Move to settings
	static const auto mouse_double_click_threshold = chrono::milliseconds( 500 );

	// TODO: Make these non-static
	static int     mouse_down_button;
	static GuiVec2 mouse_down_pos;
	static bool    mouse_down        = false;
	static bool    mouse_dragged     = false;
	static auto    last_mouse_down   = chrono::steady_clock::now();

	switch( e.type )
	{
		case SDL_WINDOWEVENT:
			switch( e.window.event )
			{
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_EXPOSED:
				case SDL_WINDOWEVENT_MOVED:
				case SDL_WINDOWEVENT_MINIMIZED:
				case SDL_WINDOWEVENT_MAXIMIZED:
				case SDL_WINDOWEVENT_RESTORED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					break;

				case SDL_WINDOWEVENT_RESIZED:
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					min_size = get_minimum_size();
					size = { e.window.data1, e.window.data2 };
					if( size.w < min_size.w )
					{
						size.w = min_size.w;
						SDL_SetWindowMinimumSize( window.get(), min_size.w, min_size.h );
						return;
					}

					if( size.h < min_size.h )
					{
						size.h = min_size.h;
						SDL_SetWindowMinimumSize( window.get(), min_size.w, min_size.h );
						return;
					}

					gui_event.type = RESIZE;
					gui_event.resize.size = size;
					handle_event( gui_event );
					break;


				case SDL_WINDOWEVENT_HIDDEN:
				case SDL_WINDOWEVENT_LEAVE:
					gui_event.type = WINDOW_BLUR;
					handle_event( gui_event );
					break;

				case SDL_WINDOWEVENT_ENTER:
					gui_event.type = WINDOW_FOCUS;
					handle_event( gui_event );
					break;

				case SDL_WINDOWEVENT_CLOSE:
					closed = true;
					break;
			}
			break;

		case SDL_MOUSEMOTION:
			if( mouse_down )
			{
				mouse_dragged                    = true;
				gui_event.type                   = MOUSE_DRAG;
				gui_event.mouse_drag.pos_start   = mouse_down_pos;
				gui_event.mouse_drag.pos_current = { e.motion.x, e.motion.y };
				gui_event.mouse_drag.button      = mouse_down_button;
			}
			else
			{
				mouse_dragged            = false;
				gui_event.type           = MOUSE_MOVE;
				gui_event.mouse_move.pos = { e.motion.x, e.motion.y };
			}
			handle_event( gui_event );
			break;

		case SDL_MOUSEBUTTONUP:
			mouse_down = false;
			if( !mouse_dragged )
			{
				gui_event.type                = MOUSE_BUTTON;
				gui_event.mouse_button.button = e.button.button;
				gui_event.mouse_button.state  = RELEASED;
				gui_event.mouse_button.pos    = { e.button.x, e.button.y };
			}
			else
			{
				mouse_dragged = false;
				gui_event.type                     = MOUSE_DRAG_END;
				gui_event.mouse_drag_end.button    = e.button.button;
				gui_event.mouse_drag_end.pos_start = mouse_down_pos;
				gui_event.mouse_drag_end.pos_end   = { e.button.x, e.button.y };
			}
			handle_event( gui_event );
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse_down        = true;
			mouse_down_button = e.button.button;
			mouse_down_pos    = { e.button.x, e.button.y };

			if( last_mouse_down + mouse_double_click_threshold > chrono::steady_clock::now() )
			{
				gui_event.type                      = MOUSE_DOUBLE_CLICK;
				gui_event.mouse_double_click.button = e.button.button;
				gui_event.mouse_double_click.pos    = { e.button.x, e.button.y };
			}
			else
			{
				last_mouse_down               = chrono::steady_clock::now();
				gui_event.type                = MOUSE_BUTTON;
				gui_event.mouse_button.button = e.button.button;
				gui_event.mouse_button.state  = PRESSED;
				gui_event.mouse_button.pos    = { e.button.x, e.button.y };
			}
			handle_event( gui_event );
			break;

		case SDL_MOUSEWHEEL:
			gui_event.type = MOUSE_SCROLL;
			SDL_GetMouseState( &gui_event.mouse_scroll.pos.x, &gui_event.mouse_scroll.pos.y );
			gui_event.mouse_scroll.direction = (e.wheel.y > 0) ?
				gui::GuiDirection::NORTH :
				gui::GuiDirection::SOUTH;
			gui_event.mouse_scroll.value = abs( e.wheel.y );
			handle_event( gui_event );
			break;

		case SDL_TEXTINPUT:
			wcout << "TEXT: " << e.text.text << endl;
			break;

		case SDL_TEXTEDITING:
			SDL_Rect rect = { 100, 100, 200, 200 };
			SDL_SetTextInputRect( &rect );
			wcout << "EDITING: " << e.edit.text << endl;
			break;
	}
}



void Window::render() const
{
	SDL_GL_MakeCurrent( window.get(), gl_context );

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		return;
	}

	glClearColor( 0.2f, 0.2f, 0.2f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	GuiElement::render();

	for( const auto &popup : popup_elements )
	{
		popup->render();
	}

	glFlush();
	SDL_GL_SwapWindow( window.get() );
}



void Window::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		SDL_GL_MakeCurrent( window.get(), gl_context );
		glViewport( 0, 0, e.resize.size.w, e.resize.size.h );

		const auto minimum_size = get_minimum_size();
		auto size_fix = minimum_size;
		bool needs_size_fix = false;

		if( e.resize.size.w < minimum_size.w )
		{
			size_fix.w = minimum_size.w;
			needs_size_fix = true;
		}
		else size_fix.w = size.w;

		if( e.resize.size.h < minimum_size.h )
		{
			size_fix.h = minimum_size.h;
			needs_size_fix = true;
		}
		else size_fix.h = size.h;

		if( needs_size_fix )
		{
			SDL_SetWindowSize( window.get(), size_fix.w, size_fix.h );
			return;
		}
	}

	// For certain mouse events, check if one of the popup elements captures it
	// and if it does, let it handle the event
	auto captured_by_popup = false;
	if( e.type == MOUSE_BUTTON ||
	    e.type == MOUSE_MOVE ||
	    e.type == MOUSE_SCROLL ||
	    e.type == MOUSE_DRAG ||
		e.type == MOUSE_DRAG_END )
	{
		// We want to go over the elements starting from newest one (the last one pushed in)
		for( auto it = popup_elements.rbegin(); it != popup_elements.rend(); it++ )
		{
			GuiVec2 pos;
			switch( e.type )
			{
				case MOUSE_BUTTON:
					pos = e.mouse_button.pos;
					break;

				case MOUSE_MOVE:
					pos = e.mouse_move.pos;
					break;

				case MOUSE_SCROLL:
					pos = e.mouse_scroll.pos;
					break;

				case MOUSE_DRAG:
					pos = e.mouse_drag.pos_start;
					break;

				case MOUSE_DRAG_END:
					pos = e.mouse_drag_end.pos_start;
					break;
			}

			if( (*it)->in_area( pos ) )
			{
				captured_by_popup = true;
				(*it)->handle_event( e );
				break;
			}
			// We want to pass MOUSE_MOVE events to all popup elements
			// so that they can generate their MOUSE_OUT events
			else if( e.type == MOUSE_MOVE )
			{
				(*it)->handle_event( e );
			}
		}
	}

	popup_elements.erase( remove_if( popup_elements.begin(), popup_elements.end(), []( auto element )
	{
		auto deleted = element->deleted;
		if( deleted )
		{
			return true;
		}
		return deleted;
	} ), popup_elements.end());

	if( captured_by_popup )
	{
		return;
	}

	for( auto &child : children )
	{
		child->handle_event( e );
	}

	if( e.type == MOUSE_BUTTON ||
		e.type == MOUSE_MOVE ||
		e.type == MOUSE_SCROLL ||
		e.type == MOUSE_DRAG ||
		e.type == MOUSE_DRAG_END )
	{
		return;
	}

	// If the event wasn't sent to popup elements yet
	// do it now

	for( auto &popup : popup_elements )
	{
		if( e.type == RESIZE )
		{
			// Resize the popup to it's minimum size
			GuiEvent resize_event;
			resize_event.type = GuiEventType::RESIZE;
			resize_event.resize.size.w = 1;
			resize_event.resize.size.h = 1;
			popup->handle_event( resize_event );
		}
		else
		{
			popup->handle_event( e );
		}
	}
}

