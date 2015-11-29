#pragma once

#include <GL/glut.h>
#include <lib/Singleton.h>
#include <lib/Enum_Direction.h>
#include <lib/Struct_PlayerToMoveData.h>
#include <game/InputManager.h>
#include <game/Players.h>
#include <game/Player.h>
#include <game/Grid.h>

class Controller : public Singleton<Controller>
{
	public:

		int control(int playerId, int** powerupGrid)
		{
//			Struct_Coordinates playerCoords;
//			playerCoords.x = -1;
//			playerCoords.y = -1;
			InputManager& input = InputManager::getInstance();
			bool left = input.isAsciiKeyPressed('a');
			bool right = input.isAsciiKeyPressed('d');
//			bool left = input.isSpecialKeyPressed(GLUT_KEY_LEFT);
//			bool right = input.isSpecialKeyPressed(GLUT_KEY_RIGHT);
			Enum_Direction direction = NONE;
			if ( left && right )
			{}
			else if(left)
			{
				direction = LEFT;
			}
			else if(right)
			{
				direction = RIGHT;
			}
			Player* localPlayer = Players::getInstance().getLocalPlayer();
			if ( (localPlayer->playerId == -1) || (!localPlayer->acceptInput) )
			{}
			else
			{
				localPlayer->positionUpdated = true;
//				playerCoords = localPlayer->moveByFrameLocally(direction,powerupGrid).end;
			}
//			return playerCoords
			return direction;
		}

	protected:

		Controller()
		{

		}
		friend class Singleton<Controller>;
};
