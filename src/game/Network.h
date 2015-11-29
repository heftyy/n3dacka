#pragma once

#include<lib/Singleton.h>
#include<lib/Struct_PacketData.h>
#include<lib/Struct_PlayerToMoveData.h>
#include<lib/Thread_Lock.h>
#include<lib/Struct_PlayersManagment.h>
#include<SDL_net.h>
#include<game/Players.h>
#include<game/Grid.h>
#include<game/Lobby.h>
#include<game/Hud.h>
#include<vector>


class Network : public Singleton<Network>
{
	protected:

		Struct_PacketData *_PacketData;
		UDPsocket socket;
		UDPpacket *receivePacket;
		UDPpacket *sendPacket;
		IPaddress srvadd;
		SDLNet_SocketSet set;
		SDL_Thread *thread;
		char *serverAddress;
		int port;
		bool helloSent;
		int allowSendingPackets;
		float x,y;
		int receivingThreadRunning;
		int playerConnected, playerDisconnected;
		std::vector<Struct_PlayerToMoveData> playersToMove;
		std::vector<Struct_PlayersManagment> playersManagment;
		SDL_mutex *lock;

	public:

		Network()
		{
			_PacketData = getPacketData();
			helloSent = false;
			allowSendingPackets = 0;
			playerConnected = playerDisconnected = 0;
			lock = getThreadLock();
		}
		friend class Singleton<Network>;
		~Network()
		{
			receivingThreadRunning = 0;
			SDL_WaitThread(thread, NULL);
			sendGoodbye();
//			SDLNet_FreePacket(receivePacket);
//			SDLNet_FreePacket(sendPacket);
			SDLNet_Quit();
		}

		std::vector<Struct_PlayerToMoveData>* getPlayersToMove()
		{
			return &playersToMove;
		}

		void addPlayerToMove(Struct_PlayerToMoveData playerToMove)
		{
			playersToMove.push_back(playerToMove);
		}

		void resetPlayersToMove()
		{
			for ( std::vector<Struct_PlayerToMoveData>::iterator iterator = playersToMove.begin(); iterator != playersToMove.end(); iterator++ )
			{
//				delete &iterator;
			}
			playersToMove.clear();
		}
		
		int getPlayerConnected()
		{
			return playerConnected;
		}

		int getPlayerDisconnected()
		{
			return playerDisconnected;
		}

		void resetPlayerConnected()
		{
			playerConnected = 0;
		}

		void resetPlayerDisconnected()
		{
			playerDisconnected = 0;
		}

		std::vector<Struct_PlayersManagment>* getPlayersManagment()
		{
			return &playersManagment;
		}

		void setupNet(char* srvAddr, int srvport)
		{
			serverAddress = srvAddr;
			port = srvport;
			/* Initialize SDL_net */
			if (SDLNet_Init() < 0)
			{
				fprintf(stderr, "SDLNet_Init: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}

			if (!(socket = SDLNet_UDP_Open(0)))
			{
				fprintf(stderr, "SDLNet_UDP_Open: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}

			/* Resolve server name  */
			if (SDLNet_ResolveHost(&srvadd, serverAddress, port) == -1)
			{
				fprintf(stderr, "SDLNet_ResolveHost(%s %d): %s\n", serverAddress, port, SDLNet_GetError());
				exit(EXIT_FAILURE);
			}

			/* Allocate memory for the packet */
			if (!(receivePacket = SDLNet_AllocPacket(512)))
			{
				fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}

			if (!(sendPacket = SDLNet_AllocPacket(512)))
			{
				fprintf(stderr, "SDLNet_AllocPacket: %s\n", SDLNet_GetError());
				exit(EXIT_FAILURE);
			}

			if(!(set=SDLNet_AllocSocketSet(16)))
			{
				printf("SDLNet_AllocSocketSet: %s\n", SDLNet_GetError());
				exit(1); //most of the time this is a major error, but do what you want.
			}

			if(SDLNet_UDP_AddSocket(set,socket) == -1)
			{
				printf("SDLNet_AddSocket: %s\n", SDLNet_GetError());
				// perhaps you need to restart the set and make it bigger...
			}

			receivingThreadRunning = 1;
			thread = SDL_CreateThread(runRecevingThread, NULL);

			if ( thread == NULL ) {
				fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
				receivingThreadRunning = 0;
				return;
			}
		}

		void setSendingPackets(int in)
		{
			allowSendingPackets = in;
		}

		void sendHello()
		{
			_PacketData->playerId = -1;
			_PacketData->header = 0;
			_PacketData->rotation = 0;

			printf("Sending hello, port: %d\n",srvadd.port);

			sendPacket->address.host = srvadd.host;
			sendPacket->address.port = srvadd.port;
			sendPacket->data = (Uint8*)_PacketData;
			sendPacket->len = sizeof(Struct_PacketData);

			SDLNet_UDP_Send(socket, -1, sendPacket);
		}

		void sendGoodbye()
		{
			_PacketData->playerId = Players::getInstance().getLocalPlayer()->playerId;
			_PacketData->header = 4;
			_PacketData->rotation = 0;

			printf("Sending goodbye\n");

			sendPacket->address.host = srvadd.host;
			sendPacket->address.port = srvadd.port;
			sendPacket->data = (Uint8*)_PacketData;
			sendPacket->len = sizeof(Struct_PacketData);

			SDLNet_UDP_Send(socket, -1, sendPacket);
		}

		void sendReadySignal()
		{
			_PacketData->playerId = Players::getInstance().getLocalPlayer()->playerId;
			_PacketData->header = 6;
			_PacketData->readyInLobby = Players::getInstance().getLocalPlayer()->switchReady();

			printf("Sending ready signal\n");

			sendPacket->address.host = srvadd.host;
			sendPacket->address.port = srvadd.port;
			sendPacket->data = (Uint8*)_PacketData;
			sendPacket->len = sizeof(Struct_PacketData);

			SDLNet_UDP_Send(socket, -1, sendPacket);
		}

		static int runRecevingThread(void *unused)
		{
			Network::getInstance().receivingPackets();
			return 0;
		}

		void receivingPackets()
		{
			while (receivingThreadRunning == 1)
			{
				int numready=SDLNet_CheckSockets(set, 1000);
				if(numready==-1)
				{
					printf("SDLNet_CheckSockets: %s\n", SDLNet_GetError());
					//most of the time this is a system error, where perror might help you.
					perror("SDLNet_CheckSockets");
				}
				else if(numready) {
					if ( receivingThreadRunning == 0 ) break;
//					clock_t startwait = clock();
					SDL_mutexP(lock);
					if(SDLNet_SocketReady(socket)) {
						if(SDLNet_UDP_Recv(socket,receivePacket)) {
							// process the packet.
							_PacketData = (Struct_PacketData*)receivePacket->data;
							if ( _PacketData->header == 0 ) //HANDSHAKE
							{
								printf("Local player ID received: %d\n", _PacketData->playerId);
								if ( _PacketData->spectator == true )
								{
									printf("local player is a spectator\n");
								}
								Struct_PlayersManagment newplayer;
								newplayer.playerConnected = true;
								newplayer.playerDisconnected = false;
								newplayer.playerId = _PacketData->playerId;
								newplayer.localPlayer = true;
								newplayer.spectator = _PacketData->spectator;
								playersManagment.push_back(newplayer);
							}

							else if ( _PacketData->header == 1 ) //RECEIVING DATA PACKETS
							{
								x = _PacketData->x;
								y = _PacketData->y;
//								printf("data from the server %f %f\n",x,y);
								if ( Players::getInstance().getPlayer(_PacketData->playerId) == NULL )
									continue;
								Struct_PlayerToMoveData playerToMoveData;
								playerToMoveData.moveDefinition = Players::getInstance().getPlayer(_PacketData->playerId)->moveByFrame(x,y,_PacketData->rotation);
								playerToMoveData.playerId = _PacketData->playerId;
								playersToMove.push_back(playerToMoveData);
							}

							else if ( _PacketData->header == 3 ) //NEW PLAYER JOINED
							{
								printf("New player ID received: %d\n", _PacketData->playerId);
								if ( _PacketData->spectator == true )
								{
									printf("new player is a spectator\n");
								}
								Struct_PlayersManagment newplayer;
								newplayer.playerConnected = true;
								newplayer.playerDisconnected = false;
								newplayer.playerId = _PacketData->playerId;
								newplayer.localPlayer = false;
								newplayer.spectator = _PacketData->spectator;
								playersManagment.push_back(newplayer);
							}
							else if (_PacketData->header == 5 ) //PLAYER DISCONNECTED
							{
								printf("Player disconnected ID received: %d\n", _PacketData->playerId);
								Struct_PlayersManagment newplayer;
								newplayer.playerConnected = false;
								newplayer.playerDisconnected = true;
								newplayer.playerId = _PacketData->playerId;
								newplayer.localPlayer = false;
								newplayer.spectator = _PacketData->spectator;
								playersManagment.push_back(newplayer);
							}
							else if (_PacketData->header == 6 ) //OTHER PLAYER IS READY
							{
								printf("Player ID: %d is ready/unready \n", _PacketData->playerId);
								Players::getInstance().getPlayer(_PacketData->playerId)->switchReady();
							}
							else if (_PacketData->header == 7 ) //ALL PLAYERS READY
							{
								printf("All players ready\n");
								Lobby::getInstance().setLobbyDone();
							}
							else if (_PacketData->header == 8 ) //POWERUP INFO
							{
								int pX = _PacketData->x, pY = _PacketData->y;
								int id = _PacketData->playerId;
								printf("Received powerup info x %d y %d id %d\n",pX,pY,id);
								Hud::getInstance().addPowerupToVector(pX,pY,id);
							}
						}
					}
					SDL_mutexV(lock);
					/*
					clock_t endwait = clock();
					int waited = endwait - startwait;
					printf("took %d ms to handle packet\n",waited);
					*/
					/*
					SDL_mutexP(lock);
					if(SDLNet_SocketReady(socket)) {
						if(SDLNet_UDP_Recv(socket,receivePacket)) {
							// process the packet.
							_PacketData = (Struct_PacketData*)receivePacket->data;
			//				printf("packet received\n");
							if ( _PacketData->header == 0 ) //HANDSHAKE
							{
								printf("Local player ID received: %d\n", _PacketData->playerId);
//								playerConnected = _PacketData->playerId;
								Grid::getInstance().playerConnected(_PacketData->playerId);
								Players::getInstance()._localPlayer = Players::getInstance().getPlayer(_PacketData->playerId);
								printf("local player id %d\n",Players::getInstance()._localPlayer->playerId);
							}

							else if ( _PacketData->header == 1 ) //RECEIVING DATA PACKETS
							{
								x = _PacketData->x;
								y = _PacketData->y;
								Struct_PlayerToMoveData *playerToMoveData = new Struct_PlayerToMoveData;
								playerToMoveData->moveDefinition = Players::getInstance().getPlayer(_PacketData->playerId)->moveByFrame(x,y,_PacketData->rotation);
								playerToMoveData->playerId = _PacketData->playerId;
								playersToMove.push_back(*playerToMoveData);
							}

							else if ( _PacketData->header == 3 ) //NEW PLAYER JOINED
							{
								printf("New player ID received: %d\n", _PacketData->playerId);
								playerConnected = _PacketData->playerId;
//								Grid::getInstance().playerConnected(_PacketData->playerId);
							}
							else if (_PacketData->header == 5 ) //PLAYER DISCONNECTED
							{
								printf("Player disconnected ID received: %d\n", _PacketData->playerId);
								playerDisconnected = _PacketData->playerId;
//								Grid::getInstance().playerDisconnected(_PacketData->playerId);
//								Players::getInstance().removePlayer(_PacketData->playerId);
							}
							else if (_PacketData->header == 6 ) //OTHER PLAYER IS READY
							{
								printf("Player ID: %d is ready/unready \n", _PacketData->playerId);
								Players::getInstance().getPlayer(_PacketData->playerId)->switchReady();
							}
							else if (_PacketData->header == 7 ) //ALL PLAYERS READY
							{
								printf("All players ready\n");
								Lobby::getInstance().setLobbyDone();
							}
						}
					}
					SDL_mutexV(lock);
					*/
				}
			}
			printf("Receving thread quitting\n");
		}

		/*
		void sendingPackets(float pX, float pY, int playerId, int direction)
		{
			if ( (playerId != -1) && (allowSendingPackets > 0) && ( pX != -1 ) && ( pY != -1 )) //SEND DATA PACKETS
			*/
		void sendingPackets(int playerId, int direction)
		{
			if ( (playerId != -1) && (allowSendingPackets > 0) ) //SEND DATA PACKETS
			{
				if ( Players::getInstance().getLocalPlayer()->positionUpdated == false ) return;

				if ( allowSendingPackets == 1 )
				{
					allowSendingPackets = 0;
				}
//				printf("data sent to the server %f %f\n",pX,pY);
				_PacketData->header = 1;
				_PacketData->playerId = playerId;
//				_PacketData->rotation = Players::getInstance().getLocalPlayer()->rotation;
//				_PacketData->acceptInput = Players::getInstance().getLocalPlayer()->acceptInput;
				_PacketData->direction = direction;
//				x = _PacketData->x = pX;
//				y = _PacketData->y = pY;
//				printf("x %f y %f \n",x,y);

				sendPacket->address.host = srvadd.host;
				sendPacket->address.port = srvadd.port;
				sendPacket->data = (Uint8*)_PacketData;
				sendPacket->len = sizeof(Struct_PacketData);

				SDLNet_UDP_Send(socket, -1, sendPacket);
			}
			else if ( !helloSent )
			{
				sendHello();
				helloSent = true;
			}
		}

		void sendingPowerupPackets(int pX, int pY, int powerupId)
		{
			if ( (powerupId != -1) && ( pX != -1 ) && ( pY != -1 )) //SEND DATA PACKETS
			{
				if ( allowSendingPackets == 1 )
				{
					allowSendingPackets = 0;
				}
				printf("Sending powerup data to server x %d y %d id %d\n",pX,pY,powerupId);
				_PacketData->header = 8;
				_PacketData->playerId = powerupId;
				_PacketData->x = pX;
				_PacketData->y = pY;

				sendPacket->address.host = srvadd.host;
				sendPacket->address.port = srvadd.port;
				sendPacket->data = (Uint8*)_PacketData;
				sendPacket->len = sizeof(Struct_PacketData);

				SDLNet_UDP_Send(socket, -1, sendPacket);
			}
		}

		void shutdownNet(void)
		{
			//TODO
		}
};
