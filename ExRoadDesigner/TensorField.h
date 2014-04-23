#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "Polygon2D.h"
#include "BBox.h"
#include "Polyline2D.h"

class TensorField {
public:
	Polygon2D area;
	BBox bbox;
	cv::Mat mat;

public:
	TensorField() {}
	~TensorField() {}

	void init(const Polygon2D &area, const Polyline2D &hintLine, const Polyline2D &exampleHintLine);
	float get(const QVector2D &pt);
	void save(const QString &filename);
};

