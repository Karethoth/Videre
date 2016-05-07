#pragma once

#include <glm/glm.hpp>
#include "shaderProgram.hh"
#include "gui.hh"

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

	void render_text_2d(
		const ShaderProgram &shader,
		const glm::vec2 &window_size,
		const std::string &text,
		glm::vec2 pos,
		glm::vec2 scale,
		FT_Face face
	);

	glm::vec2 gui_to_gl_vec(
		const gui::GuiVec2 &coord
	);
}
