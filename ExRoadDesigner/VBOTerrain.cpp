#include "VBOTerrain.h"
#include "VBOUtil.h"
#include <QImage>
#include <QGLWidget>
#include "global.h"
#include "VBORenderManager.h"

VBOTerrain::VBOTerrain() {
	initialized = false;
	texId = 0;
	_resolution = 200;
}

void VBOTerrain::init(VBORenderManager* rendManager, int resolution) {
	this->rendManager = rendManager;
	this->_resolution = resolution;

	if (!initialized) {
		int numLevels=4;
		std::vector<QString> fileNames(numLevels);
		for (int i = 0; i < numLevels; ++i) {
			fileNames[i] = "../data/textures/0" + QString::number(i+1) + "_terrain.jpg";
		}
		grassText = VBOUtil::loadImageArray(fileNames);
	}

	// TERRAIN LAYER
	layerData = cv::Mat(resolution + 1, resolution + 1, CV_32FC1, cv::Scalar(70.0f));
	updateTexture();

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

void VBOTerrain::render(bool drawEditingCircle) {
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
	glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(4*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(8*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(12*sizeof(float)));

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
	if (editionMode && drawEditingCircle) {
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
			points[i] = Vertex(pos.x(),pos.y(),pos.z(), QColor(), 0,0,1.0f,0,0,0);
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
	float x0 = u * _resolution;
	float y0 = v * _resolution;
	float sigma = rad_ratio * _resolution;

	for (int c = 0; c < layerData.cols; c++) {
		for (int r = 0; r < layerData.rows; r++) {
			float x = c;
			float y = r;

			float z = layerData.at<float>(r, c) + height * expf(-(SQR(x - x0) + SQR(y - y0)) / (2 * sigma * sigma));
			if (z < 0) z = 0.0f;
			layerData.at<float>(r, c) = z;
		}
	}

	// update texture
	updateTexture();
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
	float x0 = u * _resolution;
	float y0 = v * _resolution;
	float rad = rad_ratio * _resolution;

	for (int c = x0 - rad; c <= x0 + rad + 1; ++c) {
		if (c < 0 || c >= layerData.cols) continue;
		for (int r = y0 - rad; r <= y0 + rad + 1; ++r) {
			if (r < 0 || r >= layerData.rows) continue;

			float x = c;
			float y = r;

			if (SQR(x - x0) + SQR(y - y0) > SQR(rad)) continue;
			
			layerData.at<float>(r, c) = height;
		}
	}

	// update texture
	updateTexture();
}

/**
 * Return the terrain elevation.
 *
 * @param u			X coordinate [0, 1]
 * @param v			Y coordinate [0, 1]
 */
float VBOTerrain::getTerrainHeight(float u, float v) {
	if (u < 0) u = 0.0f;
	if (u >= 1.0f) u = 1.0f;
	if (v < 0) v = 0.0f;
	if (v >= 1.0f) v = 1.0f;

	assert(_resolution + 1 == layerData.cols);
	assert(_resolution + 1 == layerData.rows);

	int c1 = u * _resolution;
	int c2 = u * _resolution + 1;
	int r1 = v * _resolution;
	int r2 = v * _resolution + 1;


	if (c1 >= layerData.cols) c1 = layerData.cols - 1;
	if (c2 >= layerData.cols) c2 = layerData.cols - 1;
	if (r1 >= layerData.rows) r1 = layerData.rows - 1;
	if (r2 >= layerData.rows) r2 = layerData.rows - 1;

	float v1 = layerData.at<float>(r1, c1);
	float v2 = layerData.at<float>(r2, c1);
	float v3 = layerData.at<float>(r1, c2);
	float v4 = layerData.at<float>(r2, c2);

	float v12,v34;
	if (r2 == r1) {
		v12 = v1;
		v34 = v3;
	} else {
		float t = v * _resolution - r1;
		v12 = v1 * (1-t) + v2 * t;
		v34 = v3 * (1-t) + v4 * t;
	}

	if (c2 == c1) {
		return v12;
	} else {
		float s = u * _resolution - c1;
		return v12 * (1-s) + v34 * s;
	}
}

void VBOTerrain::loadTerrain(const QString& fileName) {
	cv::Mat loadImage = cv::imread(fileName.toUtf8().data(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat tmp = cv::Mat(loadImage.rows, loadImage.cols, CV_32FC1, loadImage.data);
	tmp.copyTo(layerData);

	_resolution = loadImage.rows - 1;

	// update texture
	updateTexture();
}
	
void VBOTerrain::saveTerrain(const QString& fileName) {
	cv::Mat saveImage	= cv::Mat(layerData.rows, layerData.cols, CV_8UC4, layerData.data);
	cv::imwrite(fileName.toUtf8().data(), saveImage);
}

/**
 * Update the texture according to the layer data.
 */
void VBOTerrain::updateTexture() {
	if (texId != 0) {
		glDeleteTextures(1, &texId);
		texId = 0;
	}

	glActiveTexture(GL_TEXTURE7);
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, layerData.cols, layerData.rows);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, layerData.cols, layerData.rows, GL_RED, GL_FLOAT, layerData.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glActiveTexture(GL_TEXTURE0);
}