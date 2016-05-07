#version 330 core

layout (location=0) in vec4 vertex;
varying vec2 tex_coords;

uniform mat4 MP;

void main()
{
	gl_Position = MP * vec4(vertex.xy, 0, 1);
	tex_coords = vertex.zw;
}
