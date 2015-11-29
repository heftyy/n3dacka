#version 140

#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D texture;

//out vec4 vFragColor;

in vec2 textureCoords;
in vec4 vFragColor;

void main(void)
{ 
	vec4 tmp = texture2D(texture, textureCoords);
	if (tmp.a > 0) gl_FragColor = vec4(vFragColor.rgb,tmp.a);
	else discard;
}