#pragma once

SDL_mutex *getThreadLock(){
	static SDL_mutex *lock = SDL_CreateMutex();
	return lock;
}
