#pragma once

#include<meshes/Cylinder.h>
#include<meshes/Cube.h>
#include<lib/Enum_Direction.h>
#include<GL/glew.h>

struct meshStruct
{
	GLBatch batch;
	GLuint iboNumber;
	int drawIndicesCount;
	int lastIndicesAdded;
	int drawnElementCounter;
	float oldX,oldY;
};

class Mesh3D
{
	protected:

		Enum_Mesh3D meshType;

//		GLBatch myMesh3D;
		std::vector<meshStruct*> MeshesVector;
		int vertexCount, indexCount;
		int sizeofIndicesToAdd;

		int elementNumber;
		int CURRENT_CHUNK_NUMBER;

		unsigned int *_indices;

		M3DVector3f *_vertices,*_normals,*_colors,*_textures;

		void load3DMesh(GLBatch &batch,int type, int vertexCount, M3DVector3f* verts, M3DVector3f* normals, float *colors, float *texture,
				GLuint &_indices, unsigned int *indices, int indicesCount)
		{
			glGenBuffers(1, &_indices);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _indices);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indicesCount, indices, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			int textureUnits = 0;
			if ( texture != NULL ) textureUnits = 1;
			batch.Begin(type,vertexCount,textureUnits);
			if ( verts != NULL) batch.CopyVertexData3f(verts);
			if ( normals != NULL) batch.CopyNormalDataf(normals);
			if ( colors != NULL) batch.CopyColorData4f(colors);
			if ( texture != NULL) batch.CopyTexCoordData2f(texture,0);
			batch.End();
		}

	public:

		Mesh3D(Enum_Mesh3D Mesh3D)
		{
			this->meshType = Mesh3D;

			vertexCount = indexCount = 0;
			sizeofIndicesToAdd = 0;
			elementNumber = 0;
			CURRENT_CHUNK_NUMBER = 0;

			_indices = NULL;

			_vertices = _normals = _colors = _textures = NULL;

			if ( meshType == CYLINDER )
			{
				meshStruct *mesh = new meshStruct();
				MeshesVector.push_back(mesh);
				vertexCount = cylinder_vertex_count;
				indexCount = cylinder_faces_count * 3;
				_vertices = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_vertices[i][0] = cylinder_vertices[i].x;
					_vertices[i][1] = cylinder_vertices[i].y;
					_vertices[i][2] = cylinder_vertices[i].z;
				}
				_normals = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_normals[i][0] = cylinder_vertices[i].nx;
					_normals[i][1] = cylinder_vertices[i].ny;
					_normals[i][2] = cylinder_vertices[i].nz;
				}
				load3DMesh(MeshesVector[0]->batch,GL_TRIANGLES,vertexCount,_vertices,_normals,NULL,NULL,MeshesVector[0]->iboNumber,cylinder_indices,indexCount);
				MeshesVector[0]->drawIndicesCount = indexCount;
				printf("%d\n",MeshesVector[0]->iboNumber);
			}

			if ( meshType == PLAYER_TAIL_CYLINDER )
			{
				vertexCount = cylinder_vertex_count;
				indexCount = cylinder_faces_count * 3;
				_indices = new unsigned int[indexCount];
				for (int i = 0; i < indexCount; i++)
				{
					_indices[i] = cylinder_indices[i];
				}
				sizeofIndicesToAdd = sizeof(int) * indexCount;
				_vertices = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_vertices[i][0] = cylinder_vertices[i].x;
					_vertices[i][1] = cylinder_vertices[i].y;
					_vertices[i][2] = cylinder_vertices[i].z;
				}
				_normals = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_normals[i][0] = cylinder_vertices[i].nx;
					_normals[i][1] = cylinder_vertices[i].ny;
					_normals[i][2] = cylinder_vertices[i].nz;
				}
			}

			if ( meshType == PLAYER_TAIL )
			{
				vertexCount = cube_vertex_count;
				indexCount = cube_faces_count * 3;
				_indices = new unsigned int[indexCount];
				for (int i = 0; i < indexCount; i++)
				{
					_indices[i] = cube_indices[i];
				}
				sizeofIndicesToAdd = sizeof(int) * indexCount;
				_vertices = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_vertices[i][0] = cube_vertices[i].x;
					_vertices[i][1] = cube_vertices[i].y;
					_vertices[i][2] = cube_vertices[i].z;
				}
				_normals = new M3DVector3f[vertexCount];
				for (int i = 0; i < vertexCount; i++)
				{
					_normals[i][0] = cube_vertices[i].nx;
					_normals[i][1] = cube_vertices[i].ny;
					_normals[i][2] = cube_vertices[i].nz;
				}
			}
//			elementNumber = 20000 / (vertexCount*3*sizeof(GLfloat));
			elementNumber = 1048576 / (vertexCount*3*sizeof(GLfloat));
		}

		void resetPlayerTailData()
		{
			if ( meshType == PLAYER_TAIL_CYLINDER )
			{
				for (int i = 0; i < indexCount; i++)
				{
					_indices[i] = cylinder_indices[i];
				}
				for (int i = 0; i < vertexCount; i++)
				{
					_vertices[i][0] = cylinder_vertices[i].x;
					_vertices[i][1] = cylinder_vertices[i].y;
					_vertices[i][2] = cylinder_vertices[i].z;
				}
				for (int i = 0; i < vertexCount; i++)
				{
					_normals[i][0] = cylinder_vertices[i].nx;
					_normals[i][1] = cylinder_vertices[i].ny;
					_normals[i][2] = cylinder_vertices[i].nz;
				}
			}

			if ( meshType == PLAYER_TAIL )
			{
				for (int i = 0; i < indexCount; i++)
				{
					_indices[i] = cube_indices[i];
				}
				for (int i = 0; i < vertexCount; i++)
				{
					_vertices[i][0] = cube_vertices[i].x;
					_vertices[i][1] = cube_vertices[i].y;
					_vertices[i][2] = cube_vertices[i].z;
				}
				for (int i = 0; i < vertexCount; i++)
				{
					_normals[i][0] = cube_vertices[i].nx;
					_normals[i][1] = cube_vertices[i].ny;
					_normals[i][2] = cube_vertices[i].nz;
				}
			}
		}

		void prepareVerticesBuffer(float newX, float newY)
		{
			float diffX = newX - MeshesVector[CURRENT_CHUNK_NUMBER]->oldX;
			float diffY = newY - MeshesVector[CURRENT_CHUNK_NUMBER]->oldY;
			if ( diffX == 0 && diffY == 0 ) return;
			for (int i = 0; i < vertexCount; i++)
			{
				_vertices[i][0] += diffX;
				_vertices[i][1] += diffY;
//				_vertices[i][2] = cylinder_vertices[i].z;
//				_vertices[i][2] = cube_vertices[i].z;
			}
			MeshesVector[CURRENT_CHUNK_NUMBER]->oldX = newX;
			MeshesVector[CURRENT_CHUNK_NUMBER]->oldY = newY;

	        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshesVector[CURRENT_CHUNK_NUMBER]->iboNumber);
	        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,MeshesVector[CURRENT_CHUNK_NUMBER]->lastIndicesAdded, sizeofIndicesToAdd, _indices);
	        for (int i = 0; i < indexCount; i++)
			{
				_indices[i] += vertexCount;
			}
	        MeshesVector[CURRENT_CHUNK_NUMBER]->lastIndicesAdded += sizeofIndicesToAdd;
	        MeshesVector[CURRENT_CHUNK_NUMBER]->drawIndicesCount += indexCount;
	        MeshesVector[CURRENT_CHUNK_NUMBER]->drawnElementCounter++;
	        if ( MeshesVector[CURRENT_CHUNK_NUMBER]->drawnElementCounter >= elementNumber )
	        {
	        	printf("create new chunk ");
	        	createNewChunk();
	        }
		}

		void createNewChunk()
		{
			CURRENT_CHUNK_NUMBER++;
			resetPlayerTailData();
			createPlayerTail();
		}

		void updatePlayerTail(int current, float x, float y)
		{
//			if ( CURRENT_CHUNK_NUMBER >= 1 ) return;
//			printf("updating\n");
			prepareVerticesBuffer(2*x,2*y);
			MeshesVector[CURRENT_CHUNK_NUMBER]->batch.SetVertNumber(MeshesVector[CURRENT_CHUNK_NUMBER]->drawnElementCounter*vertexCount);
			MeshesVector[CURRENT_CHUNK_NUMBER]->batch.ChangeVertexData3f(_vertices,sizeof(GLfloat) * 3 * vertexCount);
			MeshesVector[CURRENT_CHUNK_NUMBER]->batch.ChangeNormalDataf(_normals,sizeof(GLfloat) * 3 * vertexCount);
		}

		void createPlayerTail()
		{
  			printf("creating new player tail ");
			meshStruct *mesh = new meshStruct();
			mesh->drawIndicesCount = 0;
			mesh->drawnElementCounter = 0;
			mesh->lastIndicesAdded = 0;
			mesh->oldX = 0;
			mesh->oldY = 0;
//			x*vertexCount*3*4 288
			mesh->batch.Begin(GL_TRIANGLES, elementNumber*vertexCount,0);
			mesh->batch.ChangeVertexData3f(_vertices,sizeof(GLfloat) * 3 * vertexCount);
			mesh->batch.ChangeNormalDataf(_normals,sizeof(GLfloat) * 3 * vertexCount);
			mesh->batch.End();

			glGenBuffers(1, &mesh->iboNumber);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->iboNumber);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(_indices[0]) * indexCount * elementNumber, NULL, GL_DYNAMIC_DRAW);
			printf("tailbatch done ibo:%d \n",mesh->iboNumber);
			MeshesVector.push_back(mesh);
		}

		~Mesh3D()
		{
//			delete &myMesh3D;
			auto iterator = MeshesVector.begin();
			while ( iterator != MeshesVector.end() )
			{
				delete (*iterator);
				iterator = MeshesVector.erase(iterator);
			}
			printf("removed all chunks\n");
		}

		void drawMesh()
		{
			for ( auto iterator = MeshesVector.begin(); iterator != MeshesVector.end(); iterator++ )
			{

				(*iterator)->batch.DrawElements((*iterator)->drawIndicesCount,GL_UNSIGNED_INT,(*iterator)->iboNumber);
//				printf("draw indices count:%d ibo:%d elements count:%d\n",(*iterator)->drawIndicesCount,(*iterator)->iboNumber,(*iterator)->drawnElementCounter);
			}
		}
};
