#version 140

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 Position;
layout (location = 2) in float randomFloat;
layout (location = 3) in float startTime;

uniform mat4   mvpMatrix;

out vec4 fColor;

void main(void)
{
	fColor = vec4(1.0,1-startTime/100,0,1.0);
	gl_PointSize = 2;
	gl_Position = mvpMatrix * vec4(Position,1.0);
}