#pragma once

#include "Polygon2D.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "MainWindow.h"
//#include "Terrain.h"
#include "VBORenderManager.h"

class PatchWarp2RoadGenerator {
private:
	MainWindow *mainWin;
	RoadGraph &roads;
	Polygon2D targetArea;
	Polyline2D hintLine;
	VBORenderManager *vboRenderManager;
	std::vector<ExFeature> &features;

public:
	PatchWarp2RoadGenerator(MainWindow *mainWin, RoadGraph &roads, const Polygon2D &targetArea, const Polyline2D &hintLine, VBORenderManager *vboRenderManager, std::vector<ExFeature> &features) : mainWin(mainWin), roads(roads), targetArea(targetArea), hintLine(hintLine), vboRenderManager(vboRenderManager), features(features) {}
	~PatchWarp2RoadGenerator() {}

	void generateRoadNetwork();

private:
	void generateAvenueSeeds(std::list<RoadVertexDesc>& seeds);
	bool addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, float angle, std::list<RoadVertexDesc>& seeds);
	void generateStreetSeeds(std::list<RoadVertexDesc> &seeds);
	bool attemptExpansion(int roadType, RoadVertexDesc srcDesc, ExFeature &f, std::vector<RoadEdgeDescs> &shapes, std::list<RoadVertexDesc> &seeds);

	void buildReplacementGraphByExample(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, RoadEdgeDescs &shape);
	void rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, std::list<RoadVertexDesc>& seeds);
	bool isValidRule(RoadVertexDesc srcDesc, RoadGraph &replacementGraph);

	void attemptExpansion2(int roadType, RoadVertexDesc srcDesc, ExFeature &f, std::vector<RoadEdgeDescs> &shapes, std::list<RoadVertexDesc> &seeds);
	bool growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature &f, const Polyline2D &polyline, int lanes, RoadVertexDesc next_ex_v_desc, bool byExample, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds);

	std::pair<int, RoadVertexDesc> synthesizeItem(int roadType, RoadVertexDesc v_desc, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges, float &rotation_angle);

	//void replaceEdgeByExample(ExFeature &f, int roadType, RoadEdgePtr edge);
public:
};

