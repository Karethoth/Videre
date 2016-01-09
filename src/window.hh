#pragma once

#include "sdl2.hh"
#include "gui.hh"

namespace gui
{

struct Window : GuiElement
{
	sdl2::WindowPtr   window;
	sdl2::RendererPtr renderer;
	uint32_t          sdl_id; // SDL Window Id
	bool              closed;


	Window();
	virtual ~Window();

	Window( Window&& );
	Window& operator=( Window&& );

	bool is_initialized() const;
	void handle_sdl_event( const SDL_Event &e );

	void render() const;

	virtual void handle_event( const gui::GuiEvent &e ) override;

	// Delete potentially dangerous constructors and operators
	Window( Window& ) = delete;
	Window& operator=( Window& ) = delete;
};

} // namespace gui

