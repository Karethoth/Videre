#pragma once

#include "gui.hh"
#include "gui_gl.hh"
#include "window.hh"
#include "common_types.hh"

namespace gui
{
	void render_unicode(
		const ShaderProgram &shader,
		string_unicode text,
		gui::GuiVec2 position,
		const gui::Window &window,
		FT_Face face,
		glm::vec4 color = glm::vec4{ 1.f },
		float scale = 1.f
	);



	// Get line overflow
	string_unicode get_line_overflow(
		string_unicode text,
		float line_width,
		FT_Face face
	);



	// Get line overflow
	float get_line_width(
		string_unicode text,
		FT_Face face
	);



	struct GuiLabel : GuiElement
	{
		string_unicode content;
		size_t font_size;
		bool dynamic_font_size;

		GuiLabel( string_unicode text = string_unicode{}, size_t size = 16 );
		GuiLabel( string_u8 text, size_t size = 16 );

		virtual void render() const override;
		virtual GuiVec2 get_minimum_size() const override;
	};



	struct GuiTextField : GuiLabel
	{
		bool is_active;
		size_t max_characters;

		GuiTextField();
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};



	struct GuiTextArea : GuiElement
	{
		string_unicode content;
		size_t font_size;

		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};
}

