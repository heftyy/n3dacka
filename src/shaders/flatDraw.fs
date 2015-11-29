#version 140

out vec4 vFragColor;

varying vec4 color;

void main(void) 
{
	vFragColor = color;
}