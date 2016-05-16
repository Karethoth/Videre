#include "sdl2.hh"
#include "common_tools.hh"
#include "window.hh"
#include "globals.hh"
#include "settings.hh"
#include "gui_layuts.hh"
#include "gui_gl.hh"
#include "gl_helpers.hh"
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
FT_Library Globals::freetype;
map<string, FT_Face> Globals::freetype_faces;


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

	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) )
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

	auto& first_window = Globals::windows[0];

	// Create SDL GL Context
	first_window.gl_context = SDL_GL_CreateContext( first_window.window.get() );

	if( first_window.gl_context == nullptr )
	{
		throw runtime_error( "SDL_GL_CreateContext failed" );
	}

	SDL_GL_MakeCurrent( first_window.window.get(), first_window.gl_context );

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

	SDL_GL_SetSwapInterval( 0 );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	// Load default shader
	map<string, GLuint> attributes;
	attributes["vertex"] = 1;
	map<string, string> shader_list;
	shader_list["default"] = "data/shader";
	shader_list["2d"] = "data/2d";

	gui::any_gl_errors();

	for( const auto &shader : shader_list )
	{
		Shader vertex_shader( GL_VERTEX_SHADER, shader.second + ".vert" );
		Shader fragment_shader( GL_FRAGMENT_SHADER, shader.second + ".frag" );
		Globals::shaders.emplace(
			std::piecewise_construct,
			std::forward_as_tuple( shader.first ),
			std::forward_as_tuple( vertex_shader, fragment_shader, attributes )
		);
		gui::any_gl_errors();

		// Vertex and fragment shaders free themselves at this point, but as
		// long as the shader program exists OpenGL won't do the final cleanup
	}
}


void init_freetype()
{
	if( FT_Init_FreeType( &Globals::freetype ) )
	{
		throw runtime_error( "FT_Init_FreeType failed" );
	}

	vector<std::pair<string,string>> font_list;
	font_list.push_back( { "default", "data/fonts/default.ttf" } );

	FT_Face tmp_face;
	for( const auto &font : font_list )
	{
		if( FT_New_Face( Globals::freetype, font.second.c_str(), 0, &tmp_face ) )
		{
			throw runtime_error( string{ "Couldn't load font file \"" } + font.second + "\"" );
		}

		Globals::freetype_faces.insert( { font.first, tmp_face } );
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

	try
	{
		init_freetype();
	}
	catch( runtime_error& e )
	{
		wcerr << "ERROR-init_freetype: " << e.what() << endl;
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

	auto grid = make_shared<gui::GridLayout>(2, 2);
	grid->col_sizes = { {0, gui::AUTO}, {100, gui::PIXELS} };
	grid->row_sizes = { {0, gui::AUTO}, {100, gui::PIXELS} };

	grid->add_child( make_shared<gui::SplitLayout>() );
	grid->add_child( make_shared<gui::SplitLayout>() );
	grid->add_child( make_shared<gui::SplitLayout>() );
	grid->add_child( make_shared<gui::SplitLayout>() );

	Globals::windows[0].add_child( grid );

	auto glElement = make_shared<gui::GlElement>( Globals::windows[0].window.get() );

	Globals::windows[0].add_child( glElement );

	/* Start main loop */
	SDL_Event event;

	auto next_frame = chrono::system_clock::now();

	// FPS - over 10 frames
	const auto fps_step_count = 1;
	const auto fps_cap = 60;
	auto fps_start_time = chrono::system_clock::now();
	auto fps_frames_left = fps_step_count;
	float fps = 1.f;
	while( !Globals::should_quit )
	{
		if( fps > fps_cap )
		{
			this_thread::sleep_until( next_frame );
		}
		else
		{
			this_thread::sleep_until( next_frame );
			// TODO: Check if below some critical limit
		}
		next_frame = chrono::system_clock::now() + chrono::milliseconds( 1000 / fps_cap );

		if( !--fps_frames_left )
		{
			fps_frames_left = fps_step_count;
			auto fps_end_time = chrono::system_clock::now();
			auto difference = (fps_end_time - fps_start_time);
			auto milliseconds = (float)chrono::duration_cast<chrono::milliseconds>( difference ).count();
			fps_start_time = fps_end_time;
			if( milliseconds <= 0.f )
			{
				milliseconds = 1.f;
			}
			fps = 1.f / (milliseconds / fps_step_count / 1000.f);
		}

		while( SDL_PollEvent( &event ) )
		{
			handle_sdl_event( event );
		}
		render_windows();
		update_windows();
	}


	return 0;
}

