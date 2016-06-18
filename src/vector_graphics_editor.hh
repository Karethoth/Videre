#pragma once

#include "gui.hh"
#include "window.hh"
#include "vector_img.hh"


struct VectorGraphicsCanvas : gui::GuiElement
{
	gui::GuiVec2 camera_offset;
	float scale;
	vector_img::VectorImg image;

	VectorGraphicsCanvas();

	virtual void handle_event( const gui::GuiEvent &e ) override;
	virtual void render() const override;

  protected:
	void render_vector_img() const;
	void create_context_menu( gui::GuiVec2 tgt_pos );
	glm::vec4 get_canvas_area() const;
};



struct VectorGraphicsProperties : gui::GuiElement
{
};



struct VectorGraphicsToolbar : gui::GuiElement
{
	VectorGraphicsToolbar();

	void handle_event( const gui::GuiEvent &e ) override;
	void fit_children();
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

