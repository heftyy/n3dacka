#version 140

#extension GL_ARB_explicit_attrib_location : enable

layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Velocity;
layout (location = 2) in float randomFloat;
layout (location = 3) in float startTime;
layout (location = 4) in float rotationVertex;
layout (location = 5) in int ifReset;

out vec3 position_out;
out vec3 velocity_out;
out float randomFloat_out;
out float startTime_out;
out float rotationVertex_out;
out int ifReset_out;

uniform float deltaTime;
uniform float rotation;
uniform vec2 playerPos;

void updateVelocity()
{
	float rotationTMP;
	if ( abs(rotation - rotationVertex) > 0 ) 
	{
		rotationTMP = rotation - rotationVertex;
		rotationVertex_out = rotation;
	}
	else 
	{
		rotationTMP = 0;
		rotationVertex_out = rotationVertex;
	}
	velocity_out.x = Velocity.x * cos(rotationTMP) - Velocity.y * sin(rotationTMP);
	velocity_out.y = Velocity.x * sin(rotationTMP) + Velocity.y * cos(rotationTMP);
	velocity_out.z = Velocity.z;
}

void main()
{
	//if (startTime < deltaTime)
	if ( ifReset == 0 )
	{
		if ( startTime < deltaTime ) // move particles
		{
			startTime_out = startTime - 1;
			randomFloat_out = randomFloat;
			position_out = Position+Velocity;
			velocity_out = Velocity;
			rotationVertex_out = rotationVertex;
			//updateVelocity();
		}
		else // nothing happens
		{
			position_out = vec3(playerPos,randomFloat);
			randomFloat_out = randomFloat;
			startTime_out = startTime - 1;
			updateVelocity();
		}
		if ( startTime_out < 0 )
		{
			ifReset_out = 1;
		}
	}	
	else // moving already started, continue
	{
		if ( startTime > 0 ) // move in the same direction
		{
			startTime_out = startTime - 1;
			randomFloat_out = randomFloat;
			position_out = Position+Velocity;
			velocity_out = Velocity;
			rotationVertex_out = rotationVertex;
			ifReset_out = 0;
		}
		else // reset to players position
		{
			startTime_out = 100;
			randomFloat_out = randomFloat;
			position_out = vec3(playerPos,randomFloat);
			updateVelocity();
			ifReset_out = 1;
		}
	}
}