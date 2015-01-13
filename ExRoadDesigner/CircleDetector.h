#pragma once

#include "glew.h"

#include <vector>
#include "RoadGraph.h"

class CircleDetector {
public:
	CircleDetector() {}
	~CircleDetector() {}

	std::vector<RoadGraph> detect(RoadGraph& roads);
	QVector2D detectOneCircleCenter(RoadGraph& roads, Polygon2D& area, float scale, float sigma);
};

