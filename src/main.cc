#include "sdl2.hh"
#include "common_tools.hh"
#include "window.hh"
#include "globals.hh"
#include "settings.hh"
#include "gui_layuts.hh"
#include "gui_gl.hh"
#include "shaderProgram.hh"

#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <locale>
#include <codecvt>
#include <iostream>
#include <algorithm>
#include <exception>

#include <json.hpp>

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
atomic_bool Globals::should_quit{ false };

mutex Globals::windows_mutex{};
vector<gui::Window> Globals::windows{};
map<string, ShaderProgram> Globals::shaders{};


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

	// Set GL attributes
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );

	// Create a window
	Globals::windows.emplace_back();
	if( Globals::windows.size() <= 0 ||
		!Globals::windows[0].is_initialized() )
	{
		throw runtime_error{ string{ "Window creation failed" } };
	}

	SDL_SetWindowPosition( Globals::windows[0].window.get(), 50, 50 );

	auto& firstWindow = Globals::windows[0];

	// Create SDL GL Context
	firstWindow.gl_context = SDL_GL_CreateContext( firstWindow.window.get() );

	if( firstWindow.gl_context == nullptr )
	{
		throw runtime_error( "SDL_GL_CreateContext failed" );
	}

	SDL_GL_MakeCurrent( firstWindow.window.get(), firstWindow.gl_context );

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	auto glewRes = glewInit();
	if( glewRes != GLEW_OK )
	{
		wcout << (char*)glewGetErrorString( glewRes );
		throw runtime_error{ string{ "glewInit failed" } };
	}

	// Get rid of the GL_INVALID_ENUM error caused by glewInit
	gui::clear_gl_errors();

	SDL_GL_SetSwapInterval( 1 );

	// Load default shader
	map<string, GLuint> attributes;
	attributes["vertPos"] = 1;
	map<string, string> shaderList;
	shaderList["default"] = "data/shader";
	shaderList["2d"] = "data/2d";

	gui::any_gl_errors();

	for( const auto &shader : shaderList )
	{
		Shader vertexShader( GL_VERTEX_SHADER, shader.second + ".vert" );
		Shader fragmentShader( GL_FRAGMENT_SHADER, shader.second + ".frag" );
		Globals::shaders.emplace(
			std::piecewise_construct,
			std::forward_as_tuple( shader.first ),
			std::forward_as_tuple( vertexShader, fragmentShader, attributes )
		);
		gui::any_gl_errors();

		// Vertex and fragment shaders free themselves at this point, but as
		// long as the shader program exists OpenGL won't do the final cleanup
	}
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

	wstring_convert<codecvt_utf8<wchar_t>> converter;

	function<void(void)> update_settings = [&]
	{
		wcout << "Updating settings..." << endl;

		lock_guard<mutex> settings_lock( settings::settings_mutex );

		try
		{
			auto data = tools::read_file_contents( "settings.json" );
			auto bytes = converter.to_bytes( data );
			auto settings = nlohmann::json::parse( bytes );
			wcout << "WINDOW X: " << settings["window"]["sizeX"].get<int>() << endl;
			wcout << "WINDOW Y: " << settings["window"]["sizeY"].get<int>() << endl;
		}
		catch( runtime_error &e )
		{
			wcout << "Exception: " << e.what() << endl;
		}
		catch( ... )
		{
			wcout << "Couldn't parse settings.json" << endl;
		}
	};

	auto settings_updater = tools::run_when_file_updated(
		"settings.json",
		update_settings,
		Globals::should_quit
	);

	auto settings_updater_waiter = tools::make_defer( [&] { settings_updater.join(); } );

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

	auto glElement = make_shared<gui::GlElement>( Globals::windows[0].window.get() );

	Globals::windows[0].add_child( glElement );

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

