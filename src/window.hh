#pragma once

#include "sdl2.hh"
#include "gui.hh"
#include "gui_popup_element.hh"

#include <mutex>

namespace gui
{

class Window : public GuiElement
{
  public:
	sdl2::WindowPtr   window;
	uint32_t          sdl_id; // SDL Window Id
	bool              closed;
	SDL_GLContext     gl_context;
	GuiElementPtr     active_element; // What element is active / has focus

	Window();
	virtual ~Window();

	Window( Window&& );
	Window& operator=( Window&& );

	// Delete potentially dangerous constructors and operators
	Window( Window& ) = delete;
	Window& operator=( Window& ) = delete;


	bool is_initialized() const;
	void handle_sdl_event( const SDL_Event &e );
	void update();

	virtual void render() const override;
	virtual void handle_event( const gui::GuiEvent &e ) override;

	// Popup element handling
	void add_popup( std::shared_ptr<PopupElement> );
	void remove_popup( PopupElement* );
	void clear_popups();


  protected:
	std::mutex popup_elements_mutex;
	std::vector<std::shared_ptr<PopupElement>> popup_elements;

	std::mutex popup_element_queues_mutex;
	std::vector<std::shared_ptr<PopupElement>> popup_element_queue_add;
	std::vector<PopupElement*>                 popup_element_queue_remove;

	// Add queued popups and remove those waiting to be removed
	void sync_popups();
};

} // namespace gui

