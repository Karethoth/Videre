#include "sdl2.hh"
#include "common_tools.hh"
#include "window.hh"
#include "globals.hh"
#include "settings.hh"
#include "gui_layouts.hh"
#include "gui_gl.hh"
#include "gl_helpers.hh"
#include "text_helpers.hh"
#include "shaderProgram.hh"
#include "vector_graphics_editor.hh"

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
#include <ftlcdfil.h>

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


void handle_sdl_event( const SDL_Event &e )
{

	try
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
	catch( runtime_error &e )
	{
		wcout << "Exception: " << e.what() << "\n";
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



void load_freetype_font_faces()
{
	lock_guard<mutex> freetype_lock( Globals::freetype_mutex );

	// Clear existing font faces
	for( auto existing_font_face : Globals::freetype_face_order )
	{
		FT_Done_Face( existing_font_face.second );
	}

	Globals::freetype_face_order.clear();
	Globals::freetype_faces.clear();

	for( auto& font_faces : Globals::font_face_library )
	{
		for( auto& font_face : font_faces.second )
		{
			glDeleteTextures( 1, &font_face.second.gl_texture );
		}
	}

	Globals::font_face_library.clear();

	// Parse fonts from the settings.json to a list
	vector<std::pair<string_u8, string_u8>> font_list;

	{
		lock_guard<mutex> settings_lock( settings::settings_mutex );
		const auto fonts = settings::core["fonts"];
		for( auto &font : fonts )
		{
			font_list.push_back( { font["name"].get<string_u8>(), font["path"].get<string_u8>() } );
		}
	}

	FT_Library_SetLcdFilter( Globals::freetype, FT_LCD_FILTER_DEFAULT );

	// Try to load the fonts
	FT_Face tmp_face;
	for( const auto &font : font_list )
	{
		if( !tools::is_file_readable( font.second ) )
		{
			wcout << "Failed to open font(" << font.first.c_str()
				  << ") " << font.second.c_str() << "\n";
			continue;
		}

		try
		{
			FT_New_Face( Globals::freetype, font.second.c_str(), 0, &tmp_face );
			Globals::freetype_face_order.push_back( { font.first, tmp_face } );
			Globals::freetype_faces.insert( { font.first, tmp_face } );
		}
		catch( ... )
		{
			wcout << "Failed to load font(" << font.first.c_str()
				  << ") " << font.second.c_str() << "\n";
		}
	}

	if( !Globals::freetype_faces.size() )
	{
		wcout << "Warning: No fonts loaded. Expect the unexpected behaviour on their part.\n";
	}

	// Set the initial font size for all font faces
	sync_font_face_sizes( 16 );
}



void init_freetype()
{
	if( FT_Init_FreeType( &Globals::freetype ) )
	{
		throw runtime_error( "FT_Init_FreeType failed" );
	}

	load_freetype_font_faces();
}



void render_windows()
{
	try {
		lock_guard<mutex> windows_lock{ Globals::windows_mutex };
		for( auto& window : Globals::windows )
		{
			window.render();
		}
	}
	catch( runtime_error &e )
	{
		wcout << "Exception: " << e.what() << "\n";
	}
}



void update_windows()
{
	try {
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
	catch( runtime_error &e )
	{
		wcout << "Exception: " << e.what() << "\n";
	}
}



void update_settings()
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;

	wcout << "Updating settings...\n";

	try
	{
		auto data = tools::read_file_contents( "settings.json" );
		auto bytes = converter.to_bytes( data );

		{
			lock_guard<mutex> settings_lock( settings::settings_mutex );
			settings::core = nlohmann::json::parse( bytes );
		}

		// Lock windows_mutex to prevent updates and rendering on the
		// main loop while we update font faces
		lock_guard<mutex> windows_lock( Globals::windows_mutex );

		load_freetype_font_faces();

		// Because the font size or font faces used may have changed,
		// the space that some text elements require could be different

		// Thus, we need to kick off a RESIZE event to get all elements
		// to fit around their children

		// TODO: Check if a change actually happened
		for( auto& window : Globals::windows )
		{
			gui::GuiEvent resize_event;
			resize_event.type = gui::GuiEventType::RESIZE;
			resize_event.resize.size = window.size;
			window.handle_event( resize_event );
		}

	}
	catch( runtime_error &e )
	{
		wcout << "Exception: " << e.what() << endl;
	}
	catch( ... )
	{
		wcout << "Couldn't parse settings.json" << endl;
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

	try
	{
		init_freetype();
	}
	catch( runtime_error &e )
	{
		wcerr << "Initialization error:" << e.what() << endl;
		return 1;
	}

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
	catch( runtime_error &e )
	{
		wcerr << "Initialization error:" << e.what() << endl;
		return 1;
	}

	auto defer_sdl_quit = tools::make_defer( [](){
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

	auto split_layout = make_shared<gui::SplitLayout>();
	split_layout->create_children = [] {
		return gui::GuiElementPtrPair(
			make_shared<VectorGraphicsEditor>(),
			make_shared<gui::SplitLayout>()
		);
	};

	Globals::windows[0].add_child( split_layout );
	split_layout->split_at( gui::SplitAxis::VERTICAL, Globals::windows[0].size.w / 2 );
	split_layout->split_bar.is_locked = false;


	/* Main loop */
	SDL_Event event;

	auto next_frame = chrono::system_clock::now();

	// FPS - average over 10 frames
	const auto fps_step_count = 10;
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

