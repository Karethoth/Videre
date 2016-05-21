#pragma once
#include "gui.hh"

namespace gui
{
struct Menu : GuiElement
{
	virtual void render() const override;
	virtual void handle_event( const GuiEvent &e ) override;
	virtual void add_child( GuiElementPtr child ) override;

  protected:
	void fit_children();
};
}

