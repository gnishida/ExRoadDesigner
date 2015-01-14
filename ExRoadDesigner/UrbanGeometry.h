#pragma once

#include "glew.h"

#include "PolygonBuilder.h"
#include "PolylineBuilder.h"
//#include "Terrain.h"
#include "VBORenderManager.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
#include "CircleHoughTransform.h"

class MainWindow;

class UrbanGeometry {
public:
	int width;
	int depth;
	MainWindow* mainWin;
	RoadGraph roads;
	std::vector<RoadEdgeDescs> circles;
	std::vector<RoadEdgeDescs> shapes;
	
	//std::vector<Block*> blocks;
	//Terrain* vboRenderManager;

	PolygonBuilder areaBuilder;
	PolylineBuilder hintLineBuilder;
	PolylineBuilder highwayBuilder;
	PolylineBuilder avenueBuilder;
	PolylineBuilder streetBuilder;
	PolylineBuilder controlPointsBuilder;

	Polyline2D controlPoints;
	Polyline3D controlPoints3D;
	Polyline2D origControlPoints;
	bool controlPointSelected;
	int controlPointSelectedIndex;
	RoadGraph origRoads;

	RoadAreaSet areas;
	
	//int selectedAreaIndex;

public:
	UrbanGeometry(MainWindow* mainWin);
	~UrbanGeometry();

	/** getter for width */
	int getWidth() { return width; }

	/** getter for depth */
	int getDepth() { return depth; }

	void clear();
	void clearGeometry();
	void generateRoadsEx(std::vector<ExFeature> &features);
	void generateRoadsWarp(std::vector<ExFeature> &features);
	void generateRoadsPM(std::vector<ExFeature> &features);
	void generateRoadsAliaga(std::vector<ExFeature> &features);

	void render(VBORenderManager &vboRenderManager);
	void adaptToTerrain();

	void addRoad(int roadType, Polyline2D &polyline, int lanes);
	void mergeRoads();
	void connectRoads();
	void cutRoads();

	void newTerrain(int width, int depth, int cellLength);
	void loadTerrain(const QString &filename);
	void saveTerrain(const QString &filename);

	void loadRoads(const QString &filename);
	void addRoads(const QString &filename);
	void saveRoads(const QString &filename);
	void clearRoads();

	void loadAreas(const QString &filename);
	void saveAreas(const QString &filename);

	void debug();

private:
};
