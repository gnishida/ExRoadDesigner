#pragma once

#include "Polygon2D.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "MainWindow.h"
//#include "Terrain.h"
#include "VBORenderManager.h"

class WarpRoadGenerator {
private:
	MainWindow *mainWin;
	RoadGraph &roads;
	Polygon2D targetArea;
	Polyline2D hintLine;
	VBORenderManager *vboRenderManager;
	std::vector<ExFeature> &features;

public:
	WarpRoadGenerator(MainWindow *mainWin, RoadGraph &roads, const Polygon2D &targetArea, const Polyline2D &hintLine, VBORenderManager *vboRenderManager, std::vector<ExFeature> &features) : mainWin(mainWin), roads(roads), targetArea(targetArea), hintLine(hintLine), vboRenderManager(vboRenderManager), features(features) {}
	~WarpRoadGenerator() {}

	void generateRoadNetwork();

private:
	void generateAvenueSeeds(std::list<RoadVertexDesc>& seeds);
	bool addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, float angle, std::list<RoadVertexDesc>& seeds);
	void generateStreetSeeds(std::list<RoadVertexDesc> &seeds);

	bool attemptConnect(int roadType, RoadVertexDesc srcDesc, int ex_id, std::vector<ExFeature>& features);
	bool attemptConnectToVertex(int roadType, RoadVertexDesc srcDesc, std::vector<ExFeature>& features, float dist_threshold, float z_threshold, float direction, float angle_threshold, float roadAngleTolerance);
	bool attemptConnectToEdge(int roadType, RoadVertexDesc srcDesc, float dist_threshold, float z_threshold, float direction, float angle_threshold, float roadAngleTolerance);

	bool attemptExpansion(int roadType, RoadVertexDesc srcDesc, int ex_id, ExFeature& f, std::vector<Patch> &patches, std::list<RoadVertexDesc> &seeds);

	void buildReplacementGraphByExample(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId);
	void buildReplacementGraphByExample2(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId, RoadVertexDesc v_connect, RoadVertexDesc v_root);
	void rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, int ex_id, std::list<RoadVertexDesc>& seeds);

	void attemptExpansion2(int roadType, RoadVertexDesc srcDesc, int ex_id, ExFeature &f, std::list<RoadVertexDesc> &seeds);
	bool growRoadSegment(int roadType, RoadVertexDesc srcDesc, float step, int num_steps, float angle, float curvature, int lanes, float angleTolerance, int ex_id, std::list<RoadVertexDesc> &seeds);

	int defineExId(const QVector2D& pt);
};

