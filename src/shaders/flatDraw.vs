#version 140

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec4 vVertex;

uniform mat4 mvpMatrix;
uniform vec4 inColor;

varying vec4 color;

void main(void)
{
	color = inColor;
	gl_Position = mvpMatrix * vVertex;
}