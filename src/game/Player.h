#pragma once

#include <lib/Enum_Direction.h>
#include <lib/Struct_MoveDefinition.h>
#include <math3d.h>
#include <game/ParticleSystem.h>

class Player
{
	protected:

		ParticleSystem *_ParticleSystem;

	public:

		float x;
		float y;
		float rotation;
		int playerId;
		float velX;
		float velY;
		float velocity;
		int particleNumber;
		Enum_Direction direction;
		int counter, lastDisplay;
		bool acceptInput;
		int tailBatchId;
		bool positionUpdated;
		bool readyInLobby;
		bool spectator;

		Player() : x(0), y(0), rotation(0), playerId(0)
		{
		};

		Player(float x, float y, float rotation, int playerId) : x(x), y(y), rotation(rotation), playerId(playerId)
		{
//			velocity = 0.25f;
			velocity = 1.0f;
			counter = lastDisplay = 0;
			acceptInput = false;
			_ParticleSystem = NULL;
			positionUpdated = false;
			readyInLobby = false;
			spectator = false;
		};

		~Player()
		{
			delete _ParticleSystem;
		}

		void setupParticles(int n)
		{
			if ( _ParticleSystem == NULL )
			{
				particleNumber = n;
				_ParticleSystem = new ParticleSystem(n);
				_ParticleSystem->setupParticleSystem();
			}
		}

		bool switchReady()
		{
			readyInLobby = !readyInLobby;
			return readyInLobby;
		}

		void reset(float x, float y, float rotation)
		{
			this->x = x;
			this->y = y;
			this->rotation = rotation;
			counter = lastDisplay = 0;
		}

		Struct_MoveDefinition moveByFrame(float xn, float yn, float rot)
		{
			Struct_MoveDefinition coordinates;

			float x0 = this->x;
			float y0 = this->y;

			this->rotation = rot;

			velX = xn - x;
			velY = yn - y;

			this->x += velX;
			this->y += velY;

			coordinates.start.x = x0;
			coordinates.start.y = y0;
			coordinates.end.x = this->x;
			coordinates.end.y = this->y;

			return coordinates;
		}

		Struct_MoveDefinition moveByFrameLocally(Enum_Direction direction,int** powerupGrid)
		{
			Struct_MoveDefinition coordinates;
			this->direction = direction;

			float x0 = this->x;
			float y0 = this->y;

			int helperX = x0,finalX = 0;
			int helperY = y0,finalY = 0;
			for (finalX = 0; helperX > 50; finalX++, helperX-=50);
			for (finalY = 0; helperY > 50; finalY++, helperY-=50);

			if ( powerupGrid[finalX][finalY] == 1 )
			{
				velocity = 0.5f;
			}
			else if ( powerupGrid[finalX][finalY] == 2 )
			{
				velocity = 2.0f;
			}
			else if ( powerupGrid[finalX][finalY] == 3 )
			{
				if ( direction == LEFT ) direction = RIGHT;
				else if ( direction == RIGHT ) direction = LEFT;
//				this->rotation = -this->rotation;
			}
			else velocity = 1.0f;

			if ( direction == LEFT )
			{
				this->rotation += 7*velocity;
			}
			else if ( direction == RIGHT )
			{
				this->rotation -= 7*velocity;
			}

			velX = velocity * cos(m3dDegToRad(this->rotation));
			velY = velocity * sin(m3dDegToRad(this->rotation));

			this->x += velX;
			this->y += velY;

			coordinates.start.x = x0;
			coordinates.start.y = y0;
			coordinates.end.x = x0 + velX;
			coordinates.end.y = y0 + velY;

			if ( coordinates.end.x < 0 ) coordinates.end.x = 0;
			if ( coordinates.end.x > 399 ) coordinates.end.x = 399;
			if ( coordinates.end.y < 0 ) coordinates.end.y = 0;
			if ( coordinates.end.y > 399 ) coordinates.end.y = 399;

//			Struct_RenderData *asd = getRenderData();

//			asd->cameraFrame.SetForwardVector(0,x,-1);
//			asd->cameraFrame.RotateLocalX(-1/(float)100);

			return coordinates;
		}

		void render()
		{
			if ( playerId != -1 && !spectator )
			{
				_ParticleSystem->RenderParticleSystem(x-velX,y-velY,m3dDegToRad(rotation+135));
			}
		}
};
