#pragma once

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <stdexcept>
#include <GL/gl.h>
#include <lib/Singleton.h>
#include <lib/Struct_RenderData.h>


class Fonts : public Singleton<Fonts>
{
	protected:

		FT_Library  library;   /* handle to library     */
		FT_Face     face;      /* handle to face object */

		bool done;
		float textureWidth;
		float textureHeight;

		GLuint	renderShader;
		GLint	locMVP,locTexture,locColor;

		GLuint textTexture;
		GLBatch textBatch;

		int pen_x, pen_y, n;
		int error;

		Struct_RenderData *_RenderData;

		struct character_info {
		  float ax; // advance.x
		  float ay; // advance.y

		  float bw; // bitmap.width;
		  float bh; // bitmap.rows;

		  float bl; // bitmap_left;
		  float bt; // bitmap_top;

		  float tx; // x offset of glyph in texture coordinates
		}character[128];

		int BUFFERED_STRINGS_NUMBER;
		int BUFFERED_STRINGS_ITERATOR;

		struct string_info {
			char* string;
			GLBatch* stringBatch;
		};

		string_info *bufferedStrings;

	public:

		Fonts()
		{
			_RenderData = getRenderData();
			BUFFERED_STRINGS_NUMBER = 20;
			BUFFERED_STRINGS_ITERATOR = 0;
			bufferedStrings = new string_info[BUFFERED_STRINGS_NUMBER];
			for (int i = 0; i < BUFFERED_STRINGS_NUMBER ; i++)
			{
				bufferedStrings[i].string = NULL;
				bufferedStrings[i].stringBatch = NULL;
			}
		}
		friend class Singleton<Fonts>;
		~Fonts()
		{
			for (int i = 0; i < BUFFERED_STRINGS_NUMBER ; i++)
			{
				if ( bufferedStrings[i].string != NULL )
				{
					delete[] bufferedStrings[i].string;
				}
				if ( bufferedStrings[i].stringBatch != NULL )
				{
					delete bufferedStrings[i].stringBatch;
				}
			}
			delete[] bufferedStrings;
		}

		int next_p2 ( int a )
		{
			int rval=1;
			while(rval<a) rval<<=1;
			return rval;
		}

		void setupFonts()
		{

			renderShader = gltLoadShaderPairWithAttributes("src/shaders/drawText.vs", "src/shaders/drawText.fs", 0);

			locMVP = glGetUniformLocation(renderShader, "mvpMatrix");
			locTexture = glGetUniformLocation(renderShader, "texture");
			locColor = glGetUniformLocation(renderShader, "uniformColor");

			error = FT_Init_FreeType( &library );
			if ( error )
			{
				printf("... an error occurred during library initialization ...\n");
			}
			error = FT_New_Face( library,"fonts/arial.ttf",0,&face );
			if ( error == FT_Err_Unknown_File_Format )
			{
				printf("... the font file could be opened and read, but it appears\n");
				printf("... that its font format is unsupported\n");
			}
			else if ( error )
			{
				printf("... another error code means that the font file could not\n");
				printf("... be opened or read, or simply that it is broken...\n");
			}
			error = FT_Set_Char_Size(
					face,    /* handle to face object           */
					0,       /* char_width in 1/64th of points  */
					32*64,   /* char_height in 1/64th of points */
					96,     /* horizontal device resolution    */
					96 );   /* vertical device resolution      */
			if ( error )
			{
				printf("set char size error\n");
			}

			FT_GlyphSlot g = face->glyph;
			int w = 0;
			int h = 0;

			for(int i = 31; i < 128; i++)
			{
				if(FT_Load_Char(face, i, FT_LOAD_RENDER))
				{
					fprintf(stderr, "Loading character %c failed!\n", i);
					continue;
				}
				w += g->bitmap.width;
				h = std::max(h, g->bitmap.rows);
			}
			w = next_p2(w);
			h = next_p2(h);
			textureWidth = w;
			textureHeight = h;

			glGenTextures(1, &textTexture);
			glBindTexture(GL_TEXTURE_2D, textTexture);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, 0);

			GLubyte *bytes = new GLubyte[w*h];
			memset(bytes,0,w*h);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_ALPHA, GL_UNSIGNED_BYTE, bytes);

			int x = 0;

			for(int i = 32; i < 128; i++)
			{
				if(FT_Load_Char(face, i, FT_LOAD_RENDER))
				continue;

				character[i].ax = g->advance.x >> 6;
				character[i].ay = g->advance.y >> 6;

				character[i].bw = g->bitmap.width;
				character[i].bh = g->bitmap.rows;

				character[i].bl = g->bitmap_left;
				character[i].bt = g->bitmap_top;

				character[i].tx = (float)x / w;

				glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE, g->bitmap.buffer);

				x += g->bitmap.width;
			}
			glGenerateMipmap(GL_TEXTURE_2D);

			FT_Done_Face(face);

			//Ditto for the library.
			FT_Done_FreeType(library);
		}

		void renderBatch(GLBatch *batch,int xTranslate,int yTranslate,M3DVector4f vColor)
		{
			_RenderData->viewFrustum.SetOrthographic(0, _RenderData->viewportWidth, 0, _RenderData->viewportHeight, -130.0f, 130.0f);
//		    _RenderData->viewFrustum.SetOrthographic(-200.0f, 200.0f, -200.0f, 200.0f, -130.0f, 130.0f);

		    _RenderData->projectionMatrix.LoadMatrix(_RenderData->viewFrustum.GetProjectionMatrix());

			_RenderData->modelViewMatrix.PushMatrix();

				_RenderData->modelViewMatrix.Translate(xTranslate,yTranslate,0);

				glUseProgram(renderShader);
				glUniformMatrix4fv(locMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
				glUniform1i(locTexture, 0);
				glUniform4fv(locColor,4,vColor);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				glBindTexture(GL_TEXTURE_2D, textTexture);
				batch->Draw();
				glDisable(GL_BLEND);

			_RenderData->modelViewMatrix.PopMatrix();
			_RenderData->viewFrustum.SetPerspective(60.0f, float(_RenderData->viewportWidth) / float(_RenderData->viewportHeight), 1.0f,100.0f);
			_RenderData->projectionMatrix.LoadMatrix(_RenderData->viewFrustum.GetProjectionMatrix());
		}

		bool lookupStringInBuffer(const char *text,int xTranslate,int yTranslate,M3DVector4f vColor)
		{
			bool stringFound = false;
			for (int i = 0 ; i < BUFFERED_STRINGS_NUMBER; i++ )
			{
				if ( bufferedStrings[i].string == NULL ) break;
				if ( strcmp(bufferedStrings[i].string,text) == 0 )
				{
					stringFound = true;
					renderBatch(bufferedStrings[i].stringBatch,xTranslate,yTranslate,vColor);
//					printf("found in buffer \n");
					break;
				}
			}
			if ( stringFound == false )
			{
				return false;
			}
			else
				return true;
		}

		void drawText(const char *text, float xTranslate, float yTranslate, float sx, float sy,M3DVector4f vColor)
		{
			if ( lookupStringInBuffer(text,xTranslate,yTranslate,vColor) == true )
				return;

			int len = 6 * strlen(text);
			struct point {
				GLfloat x;
				GLfloat y;
				GLfloat z;
				GLfloat s;
				GLfloat t;
			} coords[len];

			int n = 0;
			unsigned char c;
			float x = 0,y = 0;

			for(unsigned char i = 0; i < strlen(text); i++)
			{
				c = text[i];

				float x2 =  x + character[c].bl * sx;
				float y2 = -y - character[c].bt * sy;
				float w = character[c].bw * sx;
				float h = character[c].bh * sy;

				/* Advance the cursor to the start of the next character */
				x += character[c].ax * sx;
				y += character[c].ay * sy;

				/* Skip glyphs that have no pixels */
				if ( c == ' ' )
				{}
				else if(!w || !h) continue;

				coords[n++] = (point){x2,     -y2-h,0, character[c].tx,                		   			  character[c].bh / textureHeight};
				coords[n++] = (point){x2 + w, -y2-h,0, character[c].tx + character[c].bw / textureWidth,  character[c].bh / textureHeight};
				coords[n++] = (point){x2 + w, -y2  ,0, character[c].tx + character[c].bw / textureWidth,  0};
				coords[n++] = (point){x2    , -y2  ,0, character[c].tx,						   			  0};
				coords[n++] = (point){x2,     -y2-h,0, character[c].tx,                		   	          character[c].bh / textureHeight};
				coords[n++] = (point){x2 + w, -y2  ,0, character[c].tx + character[c].bw / textureWidth,  0};
			}

			M3DVector3f vVerts[len];

			for (int i = 0 ; i < len; i++)
			{
				vVerts[i][0] = coords[i].x;
				vVerts[i][1] = coords[i].y;
				vVerts[i][2] = coords[i].z;
			}

			M3DVector2f vTexCoords[len];

			for (int i = 0 ; i < len; i++)
			{
				vTexCoords[i][0] = coords[i].s;
				vTexCoords[i][1] = coords[i].t;
			}

			if ( bufferedStrings[BUFFERED_STRINGS_ITERATOR].string != NULL )
			{
				delete[] bufferedStrings[BUFFERED_STRINGS_ITERATOR].string;
			}
			if ( bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch != NULL )
			{
				delete bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch;
			}

			bufferedStrings[BUFFERED_STRINGS_ITERATOR].string = new char[strlen(text)+1];
			memcpy(bufferedStrings[BUFFERED_STRINGS_ITERATOR].string,text,strlen(text)+1);
			bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch = new GLBatch();

			bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch->Begin(GL_TRIANGLES,len,1);
			bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch->CopyVertexData3f(vVerts);
			bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch->CopyTexCoordData2f(vTexCoords,0);
			bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch->End();

			renderBatch(bufferedStrings[BUFFERED_STRINGS_ITERATOR].stringBatch,xTranslate,yTranslate,vColor);

			BUFFERED_STRINGS_ITERATOR++;
			if (BUFFERED_STRINGS_ITERATOR == BUFFERED_STRINGS_NUMBER )
				BUFFERED_STRINGS_ITERATOR = 0;
		}
};
