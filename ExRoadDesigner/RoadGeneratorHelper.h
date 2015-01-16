#pragma once

#include "glew.h"

#include "VBORenderManager.h"
#include "RoadGraph.h"
#include "ExFeature.h"
//#include "Terrain.h"
#include "VBORenderManager.h"
#include "Patch.h"

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
	static bool canConnectToVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc);
	static bool canConnectToFixVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc);
	static bool canSnapToEdge(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadEdgeDesc& snapEdge, QVector2D &closestPt);
	static bool canSnapToFixEdge(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadEdgeDesc& snapEdge, QVector2D &closestPt);

	static bool getVertexForSnapping(VBORenderManager& vboRenderManager, RoadGraph& roads, RoadVertexDesc srcDesc, float distance_threshold, float z_threshold, float angle, float angle_threshold, RoadVertexDesc& nearest_desc);
	static bool getEdgeForSnapping(VBORenderManager& vboRenderManager, RoadGraph& roads, RoadVertexDesc srcDesc, float distance_threshold, float z_threshold, float angle, float angle_threshold, RoadEdgeDesc& nearest_desc, QVector2D& nearestPt);

	static bool isRedundantEdge(RoadGraph& roads, RoadVertexDesc v_desc, const Polyline2D &polyline, float angleTolerance);

	static QVector2D modulo(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt, BBox &bbox);
	static bool containsInitialSeed(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt);

	static void createFourDirection(float direction, std::vector<float> &directions);
	static Polyline2D createOneEdge(int roadType, float length, float curvature, float angle, float step_size);
	static void createFourEdges(ExFeature &f, int roadType, const QVector2D &ex_pt, int lanes, float direction, float step, std::vector<RoadEdgePtr> &edges);

	static void chooseEdgeLengthAndCurvature(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, float &length, float &curvature, int &lanes);
	static void chooseEdgeLengthAndCurvature(RoadGraph &roads, const QVector2D &ex_pt, float distance, float direction, float &length, float &curvature);

	static void removeDeadend(RoadGraph& roads);
	static void extendDanglingEdges(RoadGraph &roads, float maxDistance);
	static void connectRoads(RoadGraph& roads, float distance_threshold, float angle_threshold);
	static void connectRoads2(RoadGraph &roads, float distance_threshold);
	static void connectRoads3(RoadGraph &roads, VBORenderManager* vboRenderManager, float avenueThreshold, float streetThreshold);
	static bool growRoadOneStep(RoadGraph& roads, RoadVertexDesc srcDesc, const QVector2D& step);
	static void cutEdgeByWater(Polyline2D &polyline, VBORenderManager& vboRenderManager, float z_threshold);
	static void bendEdgeBySteepElevationChange(int roadType, Polyline2D &polyline, VBORenderManager *vboRenderManager);
	static void bendEdgeBySteepElevationChange(Polyline2D &polyline, float z0, VBORenderManager *vboRenderManager);
	static bool submerged(int roadType, RoadGraph &roads, VBORenderManager *vboRenderManager);
	static bool submerged(int roadType, const Polyline2D &polyline, VBORenderManager *vboRenderManager);
	//static bool steepSlope(RoadGraph &roads, VBORenderManager *vboRenderManager);
	static float maxZ(RoadGraph &roads, VBORenderManager *vboRenderManager);
	static float minZ(RoadGraph &roads, VBORenderManager *vboRenderManager, bool checkConnectors);
	static float diffZ(RoadGraph &roads, VBORenderManager *vboRenderManager);
	static float diffSlope(RoadGraph &roads, VBORenderManager *vboRenderManager);
	static float largestAngleBetweenEdges(RoadGraph& roads, RoadVertexDesc srcDesc, int roadType);
	static float getFirstEdgeAngle(RoadGraph& roads, RoadVertexDesc srcDesc);
	static QVector2D getFirstEdgeDir(RoadGraph& roads, RoadVertexDesc srcDesc);

	static void removeIntersectionsOnRiver(RoadGraph &roads, VBORenderManager *vboRenderManager, float seaLevel);
	static void removeSmallBlocks(RoadGraph &roads, float minArea);

	static RoadVertexDesc getClosestVertexByExample(RoadGraph &roads, RoadVertexDesc v_desc);
	
	static int getRelativeDirectionInArea(const BBox &bbox, const QVector2D &pt);

	static bool isWithinScaledArea(const Polygon2D &area, float factor, const QVector2D &pt);

	static bool isShape(RoadGraph &roads, RoadVertexDesc desc, std::vector<RoadEdgeDescs> &shapes, int &shape_index);
	static std::vector<Patch> convertToPatch(int roadType, RoadGraph& roads, RoadGraph& avenues, std::vector<RoadEdgeDescs> &shapes);
	static void markConnectorToEdge(RoadGraph &roads, RoadVertexDesc srcDesc);

	static RoadVertexDesc createEdgesByExample(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges, float &rotation_angle);
	static RoadVertexDesc createEdgesByExample2(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges);

	static void saveRoadImage(RoadGraph& roads, std::list<RoadVertexDesc>& seeds, const char* filename);
};

