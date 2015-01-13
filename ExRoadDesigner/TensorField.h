#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "Polygon2D.h"
#include "BBox.h"
#include "Polyline2D.h"
#include "ExFeature.h"

class TensorField {
public:
	BBox bbox;
	cv::Mat mat;
	float scale;

public:
	TensorField() {}
	~TensorField() {}

	void init(const Polygon2D &area, const Polyline2D &hintLine, std::vector<ExFeature> &features, float scale = 1.0f);
	float get(const QVector2D &pt);
	void save(const QString &filename);

private:
	float getAverage(int row, int col);
};

