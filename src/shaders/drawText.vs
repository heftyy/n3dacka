#version 140

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 Position;
layout (location = 3) in vec2 texCoords;

uniform mat4 mvpMatrix;
uniform vec4 uniformColor;

out vec2 textureCoords;
out vec4 vFragColor;

void main(void)
{
	vFragColor = uniformColor;
	textureCoords = texCoords;
	gl_Position = mvpMatrix * vec4(Position,1.0);
}