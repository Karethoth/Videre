#version 330 core
layout(location = 0) out vec4 fragment;
varying vec2 tex_coords;

uniform vec4 color;
uniform bool textured;
uniform sampler2D tex;

void main()
{
	vec4 sampled = vec4(1);
	if(textured) {
		float a = texture(tex, tex_coords).r;
		sampled = vec4(1.0, 1.0, 1.0, a);
	}

	fragment = vec4( color ) * sampled;
}

