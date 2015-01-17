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
	void addGaussian(float u, float v, float height, float rad_ratio);
	void updateGaussian(float u, float v, float height, float rad_ratio);
	void excavate(float u, float v, float height, float rad_ratio);
	void updateLayerNewValue(float coordX,float coordY,float newValue,float rad);
	float getValue(float xM,float yM);
	void smoothLayer();

	// perlin noise
	void randomPerlineNoise(cv::Mat& perlinNoise);

	void loadLayer(const QString& fileName);
	void saveLayer(const QString& fileName);

public: // GEN: to access from the outside
	void updateTexFromData();
	cv::Mat layerData;

};//