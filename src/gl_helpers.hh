#pragma once

#include <glm/glm.hpp>
#include "shaderProgram.hh"
#include "gui.hh"


namespace gl
{
	void RenderLine2D( const ShaderProgram &shader, glm::vec2 a, glm::vec2 b );

	glm::vec2 GuiToGlVec(
		const gui::GuiVec2 &coord,
		const gui::GuiVec2 &windowSize
	);
}
