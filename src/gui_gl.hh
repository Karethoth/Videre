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


bool any_gl_errors();
void clear_gl_errors();

} // namespace gui

