#pragma once

#include "qmatrix4x4.h"

	class GLWidget3D;

	class GLWidgetSimpleShadow
	{
	public:
		GLWidgetSimpleShadow();
		~GLWidgetSimpleShadow();	

		void makeShadowMap(GLWidget3D* glWidget3D);
		void initShadow(int _programId,GLWidget3D* glWidget3D);
		void shadowRenderScene(int program);

		void updateShadowMatrix(GLWidget3D* glWidget3D);
		
		// show depth textures
		bool displayDepthTex;
		bool displayDepthTexInit;
		uint depthTexProgram;
		uint depthTexVBO;
		uint fustrumVBO;
		//std::vector<float> quadDepthTex;
		float applyCropMatrix(GLWidget3D* glWidget3D);

		int programId;
		//QVector3D light_position;

		QMatrix4x4 light_biasMatrix;
		QMatrix4x4 light_pMatrix;
		QMatrix4x4 light_mvMatrix;
		QMatrix4x4 light_mvpMatrix;

		void setEnableShadowMap(bool enableDisable);
	};



