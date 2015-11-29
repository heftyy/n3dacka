#version 140

#extension GL_ARB_explicit_attrib_location : enable

varying vec2 vTex;
uniform sampler2D textureUnit0;
uniform vec4 inColor;

out vec4 vFragColor;

void main(void) 
{
	vFragColor = inColor * texture2D(textureUnit0, vTex);
	//vFragColor = vec4(color);
}