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
		unsigned font_size,
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
		unsigned font_size
	);



	// Texture for single block/line of text
	struct TextTexture
	{
		TextTexture(
			const string_unicode text,
			const unsigned font_size,
			const GuiVec2 texture_size
		);

		void set_text( const string_unicode text );
		void set_font_size( const unsigned size );
		void set_texture_size( const GuiVec2 texture_size );
		void render( const GuiVec2 position, const GuiVec2 viewport_size, const glm::vec4 color ) const;
		void reset_texture();

	  protected:
		gl::FramebufferObject framebuffer;
		string_unicode content;
		unsigned font_size;
		glm::ivec2 texture_size;
		void update_texture();
	};



	// A "conceptual" line of text, which can
	// actually be wrapped on multiple lines,
	// depending on how much room there is to
	// display it.
	struct TextLine
	{
		string_unicode content{};
		std::vector<TextTexture> textures;
		bool is_dirty;

		TextLine( string_unicode text={} );
		void update( const int row_max_width );
		void render( const GuiVec2 position, const GuiVec2 viewport_size, const glm::vec4 color ) const;
	};



	struct GuiLabel : GuiElement
	{
		bool dynamic_font_size;

		GuiLabel( string_unicode text = string_unicode{}, unsigned size = 16 );
		GuiLabel( string_u8 text, unsigned size = 16 );
		virtual ~GuiLabel();

		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
		virtual GuiVec2 get_minimum_size() const override;

		void set_font_size( unsigned size );

	  protected:
		string_unicode content;
		unsigned font_size;
		GuiVec2 content_size;
		TextTexture text_texture;
		unsigned used_font_size;

		void refresh();
	};



	struct TextInfo
	{
		string_unicode input;
		unsigned max_characters;

		struct
		{
			string_unicode text;
			bool   is_ime_on;
		} edit;

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
	};


	// Will replace TextInfo
	struct TextState
	{
		struct
		{
			size_t row{ 0 };
			size_t col{ 0 };
			bool is_shown{ false };
			std::chrono::steady_clock::time_point next_step;
			std::chrono::milliseconds interval{ 500 };
		} cursor;

		struct
		{
			bool is_active{ false };
			size_t start_row{ 0 };
			size_t start_col{ 0 };
			size_t end_row{ 0 };
			size_t end_col{ 0 };
		} selection;

		struct
		{
			bool is_ime_on{ false };
			size_t row{ 0 };
			size_t col{ 0 };
			string_unicode text;
		} edit;
	};

	struct GuiTextField : GuiLabel
	{
		bool is_active;

		TextInfo text_info;

		GuiTextField();
		virtual ~GuiTextField();

		virtual void update() override;
		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
		virtual GuiVec2 get_minimum_size() const override;

	  protected:
		void update_content();
	};



	struct GuiTextArea : GuiElement
	{
		unsigned font_size;

		std::vector<TextLine> lines{};
		TextState text_state;

		GuiTextArea();
		virtual ~GuiTextArea();

		virtual void render() const override;
		virtual void handle_event( const GuiEvent &e ) override;
	};
}

