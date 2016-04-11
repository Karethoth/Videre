#include "gui_gl.hh"

#ifdef _WIN32
#pragma comment(lib, "opengl32.lib")
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

