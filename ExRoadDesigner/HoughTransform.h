#pragma once

#include "glew.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "common.h"
#include "Polygon2D.h"
#include "BBox.h"

class HoughTransform {
private:
	Polygon2D area;
	float scale;
	BBox bbox;
	cv::Mat htSpace;

public:
	HoughTransform(const Polygon2D& area, float scale);
	~HoughTransform() {}

	void line(const QVector2D& p1, const QVector2D& p2, float sigma);
	void circle(const QVector2D& p1, const QVector2D& p2, float sigma);
	QVector2D maxPoint() const;
	std::vector<QVector2D> points(float threshold) const;
};

