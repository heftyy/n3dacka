#pragma once

#include<vector>
#include<SDL_net.h>
#include<lib/Enum_Direction.h>

struct playersPositoin
{
	int x,y;
};

struct Struct_PacketData
{
	int header;
	int playerId;
	Uint32 ipAddress;
	Uint16 port;
	int direction;
	int alreadySend;
	int acceptInput;
	float x,y;
	float rotation;
	bool readyInLobby;
	bool spectator;
};

Struct_PacketData *getPacketData(){
	static Struct_PacketData data;
	return &data;
}

