#pragma once

#include "glew.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "RoadGraph.h"

class DetectedCircle {
public:
	QVector2D center;
	QVector2D scaledCenter;
	float radius;
	float scaledRadius;
	RoadEdgeDescs edges;

public:
	DetectedCircle() {}
	~DetectedCircle() {}
};

class CircleHoughTransform {
public:
	CircleHoughTransform() {}
	~CircleHoughTransform() {}

	static std::vector<RoadEdgeDescs> detect(RoadGraph &roads, float maxRadius, float houghThreshold);
};

