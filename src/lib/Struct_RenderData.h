#pragma once

#include<vector>
#include<game/Mesh3D.h>

struct Struct_RenderData
{
	public:
		float ScaleX,ScaleY, ScaleZ;
		float ratioX, ratioY, ratioZ;
		int viewportWidth,viewportHeight;

		GLShaderManager shaderManager; // Shader Manager
		GLMatrixStack modelViewMatrix; // Modelview Matrix
		GLFrustum viewFrustum; // View Frustum
		GLGeometryTransform transformPipeline; // Geometry Transform Pipeline
		GLFrame cameraFrame; // Camera frame
		GLMatrixStack projectionMatrix; // Projection Matrix

		GLfloat vWhite[4];
		GLfloat vBlack[4];
		GLfloat vLightPos[4];
		GLfloat vGreen[4];
		GLfloat vRed[4];
		GLfloat vBlue[4];

		M3DVector4f vLightTransformed;

		GLTriangleBatch sphereBatch;
		GLBatch frameBatch;
		GLBatch squareBatch;
		GLBatch backgroundBatch;
		std::vector<Mesh3D*> tailBatches;
//		std::vector<GLBatch*> tailBatches;
};

Struct_RenderData *getRenderData(){
	static Struct_RenderData data;
	return &data;
}
