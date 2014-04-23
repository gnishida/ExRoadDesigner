#pragma once

#include "Polygon2D.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "MainWindow.h"

class MultiExRoadGenerator {
private:
	MainWindow *mainWin;
	RoadGraph &roads;
	Polygon2D targetArea;
	Polyline2D hintLine;
	//mylib::Terrain *terrain;
	std::vector<ExFeature> &features;
	bool animation;

public:
	MultiExRoadGenerator(MainWindow *mainWin, RoadGraph &roads, const Polygon2D &targetArea, const Polyline2D &hintLine, std::vector<ExFeature> &features) : mainWin(mainWin), roads(roads), targetArea(targetArea), hintLine(hintLine), features(features) {}
	~MultiExRoadGenerator() {}

	void generateRoadNetwork(bool animation = false);

private:
	void generateAvenueSeeds(std::list<RoadVertexDesc>& seeds);
	bool addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, std::list<RoadVertexDesc>& seeds);
	void generateStreetSeeds(std::list<RoadVertexDesc> &seeds);
	void attemptExpansion(int roadType, RoadVertexDesc srcDesc, ExFeature& f, std::list<RoadVertexDesc> &seeds);
	void attemptExpansion2(int roadType, RoadVertexDesc srcDesc, ExFeature& f, std::list<RoadVertexDesc> &seeds);
	bool growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature& f, const Polyline2D &polyline, int lanes, RoadVertexDesc next_ex_v_desc, bool byExample, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds);

	void synthesizeItem(int roadType, RoadVertexDesc v_desc, ExFeature &f, std::vector<RoadEdgePtr> &edges);

public:
};

