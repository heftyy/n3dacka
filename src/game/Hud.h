#pragma once

#include<lib/Singleton.h>
#include<lib/Struct_RenderData.h>
#include<lib/Struct_PowerupOnTheGrid.h>
#include<GL/glew.h>
#include<time.h>
#include<initializer_list>
#include<vector>

class Hud : public Singleton<Hud>
{
	protected:
		Struct_RenderData *_RenderData;
		clock_t gameStart,readPixelsTimer;
		GLuint	crosshair,powerupsAtlas;		// textures

		GLuint powerupRenderShader;
		GLint powerupLocMVP,powerupLocTexture,powerupLocInColor;

		GLuint crosshairRenderShader;
		GLint crosshairLocMVP,crosshairLocTexture;

		GLfloat winX, winY, winZ;               // Holds Our X, Y and Z Coordinates
		GLdouble posX, posY, posZ;              // Hold The Final Values

		bool zScanned;

		GLBatch *powerupBatches;

		GLint printPosX,printPosY;

		int frameCountToSkip;
		int carriedPowerup;

		char *buffer,*timeGone;

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

		struct buildArray
		{
			float *array;
			buildArray(int number)
			{
				array = new float[number];
			}
			buildArray(std::initializer_list<float> list)
			{
				array = new float[list.size()];
				int i = 0;
				for (std::initializer_list<float>::iterator iterator = list.begin(); iterator != list.end(); iterator++)
				{
					array[i] = *iterator;
					i++;
				}
			}
			~buildArray()
			{
				delete[] array;
			}
			void operator =(std::initializer_list<float> list)
			{
				int i = 0;
				for (std::initializer_list<float>::iterator iterator = list.begin(); iterator != list.end(); iterator++)
				{
					array[i] = *iterator;
					i++;
				}
			}
		};

		std::vector<Struct_PowerupOnTheGrid> powerupsOnTheGrid;

	public:

		Hud()
		{
			_RenderData = getRenderData();

			buffer = new char[100];
			timeGone = new char[10];

			frameCountToSkip = 0;
			carriedPowerup = 0;
			printPosX = printPosY = -1;

			zScanned = false;

			winX = winY = winZ = 0;

			powerupBatches = new GLBatch[8];

			readPixelsTimer = 0;
		}
		friend class Singleton<Hud>;
		~Hud()
		{
			shutdownHud();
		}

		std::vector<Struct_PowerupOnTheGrid>* getPowerupOnTheGrid()
		{
			return &powerupsOnTheGrid;
		}

		void setupHud(void)
		{
			gameStart = clock();

			powerupRenderShader = gltLoadShaderPairWithAttributes("src/shaders/powerupDraw.vs", "src/shaders/powerupDraw.fs", 0);

			powerupLocMVP = glGetUniformLocation(powerupRenderShader, "mvpMatrix");
			powerupLocTexture = glGetUniformLocation(powerupRenderShader, "textureUnit0");
			powerupLocInColor = glGetUniformLocation(powerupRenderShader, "inColor");

			crosshairRenderShader = gltLoadShaderPairWithAttributes("src/shaders/crosshairDraw.vs", "src/shaders/crosshairDraw.fs", 0);

			crosshairLocMVP = glGetUniformLocation(crosshairRenderShader, "mvpMatrix");
			crosshairLocTexture = glGetUniformLocation(crosshairRenderShader, "textureUnit0");

			glGenTextures(1, &crosshair);
			glBindTexture(GL_TEXTURE_2D, crosshair);
			LoadTGATexture("src/textures/crosshair.tga", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE);

			glGenTextures(1, &powerupsAtlas);
			glBindTexture(GL_TEXTURE_2D, powerupsAtlas);
			LoadTGATexture("src/textures/powerupsAtlas.tga", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR, GL_CLAMP_TO_EDGE);

			GLfloat vSquare[4][3] = {
					{ 0.0f, 0.0f, 0.0f },
					{ 1.0f, 0.0f, 0.0f },

					{ 0.0f, 1.0f, 0.0f },
					{ 1.0f, 1.0f, 0.0f }
			};

			GLfloat tSlowField[4][2] = {
					{ 0.0f, 0.876f },
					{ 1.0f, 0.876f },

					{ 0.0f, 1.0f },
					{ 1.0f, 1.0f }
			};

			powerupBatches[0].Begin(GL_TRIANGLE_STRIP, 4,1);
			powerupBatches[0].CopyVertexData3f(vSquare);
			powerupBatches[0].CopyTexCoordData2f(tSlowField,0);
			powerupBatches[0].End();

			GLfloat tFastrField[4][2] = {
					{ 0.0f, 0.751f },
					{ 1.0f, 0.751f },

					{ 0.0f, 0.875f },
					{ 1.0f, 0.875f }
			};

			powerupBatches[1].Begin(GL_TRIANGLE_STRIP, 4,1);
			powerupBatches[1].CopyVertexData3f(vSquare);
			powerupBatches[1].CopyTexCoordData2f(tFastrField,0);
			powerupBatches[1].End();

			GLfloat tOppositeField[4][2] = {
					{ 0.0f, 0.626f },
					{ 1.0f, 0.626f },

					{ 0.0f, 0.75f },
					{ 1.0f, 0.75f }
			};

			powerupBatches[2].Begin(GL_TRIANGLE_STRIP, 4,1);
			powerupBatches[2].CopyVertexData3f(vSquare);
			powerupBatches[2].CopyTexCoordData2f(tOppositeField,0);
			powerupBatches[2].End();
		}

		void shutdownHud(void)
		{
			glDeleteTextures(1,&powerupsAtlas);
			glDeleteTextures(1,&crosshair);
			//TODO
		}

		void renderTime(float xTranslate, float yTranslate)
		{
			frameCountToSkip++;
			if ( frameCountToSkip == 50 )
			{
				frameCountToSkip = 0;
				int stringLength = strlen((char*)"time: ");
				memcpy(buffer,(char*)"time: ",stringLength);
				buffer[stringLength] = '\0';
				int showMS = clock()-gameStart;
				sprintf(timeGone,"%d ms",showMS);
				strcat(buffer,timeGone);
			}
			Fonts::getInstance().drawText(buffer,10,_RenderData->viewportHeight-50,0.5,0.5,_RenderData->vWhite);
		}

		void renderPowerups()
		{
			buildArray color1 = { 0.3f , 1.0f, 0.6f, 1.0f };
			buildArray color2 = { 0.7f , 1.0f, 0.6f, 1.0f };
			buildArray color3 = { 0.7f , 1.0f, 0.6f, 1.0f };
			if ( InputManager::getInstance().mouseLeftPressed == true )
			{
				Struct_Coordinates mouseCoords = InputManager::getInstance().mouseCoords;
				mouseCoords.y = _RenderData->viewportHeight - mouseCoords.y;
				if ( mouseCoords.x >= 0 && mouseCoords.x <= 128 && mouseCoords.y >= 100 && mouseCoords.y <= 228)
				{
					color1 = { 1.0f , 0.0f, 0.0f, 1.0f };
					carriedPowerup = 1;
				}
				else if ( mouseCoords.x >= 0 && mouseCoords.x <= 128 && mouseCoords.y >= 250 && mouseCoords.y <= 378)
				{
					color2 = { 1.0f , 0.0f, 0.0f, 1.0f };
					carriedPowerup = 2;
				}
				else if ( mouseCoords.x >= 0 && mouseCoords.x <= 128 && mouseCoords.y >= 400 && mouseCoords.y <= 528)
				{
					color3 = { 1.0f , 0.0f, 0.0f, 1.0f };
					carriedPowerup = 3;
				}
			}
			else if ( carriedPowerup != 0 ) // put powerup down on the grid
			{
				int x = printPosX, y = printPosY;
				int** powerupGrid = Grid::getInstance().getPowerupGrid();
				if ( x != -1 && y != -1 )
				{
					if ( powerupGrid[x][y] == 0 )
					{
//						Network::getInstance().sendingPowerupPackets(x,y,carriedPowerup);
						Struct_PowerupOnTheGrid newPowerup;
						newPowerup.x = x;
						newPowerup.y = y;
						newPowerup.display = false;
						newPowerup.powerupNumber = carriedPowerup;
						powerupsOnTheGrid.push_back(newPowerup);
					}
				}
				carriedPowerup = 0;
			}

			//LEFT SIDE - AVAILABLE POWERUPS
			renderSinglePowerup(0,100,0,128,128,&powerupBatches[0], color1.array);
			renderSinglePowerup(0,250,0,128,128,&powerupBatches[1], color2.array);
			renderSinglePowerup(0,400,0,128,128,&powerupBatches[2], color3.array);
			//RENDER DRAGGED POWERUP
			if ( carriedPowerup != 0 )
			{
				Struct_Coordinates mouseCoords = InputManager::getInstance().mouseCoords;
				mouseCoords.y = _RenderData->viewportHeight - mouseCoords.y;
				color1 = { 0.3f , 0.3f, 0.3f, 1.0f };
				renderSinglePowerup(mouseCoords.x,mouseCoords.y,0,128,128,&powerupBatches[carriedPowerup-1], color1.array);
			}
		}

		void addPowerupToVector(int x, int y, int id)
		{
			int** powerupGrid = Grid::getInstance().getPowerupGrid();
			Struct_PowerupOnTheGrid newPowerup;
			newPowerup.x = x;
			newPowerup.y = y;
			newPowerup.display = true;
			newPowerup.powerupNumber = id;
			powerupGrid[x][y] = id;
			powerupsOnTheGrid.push_back(newPowerup);
		}

		void renderSinglePowerup(float xTranslate, float yTranslate, float zTranslate, float scaleX,float scaleY, GLBatch* batchToRender, M3DVector4f color)
		{
			_RenderData->modelViewMatrix.PushMatrix();

				_RenderData->modelViewMatrix.Translate(xTranslate,yTranslate,zTranslate);
				_RenderData->modelViewMatrix.Scale(scaleX,scaleY,1.0f);

				glUseProgram(powerupRenderShader);
				glUniformMatrix4fv(powerupLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform1i(powerupLocTexture, 0);
				glUniform4fv(powerupLocInColor, 1, color);
				batchToRender->Draw();

			_RenderData->modelViewMatrix.PopMatrix();
		}

		void renderCrosshair()
		{
			_RenderData->modelViewMatrix.PushMatrix();

				Struct_Coordinates mouseCoords = InputManager::getInstance().mouseCoords;
				_RenderData->modelViewMatrix.Translate(mouseCoords.x-16,_RenderData->viewportHeight-mouseCoords.y-16,0);
				_RenderData->modelViewMatrix.Scale(32.0f,32.0f,1.0f);

				glUseProgram(crosshairRenderShader);
				glUniformMatrix4fv(crosshairLocMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform1i(crosshairLocTexture, 0);
				glBindTexture(GL_TEXTURE_2D, crosshair);
				_RenderData->squareBatch.Draw();

			_RenderData->modelViewMatrix.PopMatrix();
		}

		void drawSelectedField()
		{
			if ( carriedPowerup != 0 )
			{
				Struct_Coordinates mouseCoords = InputManager::getInstance().mouseCoords;

				GLint viewport[4];
				GLdouble modelviewd[16];
				GLdouble projectiond[16];
				GLfloat *modelviewf;
				GLfloat *projectionf;

				modelviewf = (GLfloat*)_RenderData->transformPipeline.GetModelViewMatrix();
				projectionf = (GLfloat*)_RenderData->transformPipeline.GetProjectionMatrix();

				for (int i = 0; i < 16; i++)
				{
					modelviewd[i] = (GLdouble)modelviewf[i];
				}
				for (int i = 0; i < 16; i++)
				{
					projectiond[i] = (GLdouble)projectionf[i];
				}

				winX = (GLdouble)mouseCoords.x;
				winY = _RenderData->viewportHeight - (GLdouble)mouseCoords.y;
				/*
				if ( clock() < readPixelsTimer)
				{
				}
				else
				{
					readPixelsTimer = clock () + CLOCKS_PER_SEC ;
					glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
					printf("winX %d winY %d winZ %f\n",(int)winX,(int)winY,winZ);
				}
				*/
				if ( zScanned == false )
				{
//					glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
					winZ = 0.887813f;
					zScanned = true;
				}
				viewport[0] = viewport[1] = 0;
				viewport[2] = _RenderData->viewportWidth;
				viewport[3] = _RenderData->viewportHeight;
				GLMatrixStack matrix;
				gluUnProject( winX, winY, winZ, modelviewd , projectiond, viewport, &posX, &posY, &posZ);
	//			printf("%f %f %f\n",posX,posY,posZ);
				printPosX = (int)posX;
				printPosY = (int)posY;
				if ( printPosX < 0 )
				{
					printPosX = -1;
					return;
				}
				if ( printPosX > 7 )
				{
					printPosX = -1;
					return;
				}
				if ( printPosY < 0 )
				{
					printPosY = -1;
					return;
				}
				if ( printPosY > 7 )
				{
					printPosY = -1;
					return;
				}

				_RenderData->modelViewMatrix.PushMatrix();

					_RenderData->modelViewMatrix.Translate(printPosX,printPosY,-0.25f);
//					_RenderData->modelViewMatrix.Translate(posX,posY,-0.25f);

					_RenderData->shaderManager.UseStockShader(GLT_SHADER_FLAT, _RenderData->transformPipeline.GetModelViewProjectionMatrix(), _RenderData->vBlue);
					_RenderData->squareBatch.Draw();

				_RenderData->modelViewMatrix.PopMatrix();
			}
		}

		void renderPowerupsOnTheGrid()
		{
			for (auto iterator = powerupsOnTheGrid.begin(); iterator != powerupsOnTheGrid.end(); iterator++)
			{
				if ( iterator->display == true )
				{
					buildArray color1 = { 0.3f , 1.0f, 0.6f, 1.0f };
					renderSinglePowerup(iterator->x, iterator->y,-0.25f, 1, 1, &powerupBatches[iterator->powerupNumber-1], color1.array);
				}
			}
//			printf("size %d\n",powerupsOnTheGrid.size());
		}

		void render()
		{
			glBindTexture(GL_TEXTURE_2D, powerupsAtlas);
			renderPowerupsOnTheGrid();
			drawSelectedField();
//			renderTime(500, 500);

			_RenderData->viewFrustum.SetOrthographic(0, _RenderData->viewportWidth, 0, _RenderData->viewportHeight, -130.0f, 130.0f);
		    _RenderData->projectionMatrix.LoadMatrix(_RenderData->viewFrustum.GetProjectionMatrix());
//		    renderCrosshair();
			renderPowerups();
			_RenderData->viewFrustum.SetPerspective(60.0f, float(_RenderData->viewportWidth) / float(_RenderData->viewportHeight), 1.0f,100.0f);
			_RenderData->projectionMatrix.LoadMatrix(_RenderData->viewFrustum.GetProjectionMatrix());
		}
};
