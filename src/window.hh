#pragma once

#include "sdl2.hh"
#include "gui.hh"
#include "gui_popup_element.hh"

namespace gui
{

struct Window : GuiElement
{
	sdl2::WindowPtr   window;
	uint32_t          sdl_id; // SDL Window Id
	bool              closed;
	SDL_GLContext     gl_context;
	GuiElementPtr     active_element; // What element is active / has focus

	std::vector<std::shared_ptr<PopupElement>> popup_elements;

	Window();
	virtual ~Window();

	Window( Window&& );
	Window& operator=( Window&& );

	bool is_initialized() const;
	void handle_sdl_event( const SDL_Event &e );

	void render() const override;

	virtual void handle_event( const gui::GuiEvent &e ) override;

	// Delete potentially dangerous constructors and operators
	Window( Window& ) = delete;
	Window& operator=( Window& ) = delete;
};

} // namespace gui

