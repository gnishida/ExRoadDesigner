#pragma once

#include "glew.h"

#include "RoadGraph.h"
#include "ExFeature.h"
#include "Terrain.h"

/**
 * 道路網生成のヘルパークラス。
 * 道路網生成に必要な、汎用的な関数などを実装する。
 */
class RoadGeneratorHelper {
protected:
	RoadGeneratorHelper() {}
	~RoadGeneratorHelper() {}

public:
	static bool intersects(RoadGraph &roads, const QVector2D& p0, const QVector2D& p1, RoadEdgeDesc &eiClosest, QVector2D &closestIntPt);
	static bool intersects(RoadGraph &roads, RoadVertexDesc srcDesc, const Polyline2D &polyline, RoadEdgeDesc &eiClosest, QVector2D &closestIntPt);
	static bool canSnapToVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc);
	static bool canConnectToVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc);
	static bool canSnapToEdge(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadEdgeDesc& snapEdge, QVector2D &closestPt);

	static float getNearestVertex(RoadGraph& roads, const QVector2D& pos, RoadVertexDesc srcDesc, RoadVertexDesc& snapDesc);
	static float getNearestEdge(RoadGraph& roads, const QVector2D& pt, RoadVertexDesc srcDesc, RoadEdgeDesc& snapEdge, QVector2D &closestPt);
	//static RoadVertexDesc getNearestVertexWithKernel(RoadGraph &roads, const QVector2D &pt);

	//static bool invadingTerritory(RoadGraph &roads, const QVector2D &pt, RoadVertexDesc srcVertex, const QVector2D &targetPt);

	//static int getClosestItem(const KDEFeature &f, int roadType, const QVector2D &pt);
	static bool isRedundantEdge(RoadGraph& roads, RoadVertexDesc v_desc, const Polyline2D &polyline, float angleTolerance);

	static QVector2D modulo(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt, BBox &bbox);
	static bool containsInitialSeed(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt);

	static void createFourDirection(float direction, std::vector<float> &directions);
	static void createFourEdges(ExFeature &f, int roadType, const QVector2D &ex_pt, int lanes, float direction, float step, std::vector<RoadEdgePtr> &edges);
	static void chooseEdgeLengthAndCurvature(RoadGraph &roads, const QVector2D &ex_pt, float distance, float direction, float &length, float &curvature);

	static void removeDeadend(RoadGraph& roads);
	static void extendDanglingEdges(RoadGraph &roads);
	static void connectRoads(RoadGraph& roads, float distance_threshold, float angle_threshold);
	static bool growRoadOneStep(RoadGraph& roads, RoadVertexDesc srcDesc, const QVector2D& step);
	static void cutEdgeBySteepElevationChange(Polyline2D &polyline, Terrain *terrain);

	static RoadVertexDesc getClosestVertexByExample(RoadGraph &roads, RoadVertexDesc v_desc);
	
	static int getRelativeDirectionInArea(const BBox &bbox, const QVector2D &pt);

	static bool isWithinScaledArea(const Polygon2D &area, float factor, const QVector2D &pt);

	//static void buildGraphFromKernel(RoadGraph& roads, const KDEFeatureItem &item, const QVector2D &offset);

	static void saveSnappingImage(RoadGraph &roads, const Polygon2D &area, RoadVertexDesc srcDesc, const Polyline2D &old_polyline, const Polyline2D &new_polyline, RoadVertexDesc snapDesc, const QString &filename_prefix);


	static void check(RoadGraph &roads);
};

