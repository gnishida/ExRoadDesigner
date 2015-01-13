#pragma once

#include "../../common/Polygon2D.h"
#include "../RoadGraph.h"
#include "../feature/RadialFeature.h"

class RadialRoadGenerator {
public:
	RadialRoadGenerator();
	~RadialRoadGenerator();

	void generateRoadNetwork(RoadGraph &roads, const Polygon2D &area, const RadialFeature& rf);

private:
	void generateCircleAvenues(RoadGraph& roads, const Polygon2D& area, const RadialFeature& rf, std::list<RoadVertexDesc>& seeds);
	void expandRadialAvenues(RoadGraph& roads, const Polygon2D& area, const RadialFeature& rf, RoadVertexDesc desc);
};

