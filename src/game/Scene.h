#pragma once

#include<lib/Singleton.h>
#include<lib/Struct_RenderData.h>
#include<lib/Thread_Lock.h>

#include<game/Network.h>
#include<game/Grid.h>
#include<game/Lobby.h>
#include<game/Controller.h>
#include<game/Hud.h>

class Scene : public Singleton<Scene>
{
	protected:

		Struct_RenderData *_RenderData;
		int GRID_SIZE;
		int POWERUP_GRID_SIZE;
		GLuint base;
		std::vector<Struct_PlayerToMoveData> *playersToMove;
		std::vector<Struct_PlayersManagment> *playersManagment;
		std::vector<Struct_PowerupOnTheGrid> *powerupsToSend;
		int inputThreadRunning;
		SDL_Thread *thread;
		SDL_mutex *lock;

	public:

		Scene()
		{
			_RenderData = getRenderData();
			GRID_SIZE = 400;
			POWERUP_GRID_SIZE = 8;
			_RenderData->ScaleX = 4.0f;
			_RenderData->ScaleY = 4.0f;
			_RenderData->ScaleZ = 0.25f;
			_RenderData->ratioX = _RenderData->ScaleX / GRID_SIZE;
			_RenderData->ratioY = _RenderData->ScaleY / GRID_SIZE;
			_RenderData->ratioZ = 0.25f;
			_RenderData->vWhite[0] =  1.0f;
			_RenderData->vWhite[1] =  1.0f;
			_RenderData->vWhite[2] =  1.0f;
			_RenderData->vWhite[3] =  1.0f;
			_RenderData->vBlack[0] =  0.0f;
			_RenderData->vBlack[1] =  0.0f;
			_RenderData->vBlack[2] =  0.0f;
			_RenderData->vBlack[3] =  1.0f;
			_RenderData->vLightPos[0] =  0.5f;
			_RenderData->vLightPos[1] =  7.5f;
			_RenderData->vLightPos[2] =  2.0f;
			_RenderData->vLightPos[3] =  1.0f;
			_RenderData->vGreen[0] =  0.0f;
			_RenderData->vGreen[1] =  0.6f;
			_RenderData->vGreen[2] =  0.0f;
			_RenderData->vGreen[3] =  1.0f;
			_RenderData->vRed[0] =  1.0f;
			_RenderData->vRed[1] =  0.0f;
			_RenderData->vRed[2] =  0.0f;
			_RenderData->vRed[3] =  1.0f;
			_RenderData->vBlue[0] = 0.3f;
			_RenderData->vBlue[1] = 0.3f;
			_RenderData->vBlue[2] = 0.95f;
			_RenderData->vBlue[3] = 1.0f;
			lock = getThreadLock();
			/*
			_RenderData->vWhite = { 1.0f, 1.0f, 1.0f, 1.0f };
			_RenderData->vBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
			_RenderData->vLightPos = { 5.0f, 5.0f, 3.0f, 1.0f };
			_RenderData->vGreen = { 0.0f, 0.6f, 0.0f, 1.0f };
			_RenderData->vRed = { 1.0f, 0.0f, 0.0f, 1.0f };
			*/

			Grid::getInstance().setSize(GRID_SIZE,GRID_SIZE,POWERUP_GRID_SIZE,POWERUP_GRID_SIZE);

		}
		friend class Singleton<Scene>;
		~Scene()
		{
			inputThreadRunning = 0;
			SDL_WaitThread(thread, NULL);
		}

		void prepareFrame()
		{
			/*
			SDL_mutexP(lock);
			int pConn = 0, pDisc = 0;
			pConn = Network::getInstance().getPlayerConnected();
			pDisc = Network::getInstance().getPlayerDisconnected();
			if (pConn)
			{
				Grid::getInstance().playerConnected(pConn);
				Network::getInstance().resetPlayerConnected();
			}
			if (pDisc)
			{
				Grid::getInstance().playerDisconnected(pDisc);
				Players::getInstance().removePlayer(pDisc);
				Network::getInstance().resetPlayerDisconnected();
			}
			playersToMove = Network::getInstance().getPlayersToMove();
			if ( playersToMove->size() > 0)
			{
				for ( std::vector<Struct_PlayerToMoveData>::iterator iterator = playersToMove->begin(); iterator != playersToMove->end(); iterator++ )
				{
					Grid::getInstance().playerMoved(iterator->playerId, iterator->moveDefinition);
					delete(&iterator->moveDefinition);
				}
				Network::getInstance().resetPlayersToMove();
			}
			SDL_mutexV(lock);
			*/
			SDL_mutexP(lock);
			playersManagment = Network::getInstance().getPlayersManagment();
			if ( playersManagment->size() > 0)
			{
				auto iterator = playersManagment->begin();
				while ( iterator != playersManagment->end() )
				{
					if ( iterator->playerConnected == true )
					{
						Grid::getInstance().playerConnected(iterator->playerId);
						if ( Players::getInstance().getPlayer(iterator->playerId) == NULL )
							continue;
						Players::getInstance().getPlayer(iterator->playerId)->spectator = iterator->spectator;
						if ( iterator->localPlayer )
						{
							Players::getInstance()._localPlayer = Players::getInstance().getPlayer(iterator->playerId);
						}
						iterator = playersManagment->erase(iterator);
					}
					else if ( iterator->playerDisconnected == true )
					{
						Grid::getInstance().playerDisconnected(iterator->playerId);
						Players::getInstance().removePlayer(iterator->playerId);
						iterator = playersManagment->erase(iterator);
					}
					else iterator++;
				}
			}

			playersToMove = Network::getInstance().getPlayersToMove();
			if ( playersToMove->size() > 0)
			{
				for ( auto iterator = playersToMove->begin(); iterator != playersToMove->end(); iterator++ )
				{
					Grid::getInstance().playerMoved(iterator->playerId, iterator->moveDefinition);
//					delete(&iterator->moveDefinition);
				}
				Network::getInstance().resetPlayersToMove();
			}
			SDL_mutexV(lock);
		}

		void render()
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glClearColor(0.05f, 0.05f, 0.05f, 1.0f );

			_RenderData->modelViewMatrix.PushMatrix();
			M3DMatrix44f mCamera;
			_RenderData->cameraFrame.GetCameraMatrix(mCamera);
			_RenderData->modelViewMatrix.MultMatrix(mCamera);
			m3dTransformVector4(_RenderData->vLightTransformed, _RenderData->vLightPos, mCamera);

			prepareFrame();
			if ( Lobby::getInstance().renderLobby() == false )
			{
				Grid::getInstance().render();
				Hud::getInstance().render();
			}

			_RenderData->modelViewMatrix.PopMatrix();
			//Sleep(10);
			// Do the buffer Swap
			glutSwapBuffers();

			// Do it again
			glutPostRedisplay();
		}

		void changeWindowSize(int width, int height)
		{
//			printf("scene.changeWindowSize\n");
			_RenderData->viewportWidth = width;
			_RenderData->viewportHeight = height;
			glViewport(0, 0, width, height);
			_RenderData->viewFrustum.SetPerspective(60.0f, float(width) / float(height), 1.0f,
					100.0f);
			_RenderData->projectionMatrix.LoadMatrix(_RenderData->viewFrustum.GetProjectionMatrix());
			_RenderData->modelViewMatrix.LoadIdentity();
			_RenderData->transformPipeline.SetMatrixStacks(_RenderData->modelViewMatrix, _RenderData->projectionMatrix);
		}

		static int runInputThread(void *unused)
		{
			Scene::getInstance().inpuThread();
			return 0;
		}

		void inpuThread()
		{
			while (inputThreadRunning == 1)
			{
				SDL_mutexP(lock);
				int localPlayerId = Players::getInstance().getLocalPlayer()->playerId;
				int direction = Controller::getInstance().control(localPlayerId,Grid::getInstance().getPowerupGrid());
//				Struct_Coordinates playerCoords = Controller::getInstance().control(localPlayerId,Grid::getInstance().getPowerupGrid());
				Network::getInstance().sendingPackets(localPlayerId,direction);
				powerupsToSend = Hud::getInstance().getPowerupOnTheGrid();
				if ( powerupsToSend->size() > 0)
				{
					auto iterator = powerupsToSend->begin();
					while ( iterator != powerupsToSend->end() )
					{
						if ( iterator->display == false && powerupsToSend->size() > 0 )
						{
							Network::getInstance().sendingPowerupPackets(iterator->x,iterator->y,iterator->powerupNumber);
							iterator = powerupsToSend->erase(iterator);
						}
						else iterator++;
					}
				}
				SDL_mutexV(lock);
				SDL_Delay(20);
			}
			printf("Input thread quitting\n");
		}

		void setupScene(void)
		{
			glewExperimental = GL_TRUE;
			// Make sure OpenGL entry points are set
	        GLenum err = glewInit();
	        if (GLEW_OK != err)
	        {
	                /* Problem: glewInit failed, something is seriously wrong. */
	                printf("glewInit failed, aborting.\n");
	                exit (1);
	        }

			inputThreadRunning = 1;
			thread = SDL_CreateThread(runInputThread, NULL);

			if ( thread == NULL ) {
				fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
				inputThreadRunning = 0;
				return;
			}

			// Initialze Shader Manager
			_RenderData->shaderManager.InitializeStockShaders();

			// INITIALIZE MY THINGS

			Hud::getInstance().setupHud();
			Grid::getInstance().setupGrid();

			// SETUP CAMERA
			_RenderData->cameraFrame.MoveForward(-8.0f);
//			_RenderData->cameraFrame.MoveUp(_RenderData->ScaleY);
			_RenderData->cameraFrame.MoveUp(1.0f);
			_RenderData->cameraFrame.MoveRight(-1.0f);
//			_RenderData->cameraFrame.MoveRight(-_RenderData->ScaleX);

			glEnable(GL_DEPTH_TEST);
//			glEnable(GL_CULL_FACE);

			glClearColor(0.7f, 0.7f, 0.7f, 1.0f );

			GLfloat vSquare[4][3] = {
					{ 0.0f, 0.0f, 0.0f },
					{ 1.0f, 0.0f, 0.0f },

					{ 0.0f, 1.0f, 0.0f },
					{ 1.0f, 1.0f, 0.0f }
			};

			GLfloat tSquare[4][2] = {
					{ 0.0f, 0.0f },
					{ 1.0f, 0.0f },

					{ 0.0f, 1.0f },
					{ 1.0f, 1.0f }
			};

			_RenderData->squareBatch.Begin(GL_TRIANGLE_STRIP, 4,1);
			_RenderData->squareBatch.CopyVertexData3f(vSquare);
			_RenderData->squareBatch.CopyTexCoordData2f(tSquare,0);
			_RenderData->squareBatch.End();

			GLfloat vBackground[4][3] = {
					{ 0.0f, 0.0f, 0.0f },
					{ 16.0f, 0.0f, 0.0f },

					{ 0.0f, 16.0f, 0.0f },
					{ 16.0f, 16.0f, 0.0f }
			};

			_RenderData->backgroundBatch.Begin(GL_TRIANGLE_STRIP, 4,1);
			_RenderData->backgroundBatch.CopyVertexData3f(vBackground);
			_RenderData->backgroundBatch.End();

		    GLfloat vLines[52][3] = {
				{ -1.0f, -1.0f, 1.0f }, //front
				{ -1.0f, 1.0f, 1.0f },

				{ 1.0f, -1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },

				{ -1.0f, -1.0f, 1.0f },
				{ 1.0f, -1.0f, 1.0f },

				{ -1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, 1.0f },

				{ -1.0f, -1.0f, -1.0f },//back
				{ -1.0f, 1.0f, -1.0f },

				{ 1.0f, -1.0f, -1.0f },
				{ 1.0f, 1.0f, -1.0f },

				{ -1.0f, -1.0f, -1.0f },
				{ 1.0f, -1.0f, -1.0f },

				{ -1.0f, 1.0f, -1.0f },
				{ 1.0f, 1.0f, -1.0f },

				{ -1.0f, -1.0f, 1.0f },//left
				{ -1.0f, -1.0f, -1.0f },

				{ -1.0f, 1.0f, 1.0f },
				{ -1.0f, 1.0f, -1.0f },

				{ 1.0f, -1.0f, 1.0f },//right
				{ 1.0f, -1.0f, -1.0f },

				{ 1.0f, 1.0f, 1.0f },
				{ 1.0f, 1.0f, -1.0f },

				{ -0.75f, 1.0f, -1.0f }, //vertical back lines
				{ -0.75f, -1.0f, -1.0f },

				{-0.5f, 1.0f, -1.0f },
				{ -0.5f, -1.0f, -1.0f },

				{ -0.25f, 1.0f, -1.0f },
				{ -0.25f, -1.0f, -1.0f },

				{ 0.0, 1.0f, -1.0f },
				{ 0.0, -1.0f, -1.0f },

				{ 0.25f, 1.0f, -1.0f },
				{ 0.25f, -1.0f, -1.0f },

				{ 0.5f, 1.0f, -1.0f },
				{ 0.5f, -1.0f, -1.0f },

				{ 0.75f, 1.0f, -1.0f },
				{ 0.75f, -1.0f, -1.0f },

				{ -1.0f, -0.75f, -1.0f }, //horizontal back lines
				{ 1.0f, -0.75f, -1.0f },

				{ -1.0f, -0.5f, -1.0f },
				{ 1.0f, -0.5f, -1.0f },

				{ -1.0f, -0.25f, -1.0f },
				{ 1.0f, -0.25f, -1.0f },

				{ -1.0f, 0.0f, -1.0f },
				{ 1.0f, 0.0f, -1.0f },

				{ -1.0f, 0.25f, -1.0f },
				{ 1.0f, 0.25f, -1.0f },

				{ -1.0f, 0.5f, -1.0f },
				{ 1.0f, 0.5f, -1.0f },

				{ -1.0f, 0.75f, -1.0f },
				{ 1.0f, 0.75f, -1.0f },
		    };

			_RenderData->frameBatch.Begin(GL_LINES, 52);
			_RenderData->frameBatch.CopyVertexData3f(vLines);
			_RenderData->frameBatch.End();

			gltMakeSphere(_RenderData->sphereBatch, 0.1f, 26, 13);
		}

		void shutdownRC(void)
		{
			//TODO
		}
};

