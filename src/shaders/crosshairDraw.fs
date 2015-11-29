#version 140

#extension GL_ARB_explicit_attrib_location : enable

varying vec2 vTex;
uniform sampler2D textureUnit0;

out vec4 vFragColor;

void main(void) 
{
	vec4 color = texture2D(textureUnit0, vTex);
	if ( color.r < 0.2f && color.g < 0.2f && color.b < 0.2f )
		discard;
	vFragColor = vec4(color);
}