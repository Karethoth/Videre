#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include "gui.hh"
#include "shaderProgram.hh"

namespace gui
{

struct GlElement : gui::GuiElement
{
	SDL_Window *window;
	SDL_GLContext gl_context;

	GlElement( SDL_Window *window );
	virtual ~GlElement();

	virtual void render() const override;
};

} // namespace gui
