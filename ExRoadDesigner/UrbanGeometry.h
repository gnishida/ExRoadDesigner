/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#pragma once

#include "glew.h"

#include "PolygonBuilder.h"
#include "PolylineBuilder.h"
/*
#include <render/TextureManager.h>
#include <render/WaterRenderer.h>
#include <render/Terrain.h>
#include <render/Renderer.h>
#include "RendererHelper.h"
#include <render/Renderer.h>
*/
#include "RoadGraph.h"
#include "RoadAreaSet.h"
#include "ExFeature.h"
//#include <pm/Block.h>

class MainWindow;

class UrbanGeometry {
public:
	int width;
	int depth;
	MainWindow* mainWin;
	RoadGraph roads;
	/*
	std::vector<Block*> blocks;
	mylib::Terrain* terrain;

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
	void generateRoadsUShape(ExFeature &feature);
	void generateRoadsWarp(ExFeature &feature);
	void generateRoadsSmoothWarp(ExFeature &feature);
	void generateRoadsVerySmoothWarp(ExFeature &feature);
	void generateBlocks();
	void generateParcels();

	void render(VBORenderManager &vboRenderManager);
	void adaptToTerrain();

	void addRoad(int roadType, const Polyline2D &polyline, int lanes);
	void mergeRoads();
	/*
	void connectRoads();

	void newTerrain(int width, int depth, int cellLength);
	void loadTerrain(const QString &filename);
	void saveTerrain(const QString &filename);
	*/
	void loadRoads(const QString &filename);
	void saveRoads(const QString &filename);
	void clearRoads();

	/*
	void perturbRoads(float perturbationFactor);
	*/

	void loadAreas(const QString &filename);
	void saveAreas(const QString &filename);

private:
};
