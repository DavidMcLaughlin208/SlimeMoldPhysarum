#version 150

// these are from the programmable pipeline system, no need to do anything, sweet!
uniform mat4 modelViewProjectionMatrix;
in vec4 position;

void main()
{
	gl_Position = modelViewProjectionMatrix * position;
}