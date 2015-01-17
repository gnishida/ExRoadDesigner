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
	void init(const QVector3D& _minPos, const QVector3D& _maxPos, int imgResX, int imgResY);
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

	//text
	GLuint texData;

	// control
	void addGaussian(float coordX, float coordY, float height, float sigma);
	void updateGaussian(float coordX, float coordY, float height, float rad);
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