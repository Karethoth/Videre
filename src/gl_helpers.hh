#pragma once

#include <glm/glm.hpp>
#include "shaderProgram.hh"
#include "gui.hh"
#include "common_types.hh"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace gl
{
	struct FramebufferObject
	{
		GLuint     framebuffer_id=0;
		GLuint     texture_id=0;
		glm::ivec2 texture_size={0, 0};

		void bind();
		void resize( const glm::ivec2 size );
		~FramebufferObject();
	};



	void render_line_2d(
		const ShaderProgram &shader,
		const glm::vec2 &window_size,
		glm::vec2 a,
		glm::vec2 b
	);



	void render_quad_2d(
		const ShaderProgram &shader,
		const glm::vec2 &window_size,
		glm::vec2 pos,
		glm::vec2 size
	);



	// Renders text and returns the width of rendered string
	size_t render_text_2d(
		const ShaderProgram &shader,
		const glm::vec2 &window_size,
		const string_u8 &text,
		glm::vec2 pos,
		glm::vec2 scale,
		FT_Face face,
		size_t font_size
	);
}

