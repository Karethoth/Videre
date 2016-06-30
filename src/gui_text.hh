#pragma once

#include "gui.hh"
#include "gui_gl.hh"
#include "gl_helpers.hh"
#include "window.hh"
#include "common_types.hh"

namespace gui
{
	void render_unicode(
		const ShaderProgram &shader,
		const string_unicode &text,
		const gui::GuiVec2 position,
		const gui::GuiVec2 viewport_size,
		FT_Face face,
		const glm::vec4 color = glm::vec4{ 1.f },
		float scale = 1.f
	);


	std::vector<glm::vec4> get_text_chararacter_rects(
		FT_Face face,
		string_unicode text,
		size_t font_size,
		float scale = 1.f
	);


	// Get line overflow
	string_unicode get_line_overflow(
		string_unicode text,
		float line_width,
		FT_Face face
	);



	// Get line overflow
	GuiVec2 get_text_bounding_box(
		FT_Face face,
		string_unicode text,
		size_t font_size
	);



	struct TextTexture
	{
		TextTexture(
			const string_unicode text,
			const size_t font_size,
			const GuiVec2 texture_size
		);

		void set_text( const string_unicode text );
		void set_font_size( const size_t size );
		void set_texture_size( const GuiVec2 texture_size );
		void render( const GuiVec2 position, const GuiVec2 viewport_size, const glm::vec4 color ) const;
		void reset_texture();

	  protected:
		gl::FramebufferObject framebuffer;
		string_unicode content;
		size_t font_size;
		glm::ivec2 texture_size;
		void update_texture();
	};



	struct GuiLabel : GuiElement
	{
		bool dynamic_font_size;

		GuiLabel( string_unicode text = string_unicode{}, size_t size = 16 );
		GuiLabel( string_u8 text, size_t size = 16 );
		virtual ~GuiLabel();

		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
		virtual GuiVec2 get_minimum_size() const override;

		void set_font_size( size_t size );

	  protected:
		string_unicode content;
		size_t font_size;
		GuiVec2 content_size;
		TextTexture text_texture;
		size_t used_font_size;

		void refresh();
	};



	struct GuiTextField : GuiLabel
	{
		bool is_active;
		size_t max_characters;

		GuiTextField();
		virtual ~GuiTextField();

		virtual void update() override;
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
		virtual GuiVec2 get_minimum_size() const override;

	  protected:
		string_unicode text_input;
		struct
		{
			string_unicode text;
			bool   is_ime_on;
		} text_edit;

		struct
		{
			size_t index{ 0 };
			size_t pos_x{ 0 };
			bool is_shown{ false };
			std::chrono::steady_clock::time_point next_step;
			std::chrono::milliseconds interval{ 500 };
		} cursor;

		struct
		{
			bool is_active{ false };
			size_t start{ 0 };
			size_t end{ 0 };
		} selection;

		void update_content();
	};



	struct GuiTextArea : GuiElement
	{
		string_unicode content;
		size_t font_size;

		virtual ~GuiTextArea();

		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};
}

