#pragma once

#include "RoadGraph.h"

class PatchConnector
{
public:
	std::vector<RoadVertexDesc> vertices;
};

class Patch
{
public:
	RoadGraph roads;
	std::vector<RoadVertexDesc> connectors;

public:
	Patch();

	float getMinCost(const Polyline2D& polyline, RoadVertexDesc& v_connector, RoadVertexDesc& v_root);
	float getMinHausdorffDist(const Polyline2D& polyline, RoadVertexDesc& v_connector, RoadVertexDesc& v_root);
	float getCost(const Polyline2D& polyline, RoadVertexDesc connector, RoadVertexDesc& root_desc);
	float getHausdorffDist(const Polyline2D& polyline, RoadVertexDesc connector, RoadVertexDesc& root_desc);
};

