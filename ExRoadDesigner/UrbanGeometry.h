#pragma once

#include "glew.h"

#include "PolygonBuilder.h"
#include "PolylineBuilder.h"
#include "Terrain.h"
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"

class MainWindow;

class UrbanGeometry {
public:
	int width;
	int depth;
	MainWindow* mainWin;
	RoadGraph roads;
	
	//std::vector<Block*> blocks;
	Terrain* terrain;

	/*
	mylib::Renderer renderer;

	mylib::WaterRenderer* waterRenderer;
	*/
	PolygonBuilder areaBuilder;
	PolylineBuilder hintLineBuilder;
	PolylineBuilder avenueBuilder;

	RoadAreaSet areas;
	
	int selectedAreaIndex;
	//RendererHelper rendererHelper;

public:
	UrbanGeometry(MainWindow* mainWin);
	~UrbanGeometry();

	/** getter for width */
	int getWidth() { return width; }

	/** getter for depth */
	int getDepth() { return depth; }

	void clear();
	void clearGeometry();
	void generateRoadsMultiEx(std::vector<ExFeature> &features);
	void generateRoadsMultiIntEx(std::vector<ExFeature> &features);
	void generateRoadsInterpolation(ExFeature &feature);
	void generateRoadsWarp(ExFeature &feature);
	void generateRoadsSmoothWarp(ExFeature &feature);
	void generateRoadsVerySmoothWarp(ExFeature &feature);

	void render(VBORenderManager &vboRenderManager);
	void adaptToTerrain();

	void addRoad(int roadType, const Polyline2D &polyline, int lanes);
	void mergeRoads();
	void connectRoads();

	void newTerrain(int width, int depth, int cellLength);
	void loadTerrain(const QString &filename);
	void saveTerrain(const QString &filename);

	void loadRoads(const QString &filename);
	void saveRoads(const QString &filename);
	void clearRoads();

	void loadAreas(const QString &filename);
	void saveAreas(const QString &filename);

private:
};
