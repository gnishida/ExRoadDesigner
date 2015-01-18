#include "VBOTerrain.h"
#include <QFileInfo>
#include "VBOUtil.h"
#include "qimage.h"
#include <QGLWidget>

#include "global.h"

#include "VBORenderManager.h"


VBOTerrain::VBOTerrain() {
	initialized = false;
	_texId = 0;
	_resolution = 200;
}

void VBOTerrain::init(VBORenderManager* rendManager, int resolution) {
	this->rendManager = rendManager;
	this->_resolution = resolution;

	if (!initialized) {
		int numLevels=4;
		std::vector<QString> fileNames(numLevels);
		for(int i=0;i<numLevels;i++){
			fileNames[i]="../data/textures/0"+QString::number(i+1)+"_terrain.jpg";
		}
		grassText=VBOUtil::loadImageArray(fileNames);
	}

	// TERRAIN LAYER
	terrainLayer.init(rendManager->minPos, rendManager->maxPos, resolution + 1, resolution + 1);
	updateTexFromData();

	//////////////////
	// VERTICES
	if (initialized) {
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &elementbuffer);
		vbo = 0;
		elementbuffer = 0;
	}

	std::vector<Vertex> vert;
	float step = rendManager->side / resolution;
		
	// VERTEX
	vert.resize((resolution + 1) * (resolution + 1));
	int count = 0;
	for (int row = 0; row <= resolution; ++row) {
		for (int col = 0; col <= resolution; ++col) {
			QVector3D pos = QVector3D(col * step, row * step, 0) + rendManager->minPos;
			vert[count] = Vertex(pos,pos/100.0f);
			count++;
		}
	}
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vert.size(), vert.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// INDEX
	std::vector<uint> indices;
	indices.resize(resolution * resolution * 4);
	count = 0;
	for (int row = 0; row < resolution; ++row) {
		for (int col = 0; col < resolution; ++col) {
			indices[count] = col + row * (resolution + 1);
			indices[count + 1] = col+1 + row * (resolution + 1);
			indices[count + 2] = col+1 + (row+1) * (resolution + 1);
			indices[count + 3] = col + (row+1) * (resolution + 1);
			count += 4;
		}
	}

	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);
	indicesCount=indices.size();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	initialized = true;
}

void VBOTerrain::render() {//bool editionMode,QVector3D mousePos
	bool editionMode = rendManager->editionMode;
	QVector3D mousePos = rendManager->mousePos3D;

	//////////////////////////////////////////
	// TERRAIN
	//glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	GLuint vao;
	glGenVertexArrays(1,&vao); 
	glBindVertexArray(vao); 
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D,0); 
	glBindTexture(GL_TEXTURE_2D_ARRAY, grassText);
		
	glActiveTexture(GL_TEXTURE0);

	glUniform1i(glGetUniformLocation(rendManager->program, "mode"), 3);//MODE: terrain
	glUniform1i(glGetUniformLocation(rendManager->program, "tex_3D"), 8);//tex0: 0

	glUniform1i(glGetUniformLocation(rendManager->program, "terrain_tex"), 7);//tex0: 0
	glUniform4f(glGetUniformLocation(rendManager->program, "terrain_size"), 
		rendManager->minPos.x(),
		rendManager->minPos.y(),
		(rendManager->maxPos.x() - rendManager->minPos.x()), 
		(rendManager->maxPos.y() - rendManager->minPos.y())
		);//tex0: 0

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));

	// Draw the triangles 
	glDrawElements(GL_QUADS, indicesCount, GL_UNSIGNED_INT, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glBindVertexArray(0);
	glDeleteVertexArrays(1,&vao);

	//////////////////////
	// EDITION
	if (editionMode) {
		glLineWidth(5.0f);
		glPointSize(10.0f);

		VBOUtil::check_gl_error(">>editionMode");

		const float pointOffset=5.0f;
		GLuint vao;
		glGenVertexArrays(1,&vao);
		glBindVertexArray(vao);
		GLuint edPointsVBO;
		glGenBuffers(1, &edPointsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, edPointsVBO);
			
		std::vector<Vertex> points;
		int numPointsCircle=20;
		points.resize(numPointsCircle);

		QVector3D posM = rendManager->mousePos3D;//(0,0,0);
		// circle
		float radius = G::global().getFloat("2DterrainEditSize");
		int degreesSegmentCircle=360.0f/numPointsCircle;
		for (int i=0; i < numPointsCircle; i++){
			float degInRad = i*degreesSegmentCircle*0.0174532925f;//degree to rad
			QVector3D pos(cos(degInRad)*radius,sin(degInRad)*radius,0);
			pos+=posM;
			pos.setZ(50.0f);
			points[i]=Vertex(pos.x(),pos.y(),pos.z(),0,0,1.0f,0,0,0,0,0,0);
		}


		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*points.size(), points.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
		glEnableVertexAttribArray(1);
		VBOUtil::check_gl_error("aa editionMode");
		glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));
		glUniform1i(glGetUniformLocation(rendManager->program, "mode"), 1|mode_AdaptTerrain);//MODE: color

		//draw points
		glDrawArrays(GL_POINTS,0,points.size()-numPointsCircle);
		//draw circle
		glDrawArrays(GL_LINE_LOOP,points.size()-numPointsCircle,numPointsCircle);

		glDeleteBuffers(1,&edPointsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDeleteVertexArrays(1,&vao);
	}
}

/**
 * ガウシアン分布に基づき、高さを上げる。
 *
 * @param u				中心のX座標 [0, 1]
 * @param v				中心のY座標 [0, 1]
 * @param height		ガウス分布の最大高さ
 * @param rad_ratio		半径のサイズ（グリッドサイズに対する比）
 */
void VBOTerrain::updateGaussian(float u, float v, float height, float rad_ratio) {
	terrainLayer.updateGaussian(u, v, height, rad_ratio);

	// update texture
	updateTexFromData();
}

/**
 * 指定した位置を中心とする円の範囲の高さを、指定した高さheightにする。
 *
 * @param u				中心のX座標 [0, 1]
 * @param v				中心のY座標 [0, 1]
 * @param height		この高さに設定する
 * @param rad_ratio		半径のサイズ（グリッドサイズに対する比）
 */
void VBOTerrain::excavate(float u, float v, float height, float rad_ratio) {
	terrainLayer.excavate(u, v, height, rad_ratio);

	// update texture
	updateTexFromData();
}

/**
 * Return the terrain elevation.
 *
 * @param u			X coordinate [0, 1]
 * @param v			Y coordinate [0, 1]
 */
float VBOTerrain::getTerrainHeight(float u, float v) {
	return terrainLayer.getValue(u, v);
}

void VBOTerrain::loadTerrain(const QString& fileName) {
	terrainLayer.loadLayer(fileName);

	// update texture
	updateTexFromData();
}
	
void VBOTerrain::saveTerrain(const QString& fileName) {
	terrainLayer.saveLayer(fileName);
}

// GEN: 1/16/2015. Change to not use a temporary file.
void VBOTerrain::updateTexFromData() {
	if (_texId != 0) {
		glDeleteTextures(1, &_texId);
		_texId = 0;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glGenTextures(1, &_texId);
	glBindTexture(GL_TEXTURE_2D, _texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, terrainLayer.layerData.cols, terrainLayer.layerData.rows, 0, GL_RED, GL_FLOAT, terrainLayer.layerData.data);

	glGenerateMipmap(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, _texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glActiveTexture(GL_TEXTURE0);
}