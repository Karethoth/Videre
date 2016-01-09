#include "window.hh"

#include <iostream>
#include <chrono>


using namespace std;
using namespace gui;


Window::Window()
: closed(false), sdl_id(0)
{
	pos = { 0, 0 };
	size = { 460, 320 };

	auto window_ptr = SDL_CreateWindow(
		"Videre",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		size.w, size.h,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
	);

	if( window_ptr )
	{
		sdl_id = SDL_GetWindowID( window_ptr );
		std::wcout << "Created window with id " << sdl_id << std::endl;
		window = sdl2::WindowPtr( window_ptr );
	}
	else
	{
		wcerr << "Window::Window() - SDL_CreateWindow() failed: "
		     << SDL_GetError() << endl;

		return;
	}


	auto renderer_ptr = SDL_CreateRenderer( window_ptr, 0, SDL_RENDERER_ACCELERATED );
	if( renderer_ptr )
	{
		renderer = sdl2::RendererPtr( renderer_ptr );
	}
	else
	{
		wcerr << "Window::Window() - SDL_CreateRenderer() failed: "
		     << SDL_GetError() << endl;

		return;
	}
}



Window::Window( Window&& other )
{
	using std::swap;
	swap( window,   other.window );
	swap( renderer, other.renderer );
	swap( sdl_id,   other.sdl_id );
	swap( closed,   other.closed );
}



Window& Window::operator=( Window&& other )
{
	using std::swap;
	swap( window,   other.window );
	swap( renderer, other.renderer );
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
	return !closed && !!window && !!renderer;
}



void Window::handle_sdl_event( const SDL_Event &e )
{
	GuiEvent gui_event;
	GuiVec2  min_size;
	static GuiVec2 mouse_down_pos;
	static bool  mouse_down = false;
	static auto last_mouse_down = chrono::steady_clock::now();
	static const auto mouse_double_click_threshold = chrono::milliseconds( 500 );

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
				case SDL_WINDOWEVENT_FOCUS_GAINED:
				case SDL_WINDOWEVENT_FOCUS_LOST:
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
				gui_event.type = MOUSE_DRAG;
				gui_event.mouse_drag.pos_start = mouse_down_pos;
				gui_event.mouse_drag.pos_current = { e.motion.x, e.motion.y };
			}
			else
			{
				gui_event.type = MOUSE_MOVE;
				gui_event.mouse_move.pos = { e.motion.x, e.motion.y };
			}
			handle_event( gui_event );
			break;

		case SDL_MOUSEBUTTONUP:
			mouse_down = false;
			gui_event.type = MOUSE_BUTTON;
			gui_event.mouse_button.button = e.button.button;
			gui_event.mouse_button.state = RELEASED;
			gui_event.mouse_button.pos = { e.button.x, e.button.y };
			handle_event( gui_event );
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse_down = true;
			mouse_down_pos = { e.button.x, e.button.y };
			if( last_mouse_down + mouse_double_click_threshold > chrono::steady_clock::now() )
			{
				gui_event.type = MOUSE_DOUBLE_CLICK;
				gui_event.mouse_double_click.button = e.button.button;
				gui_event.mouse_double_click.pos = { e.button.x, e.button.y };
			}
			else
			{
				last_mouse_down = chrono::steady_clock::now();
				gui_event.type = MOUSE_BUTTON;
				gui_event.mouse_button.pos = { e.button.x, e.button.y };
			}
			handle_event( gui_event );
			break;

	}
}



void Window::render() const
{
	SDL_SetRenderDrawColor( renderer.get(), 0, 0, 0, 255 );
	SDL_RenderClear( renderer.get() );
	GuiElement::render( renderer.get() );
}



void Window::handle_event( const GuiEvent &e )
{
	for( auto child : children )
	{
		child->handle_event( e );
	}
}

