#include <GLTools.h>
#include <GLShaderManager.h>
#include <GLFrustum.h>
#include <GLBatch.h>
#include <GLMatrixStack.h>
#include <GLGeometryTransform.h>
#include <StopWatch.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif
#include "SDL_net.h"
#ifdef __MINGW32__
#undef main
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_NAME "n3Dacka"

#include <game/Scene.h>
#include <game/Controller.h>
#include <game/Network.h>
#include <game/Fonts.h>
#include <game/InputManager.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int frame = 0;
clock_t endwait = 0,sendTime = 0;

Network *mNetwork;


void specialKeyDown(int key, int x, int y)
{
	InputManager::getInstance().specialKeyPressed(key,x,y);
}

void specialKeyUp(int key, int x, int y)
{
	InputManager::getInstance().specialKeyUnpressed(key,x,y);
}

void asciiKeyDown(unsigned char key, int x, int y)
{
	InputManager::getInstance().asciiKeyPressed(key,x,y);
	if(key == 32 && Lobby::getInstance().getLobbyDone())
	{
		Players::getInstance().getLocalPlayer()->acceptInput = !Players::getInstance().getLocalPlayer()->acceptInput;
		if ( Players::getInstance().getLocalPlayer()->acceptInput == 1 )
		{
			Network::getInstance().setSendingPackets(2);
		}
		else
		{
			Network::getInstance().setSendingPackets(1);
		}
	}
	if(key == 'r')
	{
		Network::getInstance().sendReadySignal();
	}
}

void asciiKeyUp(unsigned char key, int x, int y)
{
	InputManager::getInstance().asciiKeyUnpressed(key,x,y);
}

void mouseDown(int button, int state, int x, int y)
{
	InputManager::getInstance().mousePressed(button, state, x, y);
}

void mouseMove(int x, int y)
{
	InputManager::getInstance().mouseMove(x,y);
}

void changeSize(int nWidth, int nHeight)
{
	Scene::getInstance().changeWindowSize(nWidth, nHeight);
}

void renderScene(void)
{
	if ( clock() < endwait)
	{
		frame++;
	}
	else
	{
//		printf("fps: %d\n",frame);
		char buffer[10];
		sprintf(buffer,"%d",frame);
		glutSetWindowTitle(buffer);
		endwait = clock () + CLOCKS_PER_SEC ;
		frame = 0;
	}
//	Network::getInstance().receivingPackets();
	Scene::getInstance().render();
}

int main(int argc, char* argv[])
{
	srand((time(NULL)));
	gltSetWorkingDirectory(argv[0]);

	// glut - init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow(WINDOW_NAME);

	// keys - input
	glutSpecialFunc(specialKeyDown);
	glutSpecialUpFunc(specialKeyUp);
	glutKeyboardFunc(asciiKeyDown);
	glutKeyboardUpFunc(asciiKeyUp);
	glutMouseFunc(mouseDown);
	glutPassiveMotionFunc(mouseMove);
	glutMotionFunc(mouseMove);
	glutIgnoreKeyRepeat(1);
//	glutSetCursor(GLUT_CURSOR_NONE);
	// draw - callbacks
	glutReshapeFunc(changeSize);
	glutDisplayFunc(renderScene);
	// draw - initialize
	if ( argc == 3 )
	{
		Network::getInstance().setupNet(argv[1],atoi(argv[2]));
	}
	else
	{
		Network::getInstance().setupNet((char*)"localhost",2000);
//		Network::getInstance().setupNet((char*)"s3.trakos.pl",2000);
	}
	Scene::getInstance().setupScene();
	Fonts::getInstance().setupFonts();
	// main loop
	glutMainLoop();
	//finish it
	Scene::getInstance().shutdownRC();
	return 0;
}
