#include "sdl2.hh"
#include "common_tools.hh"
#include "window.hh"
#include "globals.hh"
#include "gui_layuts.hh"

#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <locale>
#include <codecvt>
#include <iostream>
#include <algorithm>
#include <exception>


#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <windows.h>

#pragma comment( lib, "SDL2.lib" )
#pragma comment( lib, "SDL2_ttf.lib" )
#pragma comment( lib, "SDL2_image.lib" )
#endif


#ifdef main
#undef main
#endif


using namespace std;


// Set up the Globals
bool Globals::should_quit = false;

vector<gui::Window> Globals::windows{};
mutex Globals::windows_mutex = {};

void handle_sdl_event( const SDL_Event &e )
{

	if( e.type == SDL_QUIT )
	{
		Globals::should_quit = true;
	}

	else if( e.type == SDL_KEYDOWN )
	{
		if( e.key.keysym.sym == SDLK_ESCAPE )
		{
			Globals::should_quit = true;
		}
	}
	else
	{
		auto window_id = sdl2::event_window_id( e );

		lock_guard<mutex> windows_lock{ Globals::windows_mutex };
		for( auto& window : Globals::windows )
		{
			if( window.sdl_id == window_id || !window_id )
			{
				window.handle_sdl_event( e );
				if( window_id )
				{
					return;
				}
			}
		}

		if( window_id )
		{
			cerr << "Unhandled SDL_Event, target window "
			     << window_id << " not found" << endl;
		}
	}
}



void init_graphics()
{
	// Handle SDL initialization
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO) )
	{
		throw runtime_error{ string{ "SDL_Init() failed - " } +
		      string{ SDL_GetError() } };
	}

	// Create a window
	Globals::windows.emplace_back();
	if( Globals::windows.size() <= 0 ||
		!Globals::windows[0].is_initialized() )
	{
		throw runtime_error{ string{ "Window creation failed" } };
	}

	SDL_SetWindowPosition( Globals::windows[0].window.get(), 50, 50 );
}



void render_windows()
{
	lock_guard<mutex> windows_lock{ Globals::windows_mutex };
	for( auto& window : Globals::windows )
	{
		window.render();
	}
}



void update_windows()
{
	lock_guard<mutex> windows_lock{ Globals::windows_mutex };
	for( auto it = Globals::windows.begin();
	it != Globals::windows.end(); )
	{
		if( (*it).closed )
		{
			it = Globals::windows.erase( it );
			continue;
		}

		SDL_RenderPresent( (*it).renderer.get() );
		++it;
	}
}



int main( int argc, char **argv )
{
	srand( static_cast<unsigned>( time( 0 ) ) );

	#ifdef  _DEBUG
	auto defer_enter_to_quit = tools::make_defer( []()
	{
		wcout << endl << "Press enter to quit... ";
		cin.ignore();
	} );
	#endif

	#ifdef _WIN32
	int const newMode = _setmode( _fileno( stdout ), _O_U8TEXT );
	#endif

	//wstring_convert<codecvt_utf8<wchar_t>> converter;


	try
	{
		init_graphics();
	}
	catch( runtime_error& e )
	{
		wcerr << "ERROR-init_graphics: " << e.what() << endl;
		return 1;
	}

	// Call SDL_Quit at the end
	auto defer_sdl_quit = tools::make_defer( [](){
		SDL_Quit();
	} );

	// Clear windows automatically at the end,
	// while the SDL context is still okay
	auto defer_close_windows = tools::make_defer( []()
	{
		lock_guard<mutex> windows_lock{ Globals::windows_mutex };
		Globals::windows.clear();
	} );

	// If on windows and not in debug mode, detach the console
	#ifdef  _WIN32
	#ifndef _DEBUG
	FreeConsole();
	#endif
	#endif

	auto splittable = make_shared<gui::SplitLayout>();

	Globals::windows[0].add_child( splittable );

	/* Start main loop */
	SDL_Event event;

	while( !Globals::should_quit )
	{
		while( SDL_PollEvent( &event ) )
		{
			handle_sdl_event( event );
		}
		render_windows();
		update_windows();
	}


	return 0;
}

