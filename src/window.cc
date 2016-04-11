#include "window.hh"
#include "gui_gl.hh"
#include "globals.hh"
#include "mesh.hh"
#include "gl_helpers.hh"

#include <math.h>
#include <iostream>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>


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


const GLfloat triangleVertexData[] = {
	0.0f, 0.8f, 0.0f,
	-0.8f,-0.8f, 0.0f,
	0.8f,-0.8f, 0.0f,
};


glm::mat4 identityMP = glm::mat4(1);

Mesh triangle;

void Window::render() const
{
	SDL_GL_MakeCurrent( window.get(), gl_context );

	auto shader = Globals::shaders.find( "2d" );
	if( shader == Globals::shaders.end() )
	{
		return;
	}

	gui::any_gl_errors();
	glUseProgram( shader->second.program );
	gui::any_gl_errors();

	if( !triangle.vao )
	{
		triangle.vao = 1;
		triangle = CreateMesh(
			shader->second,
			triangleVertexData,
			sizeof( triangleVertexData )
		);
	}

	glClearColor( 0.2, 0.2, 0.2, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT );

	GuiElement::render();

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	// Render triangle
	glBindVertexArray( triangle.vao );

	auto mpUniform = shader->second.GetUniform( "MP" );
	glUniformMatrix4fv( mpUniform, 1, GL_FALSE, &identityMP[0][0] );

	auto colorUniform = shader->second.GetUniform( "color" );
	glUniform4f( colorUniform, 1.0, 1.0, 1.0, 1.0 );

	glDrawArrays( GL_TRIANGLES, 0, triangle.vertex_count );

	// Render line
	gl::RenderLine2D( shader->second, { 0, 0 }, { -0.5, 0.0 } );

	glFlush();

	SDL_GL_SwapWindow( window.get() );
}



void Window::handle_event( const GuiEvent &e )
{
	if( e.type == RESIZE )
	{
		SDL_GL_MakeCurrent( window.get(), gl_context );
		glViewport( 0, 0, e.resize.size.w, e.resize.size.h );
	}
	for( auto child : children )
	{
		child->handle_event( e );
	}
}

