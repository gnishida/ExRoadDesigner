#pragma once

#include "RoadGraph.h"
#include "Patch.h"

class ShapeDetector {
public:
	ShapeDetector() {}
	~ShapeDetector() {}

	static std::vector<RoadEdgeDescs> detect(RoadGraph &roads, float maxRadius, float threshold);
	static void addVerticesToCircle(RoadGraph &roads, RoadEdgeDescs& shape, int patchId, float threshold, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, int> &usedEdges);
	static void addVerticesToGroup(RoadGraph &roads, RoadVertexDesc v, int patchId, float threshold, RoadEdgeDescs &shape, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, int> &usedEdges);
	//static void savePatchImages(RoadGraph& roads, std::vector<RoadEdgeDescs> shapes);
};
