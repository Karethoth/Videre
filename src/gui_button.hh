#pragma once

#include "gui.hh"

namespace gui
{
struct GuiButton : GuiElement
{
	glm::vec4 color_bg_hover;
	glm::vec4 color_bg_normal;

	gui::GuiEventListener::second_type on_click;


	virtual void handle_event( const GuiEvent &e ) override;
	virtual GuiVec2 get_minimum_size() const override;
};


}