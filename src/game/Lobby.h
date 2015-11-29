#pragma once

#include<lib/Singleton.h>
#include<game/Fonts.h>
#include<game/Players.h>
//#include<game/Grid.h>
#include<lib/Struct_RenderData.h>

class Lobby : public Singleton<Lobby>
{

	protected:

		bool lobbyDone,playersCreated;
		char buffer[20];
		M3DVector4f unreadyColor,readyColor;
		Struct_RenderData *_RenderData;
		int stringLength;

	public:

		Lobby()
		{
			lobbyDone = playersCreated = false;
			stringLength = strlen((char*)"Player id ");
			memcpy(&buffer,(char*)"Player id ",stringLength);
			unreadyColor[0] = 0.8f;
			unreadyColor[1] = 0.0f;
			unreadyColor[2] = 0.0f;
			unreadyColor[3] = 1.0f;
			readyColor[0] = 0.0f;
			readyColor[1] = 0.8f;
			readyColor[2] = 0.0f;
			readyColor[3] = 1.0f;
			_RenderData = getRenderData();
		}
		friend class Singleton<Lobby>;
		~Lobby()
		{
			delete[] buffer;
		}

		void setLobbyDone()
		{
			lobbyDone = true;
		}

		bool getLobbyDone()
		{
			return lobbyDone;
		}

		bool renderLobby()
		{
			if ( playersCreated == true ) return false;

			if ( lobbyDone == true )
			{
				Grid::getInstance().CreatePlayersTails(Players::getInstance()._players.size());
				Players::getInstance().setupParticleSystems();
				Grid::getInstance().setLobbyDone(true);
				playersCreated = true;
				return false;
			}

			char playerIdChar[4];
			memcpy(&buffer,(char*)"Player id ",stringLength);
			int i = 0;
			for ( std::list<Player*>::iterator iterator = Players::getInstance()._players.begin(); iterator != Players::getInstance()._players.end(); iterator++ )
			{
				buffer[stringLength] = '\0';
				sprintf(playerIdChar,"%d",(*iterator)->playerId);
				strcat(buffer,playerIdChar);
				if ( (*iterator)->readyInLobby )
					Fonts::getInstance().drawText(buffer,300,300-i*50,1,1,readyColor);
				else Fonts::getInstance().drawText(buffer,300,300-i*50,1,1,unreadyColor);
				i++;
			}

			return true;
		}

		void setupLobby(void)
		{

		}

		void shutdownLobby(void)
		{
			//TODO
		}
};
