#pragma once

#include "glew.h"

#include <qmatrix4x4.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QtCore/qmath.h>
#include <QFile.h>

#include "VBOUtil.h"


class Layer{
public:
	Layer();
	void init(QVector3D _minPos,QVector3D _maxPos,int _resolutionX,int _resolutionY,int _typeLayer,int imgResX,int imgResY);
	//std::vector<float> amplitudes;
	//std::vector<QVector3D> samplePosition;
	//float stdev;

	//void updateGeometry();
	//std::vector<Vertex> vert;

	bool initialized;
	QVector3D maxPos;
	QVector3D minPos;

	int imgResX;
	int imgResY;
	int resolutionX;
	int resolutionY;
	float sideX;
	float sideY;

	int typeLayer;
	// 0 vboRenderManager

	//void createRandomDistribution();
	//float sample(QVector3D samPos);

	//text
	//void updateTexture();
	GLuint texData;

	// control
	//void updateLayer(QVector3D pos,float change);
	void updateLayer(float coordX,float coordY,float change,float rad);
	void updateLayerNewValue(float coordX,float coordY,float newValue,float rad);
	float getValue(float xM,float yM);
	void smoothLayer();
	//int closestSample(QVector3D pos);

	// perlin noise
	void randomPerlineNoise(cv::Mat& perlinNoise);

	void loadLayer(QString& fileName);
	void saveLayer(QString& fileName);

public: // GEN: to access from the outside
	void updateTexFromData();
	cv::Mat layerData;

};//