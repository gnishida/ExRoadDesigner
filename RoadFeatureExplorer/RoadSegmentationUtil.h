#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <road/RoadGraph.h>
#include <road/feature/RoadFeature.h>
#include <road/feature/GridFeature.h>
#include <road/feature/RadialFeature.h>
#include <road/feature/KDEFeature.h>
#include <road/feature/GenericFeature.h>

class RoadSegmentationUtil {
protected:
	RoadSegmentationUtil() {}
	~RoadSegmentationUtil() {}

public:
	static void detectGrid(RoadGraph& roads, Polygon2D& area, int roadType, RoadFeature& roadFeature, int maxIteration, float numBins, float minTotalLength, float minMaxBinRatio, float angleThreshold, float votingRatioThreshold, float extendingDistanceThreshold, float minObbLength);
	static bool detectOneGrid(RoadGraph& roads, Polygon2D& area, int roadType, GridFeature& gf, int numBins, float minTotalLength, float minMaxBinRatio, float angleThreshold, float votingRatioThreshold, float extendingDistanceThreshold, float minObbLength);
	static int traverseConnectedEdges(RoadGraph& roads, RoadEdgeDesc e, QMap<RoadEdgeDesc, int>& edges, int segment_id);
	static void reduceGridGroup(RoadGraph& roads, GridFeature& gf, QMap<RoadEdgeDesc, float>& edges);
	static void extendGridGroup(RoadGraph& roads, Polygon2D& area, int roadType, GridFeature& gf, QMap<RoadEdgeDesc, float>& edges, float angleThreshold, float votingRatioThreshold, float distanceThreshold);

	static void detectRadial(RoadGraph& roads, Polygon2D& area, int roadType, RoadFeature& roadFeature, int maxIteration, float scale1, float scale2, float centerErrorTol2, float angleThreshold2, float scale3, float centerErrorTol3, float angleThreshold3, float sigma, float votingRatioThreshold, float seedDistance, float minSeedDirections, float extendingAngleThreshold);
	static bool detectOneRadial(RoadGraph& roads, Polygon2D& area, int roadType, RadialFeature& rf, float scale1, float scale2, float centerErrorTol2, float angleThreshold2, float scale3, float centerErrorTol3, float angleThreshold3, float detectCircleThreshold, float sigma, float votingRatioThreshold, float seedDistance, float minSeedDirections, float extendingAngleThreshold);

	static void detectRadialCenterInScaled(RoadGraph& roads, Polygon2D& area, int roadType, float scale, float sigma, RadialFeature& rf);
	static std::vector<RadialFeaturePtr> detectRadialCentersInScaled(RoadGraph& roads, Polygon2D& area, int roadType, float scale, float sigma, float candidateCenterThreshold, float detectCircleThreshold, float min_dist);

	static void refineRadialCenterInScaled(RoadGraph& roads, Polygon2D& area, int roadType, float scale, float sigma, RadialFeature& rf, float distanceThreshold, float angleThreshold);
	static bool detectCircle(RoadGraph& roads, Polygon2D& area, int roadType, float detectCircleThreshold, RadialFeature& rf);
	static bool findOneRadial(RoadGraph& roads, Polygon2D& area, int roadType, float angleThreshold, float votingRatioThreshold, float seedDistance, float minSeedDirections, float extendingAngleThreshold, RadialFeature& rf, QMap<RoadEdgeDesc, bool>& edges);

	static void reduceRadialGroup(RoadGraph& roads, RadialFeature& rf, QMap<RoadEdgeDesc, bool>& edges, float distanceThreshold);
	static void extendRadialGroup(RoadGraph& roads, Polygon2D& area, int roadType, RadialFeature& rf, QMap<RoadEdgeDesc, bool>& edges, float angleThreshold, float dirCheckRatio);
	static int countNumDirections(RoadGraph& roads, const RadialFeature& rf, QMap<RoadEdgeDesc, bool>& edges, int size);
	static void buildRadialArea(RoadGraph& roads, QMap<RoadEdgeDesc, bool>& edges, RadialFeature& rf);
};

