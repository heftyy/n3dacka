#version 140

// Incoming per vertex... position and normal

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec4 vVertex;
layout (location = 2) in vec3 vNormal;

//in vec4 vVertex;
//in vec3 vNormal;

// Set per batch
uniform vec4	vColor;	
uniform vec3	vLightPosition;
uniform mat4	mvpMatrix;
uniform mat4	mvMatrix;
uniform mat3	normalMatrix;

// Color to fragment program
smooth out vec4 vVaryingColor;

void main(void) 
{ 
    // Get surface normal in eye coordinates
    vec3 vEyeNormal = normalize(normalMatrix * vNormal);

    // Get vertex position in eye coordinates
    vec4 vPosition4 = mvMatrix * vVertex;
    vec3 vPosition3 = vPosition4.xyz / vPosition4.w;

    // Get vector to light source
    vec3 vLightDir = normalize(vLightPosition - vPosition3);

    // Dot product gives us diffuse intensity
    float diff = max(0.0, dot(vEyeNormal, vLightDir));

    // Multiply intensity by diffuse color
    vVaryingColor.rgb = diff * vColor.rgb;
    vVaryingColor.a = vColor.a;

    // Let's not forget to transform the geometry
    gl_Position = mvpMatrix * vVertex;
}