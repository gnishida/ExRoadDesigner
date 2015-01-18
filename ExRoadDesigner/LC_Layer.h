#pragma once

#include "glew.h"

#include <qmatrix4x4.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QtCore/qmath.h>
#include <QFile.h>

#include "VBOUtil.h"


class Layer {
private:
	bool initialized;
	QVector3D maxPos;
	QVector3D minPos;
	int imgResX;
	int imgResY;

public:
	Layer();

	void init(const QVector3D& _minPos, const QVector3D& _maxPos, int imgResX, int imgResY);

	// control
	void addGaussian(float u, float v, float height, float rad_ratio);
	void updateGaussian(float u, float v, float height, float rad_ratio);
	void excavate(float u, float v, float height, float rad_ratio);
	float getValue(float u, float v);

	void loadLayer(const QString& fileName);
	void saveLayer(const QString& fileName);

public: // GEN: to access from the outside
	cv::Mat layerData;
};