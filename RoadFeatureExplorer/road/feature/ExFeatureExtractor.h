#pragma once

#include "../RoadGraph.h"
#include "ExFeature.h"

class ExFeatureExtractor {
public:
	ExFeatureExtractor() {}
	~ExFeatureExtractor() {}

	static void extractFeature(RoadGraph& roads, Polygon2D& area, Polyline2D& hintLine, ExFeature& feature);

	static int extractAvenueFeature(RoadGraph &roads, const Polygon2D &area, ExFeature &feature);
	static int extractStreetFeature(RoadGraph &orig_roads, const Polygon2D &area, ExFeature &feature);
};

