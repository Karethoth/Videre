#version 140

in vec3 vertPos;

void main()
{
	gl_Position = vec4(vertPos.x, vertPos.y, 0, 1);
}