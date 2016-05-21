#pragma once

#include "gui.hh"
#include "window.hh"

struct VectorGraphicsCanvas : gui::GuiElement
{
};

struct VectorGraphicsProperties : gui::GuiElement
{
};

struct VectorGraphicsToolbar : gui::GuiElement
{
};



struct VectorGraphicsEditor : gui::GuiElement
{
	VectorGraphicsEditor();
	virtual ~VectorGraphicsEditor();

	virtual void render() const override;
	virtual void handle_event( const gui::GuiEvent &e ) override;

  protected:
	using gui::GuiElement::add_child;

	VectorGraphicsToolbar    *toolbar_element;
	VectorGraphicsCanvas     *canvas_element;
	VectorGraphicsProperties *properties_element;
};

