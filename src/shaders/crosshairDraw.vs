#version 140

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec4 vVertex;
layout (location = 3) in vec2 vTexCoord0;

uniform mat4 mvpMatrix;
varying vec2 vTex;

void main(void)
{ 
	vTex = vTexCoord0;
	gl_Position = mvpMatrix * vVertex;
}