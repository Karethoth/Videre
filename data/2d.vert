#version 140

in vec3 vertPos;

uniform mat4 MP;

void main()
{
	gl_Position = MP * vec4(vertPos.x, vertPos.y, 0, 1);
}