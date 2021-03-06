﻿#include "sdl2.hh"
#include "common_tools.hh"
#include "window.hh"
#include "globals.hh"
#include "settings.hh"
#include "gui_gl.hh"
#include "gui_layouts.hh"
#include "gui_text.hh"
#include "gl_helpers.hh"
#include "text_helpers.hh"
#include "shaderProgram.hh"
#include "vector_graphics_editor.hh"
#include "logging.hh"

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
#pragma comment( lib, "SDL2_image.lib" )
#endif


#ifdef main
#undef main
#endif


using namespace std;


void handle_sdl_event( const SDL_Event &e )
{
	try
	{
		if( e.type == SDL_QUIT )
		{
			Globals::should_quit = true;
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
	catch( runtime_error &e )
	{
		LOG( ERRORS, string_u8{ "Exception: " } + e.what() );
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

	// Use window size from the settings
	{
		lock_guard<mutex> settings_lock( settings::settings_mutex );
		const auto window_width  = settings::core["window"]["width"].get<int>();
		const auto window_height = settings::core["window"]["height"].get<int>();
		if( window_width && window_height )
		{
			SDL_SetWindowSize( first_window.window.get(), window_width, window_height );
		}
	}

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
		throw runtime_error{ string{ "glewInit failed" } + (const char*)glewGetErrorString( glewRes ) };
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
}



void render_windows()
{
	try
	{
		lock_guard<mutex> windows_lock{ Globals::windows_mutex };
		for( auto& window : Globals::windows )
		{
			window.render();
		}
	}
	catch( runtime_error &e )
	{
		LOG( ERRORS, string_u8{ "Exception: " } + e.what() );
	}
}



void update_windows()
{
	try
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

			(*it).update();
			++it;
		}
	}
	catch( runtime_error &e )
	{
		LOG( ERRORS, string_u8{ "Exception: " } + e.what() );
	}
}



void load_settings()
{
	LOG( GENERAL, "Loading settings..." );

	wstring_convert<codecvt_utf8<wchar_t>> converter;

	try
	{
		auto data = tools::read_file_contents( "settings.json" );
		auto bytes = converter.to_bytes( data );

		{
			// Require both windows_lock and settings lock 
			lock_guard<mutex> windows_lock( Globals::windows_mutex );
			lock_guard<mutex> settings_lock( settings::settings_mutex );
			settings::core = nlohmann::json::parse( bytes );
		}
	}
	catch( runtime_error &e )
	{
		LOG( ERRORS, string_u8{ "Exception: " } + e.what() );
	}
	catch( ... )
	{
		LOG( ERRORS, "Couldn't parse settings.json" );
	}
}



void apply_settings()
{
	LOG( GENERAL, "Applying settings..." );

	// Lock windows_mutex to prevent updates and rendering on the
	// main loop while we update font faces
	lock_guard<mutex> windows_lock( Globals::windows_mutex );

	Globals::font_face_manager.load_font_faces();

	// Because the font size or font faces used may have changed,
	// the space that some text elements require could be different

	// Thus, we need to kick off a RESIZE event to get all elements
	// to fit around their children

	// TODO: Check if a change actually happened

	gui::GuiEvent refresh_event{};
	refresh_event.type = gui::GuiEventType::REFRESH_RESOURCES;
	gui::any_gl_errors();
	for( auto& window : Globals::windows )
	{
		gui::GuiEvent resize_event;
		resize_event.type = gui::GuiEventType::RESIZE;
		resize_event.resize.size = window.size;
		gui::any_gl_errors();
		window.handle_event( refresh_event );
		gui::any_gl_errors();
		window.handle_event( resize_event );
	}
}



void reload_settings()
{
	load_settings();
	apply_settings();
}



int main( int argc, char **argv )
{
	srand( time( 0 ) );

	logging::Logger::add_log_file( logging::LogCategory::GENERAL, "log.txt" );
	logging::Logger::add_log_file( logging::LogCategory::ERRORS, "errors.txt" );

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

	try
	{
		load_settings();
		init_freetype();
		init_graphics();
		apply_settings();
	}
	catch( runtime_error &e )
	{
		LOG( ERRORS, string_u8{"Initlialization error:"} + e.what() );
		return 1;
	}

	auto defer_sdl_quit = tools::make_defer( []()
	{
		SDL_Quit();
	} );

	auto defer_close_windows = tools::make_defer( []()
	{
		lock_guard<mutex> windows_lock{ Globals::windows_mutex };
		Globals::windows.clear();
	} );


#ifdef  _WIN32
#ifndef _DEBUG
	FreeConsole();
#endif
#endif


	// Set up the GUI
	{
		auto split_layout = make_shared<gui::SplitLayout>();

		split_layout->create_children = []
		{
			auto text_area = make_shared<gui::GuiTextArea>();
			text_area->font_size = 12;

			return gui::GuiElementPtrPair(
				make_shared<VectorGraphicsEditor>(),
				text_area
			);
		};

		Globals::windows[0].add_child( split_layout );
		split_layout->split_at( gui::SplitAxis::VERTICAL, Globals::windows[0].size.w / 2 );
		split_layout->split_bar.is_locked = false;
	}

	// Clear glyphs and tell resources to refresh their resources.
	// Some glyphs are bad at this point
	Globals::font_face_manager.clear_glyphs();
	gui::GuiEvent refresh_event{};
	refresh_event.type = gui::GuiEventType::REFRESH_RESOURCES;

	{
		lock_guard<mutex> windows_lock( Globals::windows_mutex );
		for( auto &window : Globals::windows )
		{
			window.handle_event( refresh_event );
		}
	}

	// Settings file checks
	const auto settings_file_path = string{ "settings.json" };
	const auto settings_file_check_interval = chrono::milliseconds( 1000 );
	auto settings_file_next_check = chrono::steady_clock::now() + chrono::milliseconds( 1000 );
	auto settings_file_timestamp = tools::file_modified( settings_file_path );

	// FPS - average over 10 frames
	const auto fps_step_count = 10;
	const auto fps_cap = 60;
	auto fps_start_time = chrono::system_clock::now();
	auto fps_frames_left = fps_step_count;
	float fps = 1.f;

	auto next_frame = chrono::system_clock::now();

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
			auto milliseconds = chrono::duration_cast<chrono::milliseconds>( difference ).count();
			fps_start_time = fps_end_time;
			if( milliseconds <= 0.f )
			{
				milliseconds = 1;
			}
			fps = 1.f / (milliseconds / fps_step_count / 1000.f);
		}

		SDL_Event event{};
		while( SDL_PollEvent( &event ) )
		{
			handle_sdl_event( event );
		}

		render_windows();
		update_windows();

		auto now = chrono::steady_clock::now();
		if( now >= settings_file_next_check )
		{
			settings_file_next_check = now + settings_file_check_interval;
			auto timestamp = tools::file_modified( settings_file_path );
			if( timestamp > settings_file_timestamp )
			{
				settings_file_timestamp = timestamp;
				reload_settings();
			}
		}
	}

	return 0;
}

