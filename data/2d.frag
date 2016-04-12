#version 140
out vec4 LFragment;
uniform vec4 color;

void main()
{
	LFragment = vec4( color );
}

