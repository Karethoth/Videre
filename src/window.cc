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
: closed(false),
  sdl_id(0),
  gl_context( 0 )
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
	swap( window, other.window );
	swap( sdl_id, other.sdl_id );
	swap( closed, other.closed );
	swap( gl_context, other.gl_context );
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
			gui_event.type = TEXT_INPUT;
			strncpy_s(
				gui_event.text_input.text,
				sizeof gui_event.text_input.text,
				e.text.text,
				sizeof gui_event.text_input.text
			);
			handle_event( gui_event );
			break;

		case SDL_TEXTEDITING:
			gui_event.type = TEXT_EDIT;
			strncpy_s(
				gui_event.text_edit.text,
				sizeof gui_event.text_input.text,
				e.text.text,
				sizeof gui_event.text_edit.text
			);
			gui_event.text_edit.start = e.edit.start;
			gui_event.text_edit.length = e.edit.length;
			handle_event( gui_event );
			break;

		case SDL_KEYUP:
		case SDL_KEYDOWN:
			gui_event.type = KEY;
			gui_event.key.state = e.key.state == SDL_PRESSED ? PRESSED : RELEASED;
			gui_event.key.button = e.key.keysym;
			gui_event.key.is_repeat = e.key.repeat != 0;
			handle_event( gui_event );
			break;
	}
}



void Window::update()
{
	sync_popups();
	GuiElement::update();
}



void Window::render() const
{
	SDL_GL_MakeCurrent( window.get(), gl_context );

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		return;
	}

	glViewport( 0, 0, size.w, size.h );
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
	gui::any_gl_errors();
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

	sync_popups();

	// For certain mouse events, check if one of the popup elements captures it
	// and if it does, let it handle the event
	auto captured_by_popup = false;
	if( e.type == MOUSE_BUTTON ||
	    e.type == MOUSE_MOVE ||
	    e.type == MOUSE_SCROLL ||
	    e.type == MOUSE_DRAG ||
		e.type == MOUSE_DRAG_END )
	{
		lock_guard<mutex> popup_elements_lock( popup_elements_mutex );

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
	gui::any_gl_errors();

	if( captured_by_popup )
	{
		return;
	}

	for( auto &child : children )
	{
		child->handle_event( e );
		gui::any_gl_errors();
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

	lock_guard<mutex> popup_elements_lock( popup_elements_mutex );
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
		gui::any_gl_errors();
	}
}



void Window::add_popup( std::shared_ptr<PopupElement> popup )
{
	lock_guard<mutex> popup_lock( popup_element_queues_mutex );
	popup_element_queue_add.push_back( popup );
}



void Window::remove_popup( PopupElement *popup )
{
	lock_guard<mutex> popup_lock( popup_element_queues_mutex );
	popup_element_queue_remove.push_back( popup );
}



void Window::clear_popups()
{
	lock_guard<mutex> popup_elements_lock( popup_elements_mutex );
	lock_guard<mutex> popup_queues_lock( popup_element_queues_mutex );
	for( auto& popup : popup_elements )
	{
		popup_element_queue_remove.push_back( popup.get() );
	}
}



void Window::sync_popups()
{
	lock_guard<mutex> popup_elements_lock( popup_elements_mutex );
	lock_guard<mutex> popup_queues_lock( popup_element_queues_mutex );

	if( popup_element_queue_add.size() )
	{
		popup_elements.insert(
			popup_elements.end(),
			popup_element_queue_add.begin(),
			popup_element_queue_add.end()
		);

		popup_element_queue_add.clear();
	}

	if( popup_element_queue_remove.size() )
	{
		for( auto popup_ptr : popup_element_queue_remove )
		{
			auto it = std::find_if(
				popup_elements.begin(),
				popup_elements.end(),
				[popup_ptr]( auto popup ) {
					return popup.get() == popup_ptr;
				}
			);

			if( it != popup_elements.end() )
			{
				popup_elements.erase( it );
			}
		}
		popup_element_queue_remove.clear();
	}
}

