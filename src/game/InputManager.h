#pragma once

#include<lib/Singleton.h>
#include<GL/glut.h>
#include<list>

class InputManager : public Singleton<InputManager>
{
	protected:

		std::list<unsigned char> _asciiKeys;
		std::list<int> _specialKeys;
		SDL_mutex *lock;

		InputManager() : _asciiKeys(), _specialKeys()
		{
			mouseCoords.x = mouseCoords.y = 0;
			lock = getThreadLock();
		}
		friend class Singleton<InputManager>;

	public:

		bool mouseLeftPressed, mouseRightPressed;

		Struct_Coordinates mouseCoords;

		void mousePressed(int button, int state, int x, int y)
		{
			SDL_mutexP(lock);
			if ( button == GLUT_LEFT_BUTTON )
			{
				if ( state == GLUT_DOWN )
				{
					mouseLeftPressed = true;
					mouseCoords.x = x;
					mouseCoords.y = y;
					printf("mouse down x: %d, y: %d\n",x,y);
				}
				else if ( state == GLUT_UP )
				{
					mouseLeftPressed = false;
				}
			}
			else if ( button == GLUT_RIGHT_BUTTON )
			if ( state == GLUT_DOWN )
			{
				mouseRightPressed = true;
				mouseCoords.x = x;
				mouseCoords.y = y;
			}
			SDL_mutexV(lock);
		}

		void mouseMove(int x, int y)
		{
			SDL_mutexP(lock);
			mouseCoords.x = x;
			mouseCoords.y = y;
			SDL_mutexV(lock);
//			printf("mouse move x: %d, y: %d\n",x,y);
		}

		void specialKeyPressed(int key, int x, int y)
		{
			if ( !isSpecialKeyPressed(key) )
			{
				_specialKeys.push_back(key);
			}
		}

		void asciiKeyPressed(unsigned char key, int x, int y)
		{
			if ( !isAsciiKeyPressed(key) )
			{
				_asciiKeys.push_back(key);
			}
		}

		void specialKeyUnpressed(int key, int x, int y)
		{
			_specialKeys.remove(key);
		}

		void asciiKeyUnpressed(unsigned char key, int x, int y)
		{
			_asciiKeys.remove(key);
		}

		bool isAsciiKeyPressed(unsigned char key)
		{
			for ( std::list<unsigned char>::iterator iterator = _asciiKeys.begin(); iterator != _asciiKeys.end(); iterator++ )
			{
				if ( *iterator == key )
				{
					return true;
				}
			}
			return false;
		}

		bool isSpecialKeyPressed(int key)
		{

			for ( std::list<int>::iterator iterator = _specialKeys.begin(); iterator != _specialKeys.end(); iterator++ )
			{
				if ( *iterator == key )
				{
					return true;
				}
			}
			return false;
		}

		const std::list<unsigned char> getAllAsciiKeys()
		{
			return _asciiKeys;
		}

		const std::list<int> getAllSpecialKeys()
		{
			return _specialKeys;
		}

};
