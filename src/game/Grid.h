#pragma once

#include<lib/Singleton.h>
#include<lib/Struct_MoveDefinition.h>
#include<lib/Struct_RenderData.h>
#include<lib/Enum_Direction.h>
#include<game/InputManager.h>
#include<game/Mesh3D.h>
#include<game/Players.h>
#include<time.h>
#include<vector>

class Grid : public Singleton<Grid>
{
	protected:

		int** _grid;
		int** _powerupGrid;
		int width,height;
		int vertNumber;
		int x,y,xLast,yLast,xDiff,yDiff;
		int skipPixels;
		bool update;
		float moveRight,moveUp;
		int playersCount;
		M3DVector3f *_vTail;
		M3DVector3f *_nTail;
		Struct_RenderData *_RenderData;
		clock_t init, final;
		int GRID_SIZE,POWERUP_GRID_SIZE;
		GLuint flatShader;
		GLint flatLocMVP, flatLocColor;
		bool lobbyDone;

		GLuint diffuseLightShader;
		GLint diffuseLightLocMVP,diffuseLightLocMV,diffuseLightLocNM,diffuseLightLocColor,diffuseLightLocLightPos;

		bool LoadTGATexture(const char *szFileName, GLenum minFilter, GLenum magFilter,
				GLenum wrapMode)
		{
			GLbyte *pBits;
			int nWidth, nHeight, nComponents;
			GLenum eFormat;

			// Read the texture bits
			pBits = gltReadTGABits(szFileName, &nWidth, &nHeight, &nComponents,
					&eFormat);
			if (pBits == NULL)
				return false;

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, nWidth, nHeight, 0,
					eFormat, GL_UNSIGNED_BYTE, pBits);

			free(pBits);


			if (minFilter == GL_LINEAR_MIPMAP_LINEAR || minFilter
					== GL_LINEAR_MIPMAP_NEAREST || minFilter
					== GL_NEAREST_MIPMAP_LINEAR || minFilter
					== GL_NEAREST_MIPMAP_NEAREST)
				glGenerateMipmap(GL_TEXTURE_2D);

			return true;
		}

	public:

		Grid() : _grid(NULL)
		{
			_RenderData = getRenderData();
			update = false;
			init=clock();
			playersCount = 0;
			x = y = xLast = yLast = xDiff = yDiff = moveRight = moveUp = 0;
			vertNumber = 0;
			lobbyDone = false;
//			playerConnected(1);
		}
		friend class Singleton<Grid>;

		~Grid()
		{
			for ( auto iterator = _RenderData->tailBatches.begin(); iterator != _RenderData->tailBatches.end(); iterator++)
			{
				delete (*iterator);
			}
			_RenderData->tailBatches.clear();
			if ( _grid != NULL )
			{
				for ( int w=0; w<width; w++)
				{
					delete[] _grid[w];
				}
				delete[] _grid;
			}
			if (_vTail != NULL)
			{
				delete[] _vTail;
			}
			if (_nTail != NULL)
			{
				delete[] _nTail;
			}
		}

		void setLobbyDone(bool in)
		{
			lobbyDone = in;
		}

		int** getPowerupGrid()
		{
			return _powerupGrid;
		}

		void setPixel(int x, int y, int playerId)
		{
			final=clock()-init;

			int temp = final/CLOCKS_PER_SEC;

			if ( (temp != 0) && (temp >= 3) )
			{
				init = clock();
				skipPixels = rand()%4 + 4;
				skipPixels = 0;
//				printf("skipPixels %d\n", skipPixels);
			}

			if ( skipPixels == 0 )
			{
				if ( (x >= 0) && (x < GRID_SIZE) && (y >= 0) && (y < GRID_SIZE) )
				{
					_grid[x][y] = playerId;
					if ( Players::getInstance().getPlayer(playerId) == NULL )
						return;
					Players::getInstance().getPlayer(playerId)->counter++;
//					AddSingleCubeVertices(x,y);
					UpdatePlayersTail(playerId, x, y);
				}
				else
				{
					/*
					setSize(200,200);
					AddSingleCubeVertices(0,0);
					counter = lastDisplay = 0;
					update = false;
					Players::getInstance().reset();
					init = clock();
					_RenderData->cameraFrame.MoveUp(-(float)y/(float)48);
					_RenderData->cameraFrame.MoveRight((float)x/(float)33);
					this->x = this->y = xLast = yLast = xDiff = yDiff = moveRight = moveUp = 0;
					*/
				}
			}
			else
			{
				skipPixels--;
			}
		}

		void DDALine(int x1,int y1,int x2,int y2,int playerId)
		{
		    float dX,dY,iSteps;
		    float xInc,yInc,iCount,x,y;

		    dX = x1 - x2;
		    dY = y1 - y2;

		    if (fabs(dX) > fabs(dY))
		    {
		        iSteps = fabs(dX);
		    }
		    else
		    {
		        iSteps = fabs(dY);
		    }

		    xInc = dX/iSteps;
		    yInc = dY/iSteps;

		    x = x1;
		    y = y1;

		    for (iCount=1; iCount<=iSteps; iCount++)
		    {
		        setPixel(floor(x),floor(y),playerId);
		        x -= xInc;
		        y -= yInc;
		    }
		    return;
		}

		void render()
		{
			glUseProgram(flatShader);
			drawBackground();

			// Draw the light source
			_RenderData->modelViewMatrix.PushMatrix();
				_RenderData->modelViewMatrix.Translatev(_RenderData->vLightPos);
				glUniformMatrix4fv(flatLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform4fv(flatLocColor, 4, _RenderData->vWhite);
				_RenderData->sphereBatch.Draw();
			_RenderData->modelViewMatrix.PopMatrix();

			//frame
			_RenderData->modelViewMatrix.PushMatrix();
				_RenderData->modelViewMatrix.Translate(_RenderData->ScaleX,_RenderData->ScaleY,0.0f);
				_RenderData->modelViewMatrix.Scale(_RenderData->ScaleX,_RenderData->ScaleY,_RenderData->ScaleZ);
				glUniformMatrix4fv(flatLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform4fv(flatLocColor, 4, _RenderData->vBlue);
				_RenderData->frameBatch.Draw();
			_RenderData->modelViewMatrix.PopMatrix();

			for ( std::list<Player*>::iterator iterator = Players::getInstance()._players.begin(); iterator != Players::getInstance()._players.end(); iterator++ )
			{
				if ( (*iterator)->counter > 0 )
				{
					_RenderData->modelViewMatrix.PushMatrix();
//						_RenderData->modelViewMatrix.Scale(0.1f,0.1f,0.1f);
						_RenderData->modelViewMatrix.Scale(_RenderData->ratioX,_RenderData->ratioY,_RenderData->ratioZ);
//						_RenderData->modelViewMatrix.Scale(2*_RenderData->ratioX,2*_RenderData->ratioY,_RenderData->ratioZ);

						glUseProgram(diffuseLightShader);
						glUniformMatrix4fv(diffuseLightLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
						glUniformMatrix4fv(diffuseLightLocMV, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewMatrix());
						glUniformMatrix3fv(diffuseLightLocNM, 1, GL_FALSE, _RenderData->transformPipeline.GetNormalMatrix());
						glUniform4fv(diffuseLightLocColor,1,_RenderData->vRed);
						glUniform3fv(diffuseLightLocLightPos,1,_RenderData->vLightTransformed);

//						_RenderData->tailBatches[(*iterator)->tailBatchId]->Draw();
						_RenderData->tailBatches[(*iterator)->tailBatchId]->drawMesh();
//						Mesh3D cylinder(CYLINDER);
//						cylinder.drawMesh();
					_RenderData->modelViewMatrix.PopMatrix();
				}
			}
			adjustChasingCamera();
			Players::getInstance().render();
		}
		void drawBackground()
		{
			_RenderData->modelViewMatrix.PushMatrix();

//				_RenderData->modelViewMatrix.Scale(0.5f,0.5f,1.0f);
				_RenderData->modelViewMatrix.Translate(-6.0f,-6.0f,-0.26f);

				glUniformMatrix4fv(flatLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform4fv(flatLocColor, 4, _RenderData->vBlack);

				_RenderData->backgroundBatch.Draw();

			_RenderData->modelViewMatrix.PopMatrix();
		}

		void adjustChasingCamera()
		{
			xDiff = x - xLast;
			yDiff = y - yLast;
			xLast = x;
			yLast = y;
			moveRight = (float)-xDiff * 6.0f / GRID_SIZE + moveRight; //25 33
			moveUp = (float)yDiff * 6.0f / GRID_SIZE + moveUp; //33 50
			float substract = 0.007f;

			if ( moveRight < 0 )
			{
				moveRight += substract;
				_RenderData->cameraFrame.MoveRight(-substract);

				if ( moveRight > -substract )
				{
					_RenderData->cameraFrame.MoveRight(moveRight);
					moveRight = 0;
				}
			}
			else if ( moveRight > 0 )
			{
				moveRight -= substract;
				_RenderData->cameraFrame.MoveRight(substract);

				if ( moveRight < substract )
				{
					_RenderData->cameraFrame.MoveRight(moveRight);
					moveRight = 0;
				}
			}

			if ( moveUp < 0 )
			{
				moveUp += substract;
				_RenderData->cameraFrame.MoveUp(-substract);

				if ( moveUp > -substract )
				{
					_RenderData->cameraFrame.MoveUp(moveUp);
					moveUp = 0;
				}
			}
			else if ( moveUp > 0 )
			{
				moveUp -= substract;
				_RenderData->cameraFrame.MoveUp(substract);

				if ( moveUp < substract )
				{
					_RenderData->cameraFrame.MoveUp(moveUp);
					moveUp = 0;
				}
			}
		}

		void CreatePlayersTails(int playerNumber)
		{
			vertNumber = 10000 / playerNumber;
			for ( std::list<Player*>::iterator iterator = Players::getInstance()._players.begin(); iterator != Players::getInstance()._players.end(); iterator++ )
			{
				if ( Players::getInstance().getPlayer((*iterator)->playerId) == NULL )
					break;

				if ( Players::getInstance().getPlayer((*iterator)->playerId)->counter == 0 )
					UpdatePlayersTail((*iterator)->playerId, 0 ,0);
			}
		}

		void CreatePlayerTail(int playerNumber)
		{
			if ( lobbyDone )
			{
				if ( Players::getInstance().getPlayer(playerNumber) == NULL )
					return;

				vertNumber = 10000;
				if ( Players::getInstance().getPlayer(playerNumber)->counter == 0 )
				{
					UpdatePlayersTail(playerNumber, 0, 0);
					printf("created single player tail\n");
				}
			}
		}

		void UpdatePlayersTail(int playerId, int x, int y)
		{
			if ( Players::getInstance().getPlayer(playerId) == NULL )
				return;
			int counter = Players::getInstance().getPlayer(playerId)->counter;
			int lastDisplay = Players::getInstance().getPlayer(playerId)->lastDisplay;
			int tailBatchId = Players::getInstance().getPlayer(playerId)->tailBatchId;
			if (counter > 0)
			{
				for (int i = lastDisplay; i <= counter ; i++)
				{
//					_RenderData->tailBatches[tailBatchId]->SetVertNumber(i*36);
//					_RenderData->tailBatches[tailBatchId]->ChangeVertexData3f(_vTail,36*sizeof(float)*3);
//					_RenderData->tailBatches[tailBatchId]->ChangeNormalDataf(_nTail,36*sizeof(float)*3);
					_RenderData->tailBatches[tailBatchId]->updatePlayerTail(i, x, y);
				}
				lastDisplay = counter;
			}
			else
			{
				_RenderData->tailBatches[tailBatchId]->createPlayerTail();
//				AddSingleCubeNormals();
//				AddSingleCubeVertices(0,0);
//				_RenderData->tailBatches[tailBatchId]->Begin(GL_TRIANGLES, vertNumber,0);
//				_RenderData->tailBatches[tailBatchId]->ChangeVertexData3f(_vTail,36*sizeof(float)*3);
//				_RenderData->tailBatches[tailBatchId]->ChangeNormalDataf(_nTail,36*sizeof(float)*3);
//				_RenderData->tailBatches[tailBatchId]->End();
			}
			Players::getInstance().getPlayer(playerId)->counter = counter;
			Players::getInstance().getPlayer(playerId)->lastDisplay = lastDisplay;
		}

		void playerConnected(int playerId)
		{
//			GLBatch *tailBatchAdd = new GLBatch;
			Mesh3D *tailBatchAdd = new Mesh3D(PLAYER_TAIL);
			_RenderData->tailBatches.push_back(tailBatchAdd);
			playersCount++;
			if ( playersCount > 1 )
			{
				Players::getInstance().createPlayer(0,0,0,playerId);
				if ( Players::getInstance().getPlayer(playerId) == NULL )
					return;
				Players::getInstance().getPlayer(playerId)->tailBatchId = _RenderData->tailBatches.size() - 1;
			}
			else if ( playersCount == 1 )
			{
				Players::getInstance()._localPlayer->reset(0,0,0);
				Players::getInstance()._localPlayer->playerId = playerId;
				if ( Players::getInstance().getPlayer(playerId) == NULL )
					return;
				Players::getInstance().getPlayer(playerId)->tailBatchId = _RenderData->tailBatches.size() - 1;
			}
//			CreatePlayerTail(playerId);
			printf("player id %d tailbatch id %d\n",playerId,Players::getInstance().getPlayer(playerId)->tailBatchId);
			printf("playerscount %d\n",Players::getInstance()._players.size());
		}

		void playerDisconnected(int playerId)
		{
			if ( Players::getInstance().getPlayer(playerId) == NULL )
				return;

			int tailBatchId = Players::getInstance().getPlayer(playerId)->tailBatchId;

			int i = 0;
			auto iterator = _RenderData->tailBatches.begin();
			while ( iterator != _RenderData->tailBatches.end() )
			{
				if ( i == tailBatchId )
				{
					delete (*iterator);
					printf("erased tailbatch with id: %d\n",tailBatchId);
					iterator = _RenderData->tailBatches.erase(iterator);
					break;
				}
				else iterator++;
				i++;
			}
		}


		void AddSingleCubeNormals()
		{
			/*// Top of cube
			for (int i = 0; i < 36; i++)
			{
				if ( i < 6 )
					_nTail[i] = {0.0f, 1.0f, 0.0f};
				else if ( i < 12 )
					_nTail[i] = {0.0f, -1.0f, 0.0f};
				else if ( i < 18 )
					_nTail[i] = {-1.0f, 0.0f, 0.0f};
				else if ( i < 24 )
					_nTail[i] = {1.0f, 0.0f, 0.0f};
				else if ( i < 30 )
					_nTail[i] = {0.0f, 0.0f, 1.0f};
				else if ( i < 36 )
					_nTail[i] = {0.0f, 0.0f, -1.0f};
			}
			*/
			// Top of cube
			for (int i = 0; i < 36; i++)
			{
				if ( i < 6 )
				{
					_nTail[i][0] = 0.0f;
					_nTail[i][1] = 1.0f;
					_nTail[i][2] = 0.0f;
				}
				else if ( i < 12 )
				{
					_nTail[i][0] = 0.0f;
					_nTail[i][1] = -1.0f;
					_nTail[i][2] = 0.0f;
				}
				else if ( i < 18 )
				{
					_nTail[i][0] = -1.0f;
					_nTail[i][1] = 0.0f;
					_nTail[i][2] = 0.0f;
				}
				else if ( i < 24 )
				{
					_nTail[i][0] = 1.0f;
					_nTail[i][1] = 0.0f;
					_nTail[i][2] = 0.0f;
				}
				else if ( i < 30 )
				{
					_nTail[i][0] = 0.0f;
					_nTail[i][1] = 0.0f;
					_nTail[i][2] = 1.0f;
				}
				else if ( i < 36 )
				{
					_nTail[i][0] = 0.0f;
					_nTail[i][1] = 0.0f;
					_nTail[i][2] = -1.0f;
				}
			}
		}

		void AddSingleCubeVertices(float i,float j)
		{
			// Top of cube
//			_vTail[0] = {i+1.0f, j+1.0f, 1.0f};
			_vTail[0][0] = i+1.0f;
			_vTail[0][1] = j+1.0f;
			_vTail[0][2] = 1.0f;
//			_vTail[1] = {i+1.0f, j+1.0f, -1.0f};
			_vTail[1][0] = i+1.0f;
			_vTail[1][1] = j+1.0f;
			_vTail[1][2] = -1.0f;
//			_vTail[2] = {i-1.0f, j+1.0f, -1.0f};
			_vTail[2][0] = i-1.0f;
			_vTail[2][1] = j+1.0f;
			_vTail[2][2] = -1.0f;
//			_vTail[3] = {i+1.0f, j+1.0f, 1.0f};
			_vTail[3][0] = i+1.0f;
			_vTail[3][1] = j+1.0f;
			_vTail[3][2] = 1.0f;
//			_vTail[4] = {i-1.0f, j+1.0f, -1.0f};
			_vTail[4][0] = i-1.0f;
			_vTail[4][1] = j+1.0f;
			_vTail[4][2] = -1.0f;
//			_vTail[5] = {i-1.0f, j+1.0f, 1.0f};
			_vTail[5][0] = i-1.0f;
			_vTail[5][1] = j+1.0f;
			_vTail[5][2] = 1.0f;

			// Bottom of cube
//			_vTail[6] = {i-1.0f, j-1.0f, -1.0f};
			_vTail[6][0] = i-1.0f;
			_vTail[6][1] = j-1.0f;
			_vTail[6][2] = -1.0f;
//			_vTail[7] = {i+1.0f, j-1.0f, -1.0f};
			_vTail[7][0] = i+1.0f;
			_vTail[7][1] = j-1.0f;
			_vTail[7][2] = -1.0f;
//			_vTail[8] = {i+1.0f, j-1.0f, 1.0f};
			_vTail[8][0] = i+1.0f;
			_vTail[8][1] = j-1.0f;
			_vTail[8][2] = 1.0f;
//			_vTail[9] = {i-1.0f, j-1.0f, 1.0f};
			_vTail[9][0] = i-1.0f;
			_vTail[9][1] = j-1.0f;
			_vTail[9][2] = 1.0f;
//			_vTail[10] = {i-1.0f, j-1.0f, -1.0f};
			_vTail[10][0] = i-1.0f;
			_vTail[10][1] = j-1.0f;
			_vTail[10][2] = -1.0f;
//			_vTail[11] = {i+1.0f, j-1.0f, 1.0f};
			_vTail[11][0] = i+1.0f;
			_vTail[11][1] = j-1.0f;
			_vTail[11][2] = 1.0f;

			// Left side of cube
//			_vTail[12] = {i-1.0f, j+1.0f, 1.0f};
			_vTail[12][0] = i-1.0f;
			_vTail[12][1] = j+1.0f;
			_vTail[12][2] = 1.0f;
//			_vTail[13] = {i-1.0f, j+1.0f, -1.0f};
			_vTail[13][0] = i-1.0f;
			_vTail[13][1] = j+1.0f;
			_vTail[13][2] = -1.0f;
//			_vTail[14] = {i-1.0f, j-1.0f, -1.0f};
			_vTail[14][0] = i-1.0f;
			_vTail[14][1] = j-1.0f;
			_vTail[14][2] = -1.0f;
//			_vTail[15] = {i-1.0f, j+1.0f, 1.0f};
			_vTail[15][0] = i-1.0f;
			_vTail[15][1] = j+1.0f;
			_vTail[15][2] = 1.0f;
//			_vTail[16] = {i-1.0f, j-1.0f, -1.0f};
			_vTail[16][0] = i-1.0f;
			_vTail[16][1] = j-1.0f;
			_vTail[16][2] = -1.0f;
//			_vTail[17] = {i-1.0f, j-1.0f, 1.0f};
			_vTail[17][0] = i-1.0f;
			_vTail[17][1] = j-1.0f;
			_vTail[17][2] = 1.0f;

			// Right side of cube
//			_vTail[18] = {i+1.0f, j-1.0f, -1.0f};
			_vTail[18][0] = i+1.0f;
			_vTail[18][1] = j-1.0f;
			_vTail[18][2] = -1.0f;
//			_vTail[19] = {i+1.0f, j+1.0f, -1.0f};
			_vTail[19][0] = i+1.0f;
			_vTail[19][1] = j+1.0f;
			_vTail[19][2] = -1.0f;
//			_vTail[20] = {i+1.0f, j+1.0f, 1.0f};
			_vTail[20][0] = i+1.0f;
			_vTail[20][1] = j+1.0f;
			_vTail[20][2] = 1.0f;
//			_vTail[21] = {i+1.0f, j+1.0f, 1.0f};
			_vTail[21][0] = i+1.0f;
			_vTail[21][1] = j+1.0f;
			_vTail[21][2] = 1.0f;
//			_vTail[22] = {i+1.0f, j-1.0f, 1.0f};
			_vTail[22][0] = i+1.0f;
			_vTail[22][1] = j-1.0f;
			_vTail[22][2] = 1.0f;
//			_vTail[23] = {i+1.0f, j-1.0f, -1.0f};
			_vTail[23][0] = i+1.0f;
			_vTail[23][1] = j-1.0f;
			_vTail[23][2] = -1.0f;

			// Front of cube
//			_vTail[24] = {i+1.0f, j-1.0f, 1.0f};
			_vTail[24][0] = i+1.0f;
			_vTail[24][1] = j-1.0f;
			_vTail[24][2] = 1.0f;
//			_vTail[25] = {i+1.0f, j+1.0f, 1.0f};
			_vTail[25][0] = i+1.0f;
			_vTail[25][1] = j+1.0f;
			_vTail[25][2] = 1.0f;
//			_vTail[26] = {i-1.0f, j+1.0f, 1.0f};
			_vTail[26][0] = i-1.0f;
			_vTail[26][1] = j+1.0f;
			_vTail[26][2] = 1.0f;
//			_vTail[27] = {i-1.0f, j+1.0f, 1.0f};
			_vTail[27][0] = i-1.0f;
			_vTail[27][1] = j+1.0f;
			_vTail[27][2] = 1.0f;
//			_vTail[28] = {i-1.0f, j-1.0f, 1.0f};
			_vTail[28][0] = i-1.0f;
			_vTail[28][1] = j-1.0f;
			_vTail[28][2] = 1.0f;
//			_vTail[29] = {i+1.0f, j-1.0f, 1.0f};
			_vTail[29][0] = i+1.0f;
			_vTail[29][1] = j-1.0f;
			_vTail[29][2] = 1.0f;

			// Back of cube
//			_vTail[30] = {i+1.0f, j-1.0f, -1.0f};
			_vTail[30][0] = i+1.0f;
			_vTail[30][1] = j-1.0f;
			_vTail[30][2] = -1.0f;
//			_vTail[31] = {i-1.0f, j-1.0f, -1.0f};
			_vTail[31][0] = i-1.0f;
			_vTail[31][1] = j-1.0f;
			_vTail[31][2] = -1.0f;
//			_vTail[32] = {i-1.0f, j+1.0f, -1.0f};
			_vTail[32][0] = i-1.0f;
			_vTail[32][1] = j+1.0f;
			_vTail[32][2] = -1.0f;
//			_vTail[33] = {i-1.0f, j+1.0f, -1.0f};
			_vTail[33][0] = i-1.0f;
			_vTail[33][1] = j+1.0f;
			_vTail[33][2] = -1.0f;
//			_vTail[34] = {i+1.0f, j+1.0f, -1.0f};
			_vTail[34][0] = i+1.0f;
			_vTail[34][1] = j+1.0f;
			_vTail[34][2] = -1.0f;
//			_vTail[35] = {i+1.0f, j-1.0f, -1.0f};
			_vTail[35][0] = i+1.0f;
			_vTail[35][1] = j-1.0f;
			_vTail[35][2] = -1.0f;
		}

		void playerMoved(int playerId, Struct_MoveDefinition moveDefinition)
		{
			float x0,x1,y0,y1;

			x0 = moveDefinition.start.x;
			y0 = moveDefinition.start.y;
			x1 = moveDefinition.end.x;
			y1 = moveDefinition.end.y;
			if ( playerId == Players::getInstance().getLocalPlayer()->playerId)
			{
				this->x = x1;
				this->y = y1;
			}

			DDALine(x0,y0,x1,y1,playerId);
		}

		void setSize(int gridSizeW, int gridSizeH, int powerupSizeW, int powerupSizeH)
		{
			GRID_SIZE = gridSizeW;
			if ( _grid != NULL )
			{
				for ( int w=0; w<gridSizeW; w++)
				{
					delete[] _grid[w];
				}
				delete[] _grid;
			}
			_grid = new int*[gridSizeW];
			for ( int w=0; w<gridSizeW; w++)
			{
				_grid[w] = new int[gridSizeH];
				for ( int h=0; h<gridSizeH; h++)
				{
					_grid[w][h] = 0;
				}
			}
			this->width = gridSizeW;
			this->height = gridSizeH;
			POWERUP_GRID_SIZE = gridSizeW;
			if ( _powerupGrid != NULL )
			{
				for ( int w=0; w<powerupSizeW; w++)
				{
					delete[] _powerupGrid[w];
				}
				delete[] _powerupGrid;
			}
			_powerupGrid= new int*[powerupSizeW];
			for ( int w=0; w<powerupSizeW; w++)
			{
				_powerupGrid[w] = new int[powerupSizeH];
				for ( int h=0; h<powerupSizeH; h++)
				{
					_powerupGrid[w][h] = 0;
				}
			}
//			vertNumber = gridSizeW*height*36;
			if ( _vTail == NULL )
			{
				_vTail = new M3DVector3f[36];
			}
			if ( _nTail == NULL )
			{
				_nTail = new M3DVector3f[36];
			}
		}

		void setupGrid()
		{
			flatShader = gltLoadShaderPairWithAttributes("src/shaders/flatDraw.vs", "src/shaders/flatDraw.fs", 0);
			flatLocMVP = glGetUniformLocation(flatShader, "mvpMatrix");
			flatLocColor = glGetUniformLocation(flatShader, "inColor");

			diffuseLightShader = gltLoadShaderPairWithAttributes("src/shaders/diffuseLightDraw.vs", "src/shaders/diffuseLightDraw.fs", 0);
			diffuseLightLocMVP = glGetUniformLocation(diffuseLightShader,"mvpMatrix");
			diffuseLightLocMV = glGetUniformLocation(diffuseLightShader,"mvMatrix");
			diffuseLightLocNM = glGetUniformLocation(diffuseLightShader,"normalMatrix");
			diffuseLightLocColor = glGetUniformLocation(diffuseLightShader,"vColor");
			diffuseLightLocLightPos = glGetUniformLocation(diffuseLightShader,"vLightPosition");
		}
};
