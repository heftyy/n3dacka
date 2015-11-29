#pragma once

#include <time.h>
#include <math.h>
#include <lib/Struct_RenderData.h>
#include <string.h>
#include <GL/glew.h>
#include <stdlib.h>


struct Particle
{
	M3DVector3f Pos, Vel;
	float randFloat;
	float startTime;
	float rotationVertex;
	int ifReset;
};

class ParticleSystem
{
	protected:

		int PARTICLE_NUMBER;

		GLfloat deltaTime;
		GLfloat rotation;

		GLuint updateShader,renderShader;
		GLuint update_vs;
		GLint locMVP,locMV,locNM,locTexture,locTime,locRot,locPlayerPos;

		GLfloat playerPos[2];

		GLuint	particleTexture;		// The particle texture texture object

		unsigned int m_currVB;
		unsigned int m_currTFB;
		GLuint m_particleBuffer[2];
		GLuint m_transformFeedback[2];

		Struct_RenderData *_RenderData;

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

		ParticleSystem(int particleNumber)
		{
			PARTICLE_NUMBER = particleNumber;
			deltaTime = rotation = 0;
			m_currVB = 0;
			m_currTFB = 1;
			_RenderData = getRenderData();
		}
		~ParticleSystem()
		{
			printf("particle system deleted\n");
			glDeleteTransformFeedbacks(2,m_transformFeedback);
			glDeleteBuffers(2,m_particleBuffer);
			glDeleteTextures(1,&particleTexture);
			glDeleteProgram(updateShader);
			glDeleteProgram(renderShader);
		}

		void setupParticleSystem()
		{
			Particle *Particles;

			Particles = new Particle[PARTICLE_NUMBER];

			memset(Particles,0,sizeof(Particle)*PARTICLE_NUMBER);

			for (int i = 0 ; i < PARTICLE_NUMBER; i++)
			{
				float scale = 0.25f;
				float x = (rand()/(float)RAND_MAX)*scale;
				float y = (rand()/(float)RAND_MAX)*scale;
				//Particles[i].Vel = {x,y,0};
				Particles[i].Vel[0] = x;
				Particles[i].Vel[1] = y;
				Particles[i].Vel[2] = 0;
				Particles[i].randFloat = -(rand()/(float)RAND_MAX)/2 - (1.0f - _RenderData->ratioZ);
				Particles[i].startTime = rand()%200;
//				Particles[i].startTime = 0;
				Particles[i].rotationVertex = 0;
				Particles[i].ifReset = 0;
			}

			glGenTransformFeedbacks(2, m_transformFeedback);
			glGenBuffers(2, m_particleBuffer);

			for (unsigned int i = 0; i < 2 ; i++)
			{
				glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[i]);
				glBufferData(GL_ARRAY_BUFFER, sizeof(Particle)*PARTICLE_NUMBER, Particles, GL_STREAM_DRAW);
				glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[i]);
				glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_particleBuffer[i]);
				glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
			}

			delete [] Particles;

		//	updateShader = gltLoadShaderPair("update.vs","update.gs");

			updateShader = glCreateProgram();

			// Load and compile the vertex shader
			update_vs = glCreateShader(GL_VERTEX_SHADER);
			gltLoadShaderFile("src/shaders/particleSystemUpdate.vs", update_vs);
			glCompileShader(update_vs);
			glAttachShader(updateShader, update_vs);

			// Setup transform feedback varyings
			static const GLchar * tf_outputs[] = { "position_out", "velocity_out","randomFloat_out","startTime_out", "rotationVertex_out", "ifReset_out" };
			glTransformFeedbackVaryings(updateShader, 6, tf_outputs, GL_INTERLEAVED_ATTRIBS);

			// Link. Notice no fragment shader.
			glLinkProgram(updateShader);

			glUseProgram(updateShader);

			locTime = glGetUniformLocation(updateShader, "deltaTime");
			locRot = glGetUniformLocation(updateShader,"rotation");
			locPlayerPos = glGetUniformLocation(updateShader, "playerPos");

			renderShader = gltLoadShaderPairWithAttributes("src/shaders/particleSystemDraw.vs", "src/shaders/particleSystemDraw.fs", 0);

			locMVP = glGetUniformLocation(renderShader, "mvpMatrix");
			locTexture = glGetUniformLocation(renderShader, "texture");

			glGenTextures(1, &particleTexture);
			glBindTexture(GL_TEXTURE_2D, particleTexture);
			LoadTGATexture("src/textures/texture.tga", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,GL_CLAMP_TO_EDGE);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, particleTexture);
		}

		void RenderParticleSystem(float x, float y, float tetha)
		{
			if ( deltaTime < 200 )
				deltaTime++;

			playerPos[0] = 2*x;
			playerPos[1] = 2*y;
			rotation = tetha;

			glBindTexture(GL_TEXTURE_2D, particleTexture);

//			glEnable(GL_TEXTURE_2D);
//			glEnable(GL_POINT_SPRITE_NV);
//			glTexEnvf(GL_POINT_SPRITE_NV,GL_COORD_REPLACE_NV,GL_TRUE);

//		    _RenderData->modelViewMatrix.PushMatrix(_RenderData->cameraFrame);
		    _RenderData->modelViewMatrix.PushMatrix();

		    _RenderData->modelViewMatrix.Translate(0,0,1.0f);
		    _RenderData->modelViewMatrix.Scale(_RenderData->ratioX,_RenderData->ratioY,1.0f);

			glUseProgram(updateShader);
			glUniform1f(locTime,deltaTime);
			glUniform1f(locRot,rotation);
			glUniform2fv(locPlayerPos,1,playerPos);

		    glEnable(GL_RASTERIZER_DISCARD);

		    glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currVB]);
		    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_transformFeedback[m_currTFB]);

		    glEnableVertexAttribArray(0);
		    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);         // position
		    glEnableVertexAttribArray(1);
		    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)12);        // velocity
		    glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24);        // randomFloat
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);        // startTime
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)32);        // rotationVertex
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 1, GL_INT, GL_FALSE, sizeof(Particle), (const GLvoid*)36);        // ifReset
		    glBeginTransformFeedback(GL_POINTS);

		    glDrawArrays(GL_POINTS,0,PARTICLE_NUMBER);

		    glEndTransformFeedback();
//
		    glDisableVertexAttribArray(0);
		    glDisableVertexAttribArray(1);
		    glDisableVertexAttribArray(2);
		    glDisableVertexAttribArray(3);
		    glDisableVertexAttribArray(4);
		    glDisableVertexAttribArray(5);
//
		    glDisable(GL_RASTERIZER_DISCARD);

		    glEnable(GL_POINT_SPRITE);
		    glEnable(GL_PROGRAM_POINT_SIZE);
//		    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

			glUseProgram(renderShader);
			glUniformMatrix4fv(locMVP, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewProjectionMatrix());
//			glUniformMatrix4fv(locMV, 1, GL_FALSE, _RenderData->transformPipeline.GetModelViewMatrix());
//			glUniformMatrix3fv(locNM, 1, GL_FALSE, _RenderData->transformPipeline.GetNormalMatrix());
			glUniform1i(locTexture, 0);

			glBindBuffer(GL_ARRAY_BUFFER, m_particleBuffer[m_currTFB]);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), 0);  // position
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)24);  // randomFloat
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)28);        // startTime

			glDrawArrays(GL_POINTS,0,PARTICLE_NUMBER);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(2);
			glDisableVertexAttribArray(3);

		    glDisable(GL_PROGRAM_POINT_SIZE);
		    glDisable(GL_POINT_SPRITE);

		    _RenderData->modelViewMatrix.PopMatrix();

		    m_currVB = m_currTFB;
		    m_currTFB = (m_currTFB + 1) & 0x1;
		}
};
