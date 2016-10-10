#include "gui_gl.hh"
#include "logging.hh"

#include <string>

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")

#pragma comment(lib, "freetype240.lib")
//#pragma comment(lib, "freetype263.lib")
#endif

using namespace gui;
using namespace std;


GlElement::GlElement( SDL_Window *window )
{
	if( !window )
	{
		throw runtime_error( "GlElement::GlElement() no window given" );
	}

	this->window = window;
}


GlElement::~GlElement()
{
	/*
	if( gl_context )
	{
		SDL_GL_DeleteContext( gl_context );
	}
	*/
}



void GlElement::render() const
{
	for( auto& child : children )
	{
		child->render();
	}
}



bool gui::any_gl_errors()
{
	bool had_error = false;
	GLenum err = glGetError();
	if( err != GL_NO_ERROR )
	{
		had_error = true;
		LOG( ERRORS, string_u8{ "gl_error: " } + std::to_string(err) + '\n' );
	}
	return had_error;
}



void gui::clear_gl_errors()
{
	GLenum err;
	while( (err = glGetError()) != GL_NO_ERROR ) {}
}

