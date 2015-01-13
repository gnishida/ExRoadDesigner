#pragma once

#include "Polygon2D.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "MainWindow.h"
//#include "Terrain.h"
#include "VBORenderManager.h"

class PMRoadGenerator {
private:
	MainWindow *mainWin;
	RoadGraph &roads;
	Polygon2D targetArea;
	Polyline2D hintLine;
	VBORenderManager *vboRenderManager;
	std::vector<ExFeature> &features;

public:
	PMRoadGenerator(MainWindow *mainWin, RoadGraph &roads, const Polygon2D &targetArea, const Polyline2D &hintLine, VBORenderManager *vboRenderManager, std::vector<ExFeature> &features) : mainWin(mainWin), roads(roads), targetArea(targetArea), hintLine(hintLine), vboRenderManager(vboRenderManager), features(features) {}
	~PMRoadGenerator() {}

	void generateRoadNetwork();

private:
	void generateAvenueSeeds(std::list<RoadVertexDesc>& seeds);
	bool addAvenueSeed(const QVector2D &pt, int group_id, std::list<RoadVertexDesc>& seeds);
	void generateStreetSeeds(std::list<RoadVertexDesc> &seeds);
	void generateStreetSeeds2(std::list<RoadVertexDesc> &seeds);

	void attemptExpansion(int roadType, RoadVertexDesc srcDesc, std::list<RoadVertexDesc> &seeds);
	bool growRoadSegment(int roadType, RoadVertexDesc srcDesc, const Polyline2D &polyline, int lanes, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds);

	void synthesizeItem(int roadType, RoadVertexDesc v_desc, std::vector<RoadEdgePtr> &edges);
public:
};

