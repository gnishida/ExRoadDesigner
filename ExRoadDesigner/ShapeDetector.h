#pragma once

#include "RoadGraph.h"

class ShapeDetector {
public:
	ShapeDetector() {}
	~ShapeDetector() {}

	static std::vector<RoadEdgeDescs> detect(RoadGraph &roads, float scale, float threshold);
	static void addVerticesToGroup(RoadGraph &roads, RoadVertexDesc v, float threshold, RoadEdgeDescs &shape, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, bool> &usedEdges);
};
