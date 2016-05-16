#pragma once

#include <glm/glm.hpp>
#include "shaderProgram.hh"
#include "gui.hh"
#include "typedefs.hh"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace gl
{
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
		FT_Face face
	);

	glm::vec2 get_text_bounding_box(
		const string_u8 &text,
		FT_Face face
	);
}
