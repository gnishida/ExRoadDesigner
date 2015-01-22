#pragma once

#include "Polygon2D.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "MainWindow.h"
//#include "Terrain.h"
#include "VBORenderManager.h"

class PatchRoadGenerator {
private:
	MainWindow *mainWin;
	RoadGraph &roads;
	Polygon2D targetArea;
	Polyline2D hintLine;
	VBORenderManager *vboRenderManager;
	std::vector<ExFeature> &features;

public:
	PatchRoadGenerator(MainWindow *mainWin, RoadGraph &roads, const Polygon2D &targetArea, const Polyline2D &hintLine, VBORenderManager *vboRenderManager, std::vector<ExFeature> &features) : mainWin(mainWin), roads(roads), targetArea(targetArea), hintLine(hintLine), vboRenderManager(vboRenderManager), features(features) {}
	~PatchRoadGenerator() {}

	void generateRoadNetwork();

private:
	void generateAvenueSeeds(std::list<RoadVertexDesc>& seeds);
	bool addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, std::list<RoadVertexDesc>& seeds);
	void generateStreetSeeds(std::list<RoadVertexDesc> &seeds);
	void generateStreetSeeds2(std::list<RoadVertexDesc> &seeds);

	bool attemptConnect(int roadType, RoadVertexDesc srcDesc, int ex_id, std::vector<ExFeature>& features);
	bool attemptConnectToVertex(int roadType, RoadVertexDesc srcDesc, std::vector<ExFeature>& features, float dist_threshold, float z_threshold, float direction, float angle_threshold, float roadAngleTolerance);
	bool attemptConnectToEdge(int roadType, RoadVertexDesc srcDesc, float dist_threshold, float z_threshold, float direction, float angle_threshold);

	bool attemptExpansion(int roadType, RoadVertexDesc srcDesc, int ex_id, ExFeature& f, std::vector<Patch> &patches, std::list<RoadVertexDesc> &seeds);

	void buildReplacementGraphByExample(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId);
	void buildReplacementGraphByExample2(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId, RoadVertexDesc v_connect, RoadVertexDesc v_root);
	void rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, std::list<RoadVertexDesc>& seeds);

	void attemptExpansion2(int roadType, RoadVertexDesc srcDesc, ExFeature& f, std::list<RoadVertexDesc> &seeds);
	bool growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature& f, float length, float angle, float curvature, int lanes, float angleTolerance, std::list<RoadVertexDesc> &seeds);
	bool extendRoadAcrossRiver(int roadType, RoadVertexDesc v_desc, std::list<RoadVertexDesc> &seeds, float max_length);

	void synthesizeItem(int roadType, RoadVertexDesc v_desc, float length, float step, std::vector<RoadEdgePtr> &edges);

	int defineExId(const QVector2D& pt);

	void check();

public:
};

