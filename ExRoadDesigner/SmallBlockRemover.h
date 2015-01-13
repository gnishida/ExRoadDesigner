#pragma once

#include "glew.h"

#include "RoadGraph.h"

class SmallBlockRemover {
public:
	SmallBlockRemover() {}
	~SmallBlockRemover() {}

	static bool removeIntersectingEdges(RoadGraph &roads);
	static void remove(RoadGraph &roads, float minArea);
	static bool removeEdge(RoadGraph &roads, std::vector<RoadEdgeDesc> &edges);
};

