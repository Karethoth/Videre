#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "gui.hh"
#include "shaderProgram.hh"

#include <iostream>

namespace gui
{

struct GlElement : gui::GuiElement
{
	SDL_Window *window;

	GlElement( SDL_Window *window );
	virtual ~GlElement();

	virtual void render() const override;
};

static bool any_gl_errors()
{
	bool had_error = false;
	GLenum err;
	while( (err = glGetError()) != GL_NO_ERROR )
	{
		had_error = true;
		std::wcout << "gl_error: " << err << std::endl;
	}
	return had_error;
}

static void clear_gl_errors()
{
	GLenum err;
	while( (err = glGetError()) != GL_NO_ERROR ) {}
}

} // namespace gui
