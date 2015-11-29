#version 140

#extension GL_ARB_explicit_attrib_location : enable

uniform sampler2D texture;

out vec4 vFragColor;

in vec4 fColor;

void main(void)
{ 
	vFragColor = texture(texture, gl_PointCoord);
	
	if (vFragColor.r < 0.2) discard;

	vFragColor = fColor;
	
	//vFragColor = vec4(1,0,0,1);
	
}