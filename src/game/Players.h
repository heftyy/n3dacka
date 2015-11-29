#pragma once

#include <lib/Singleton.h>
#include <game/Player.h>
#include <list>
#include <stdexcept>

class Players : public Singleton<Players>
{

	protected:

		int PARTICLE_NUMBER;

		Players() : _players(), _localPlayer(new Player(0,0,0,-1))
		{
			PARTICLE_NUMBER = 2000;
			_players.push_back(_localPlayer);
		}
		friend class Singleton<Players>;
		~Players()
		{
//			delete[] _localPlayer;
			auto iterator = _players.begin();
			while ( iterator != _players.end() )
			{
				delete (*iterator);
				printf("destructor erased player with id: %d\n",(*iterator)->playerId);
				iterator = _players.erase(iterator);
			}
			_players.clear();
		}

	public:

		std::list<Player*> _players;
		Player* _localPlayer;

		void render()
		{
			for ( std::list<Player*>::iterator iterator = _players.begin(); iterator != _players.end(); iterator++ )
			{
				(*iterator)->render();
			}
		}

		void createPlayer(float x,float y,float rotation,int playerId)
		{
			Player* newPlayer;
			newPlayer = new Player(x,y,rotation,playerId);
//			newPlayer->setupParticles(PARTICLE_NUMBER);
			_players.push_back(newPlayer);
		}

		void removePlayer(int playerId)
		{
			auto iterator = _players.begin();
			while ( iterator != _players.end() )
			{
				if ( (*iterator)->playerId == playerId )
				{
					delete (*iterator);
					printf("erased player with id: %d\n",(*iterator)->playerId);
					iterator = _players.erase(iterator);
				}
				else iterator++;
			}
		}

		void setupParticleSystems()
		{
			for ( std::list<Player*>::iterator iterator = _players.begin(); iterator != _players.end(); iterator++ )
			{
				(*iterator)->setupParticles(PARTICLE_NUMBER);
			}
		}

		void reset()
		{
			for ( std::list<Player*>::iterator iterator = _players.begin(); iterator != _players.end(); iterator++ )
			{
				(*iterator)->reset(0, 0, 0);
			}
		}

		Player* getLocalPlayer()
		{
			return _localPlayer;
		}

		Player* getPlayer(int playerId)
		{
			for ( std::list<Player*>::iterator iterator = _players.begin(); iterator != _players.end(); iterator++ )
			{
				if ( (*iterator)->playerId == playerId )
				{
					return *iterator;
				}
			}
			printf("player not found\n");
			return NULL;
//			throw new std::runtime_error("Player not found!");
		}
};
