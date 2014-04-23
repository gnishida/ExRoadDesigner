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

#include "UrbanGeometry.h"
#include <limits>
#include <iostream>
#include <QFile>
#include "common.h"
#include "global.h"
#include "RendererHelper.h"
/*
#include <render/WaterRenderer.h>
#include <render/TextureManager.h>
#include <render/Texture.h>
#include <render/GeometryObject.h>
#include <render/Terrain.h>
*/
#include "GraphUtil.h"
#include "MultiExRoadGenerator.h"
#include "MultiIntExRoadGenerator.h"
#include "IntRoadGenerator.h"
#include "WarpRoadGenerator.h"
#include "SmoothWarpRoadGenerator.h"
#include "VerySmoothWarpRoadGenerator.h"
#include "RoadGeneratorHelper.h"
#include "MainWindow.h"
/*
#include "BlockGenerator.h"
#include "ParcelGenerator.h"
*/

UrbanGeometry::UrbanGeometry(MainWindow* mainWin) {
	this->mainWin = mainWin;

	//terrain = NULL;

	//waterRenderer = new mylib::WaterRenderer(3000, 3000, -2.0f);

	//loadTerrain("data/default.trn");

	selectedAreaIndex = -1;
}

UrbanGeometry::~UrbanGeometry() {
}

void UrbanGeometry::clear() {
	clearGeometry();
}

void UrbanGeometry::clearGeometry() {
	//if (terrain != NULL) delete terrain;

	roads.clear();

	// clean up memory allocated for blocks
	/*
	for (int i = 0; i < blocks.size(); ++i) {
		delete blocks[i];
	}
	blocks.clear();
	*/
}

void UrbanGeometry::generateRoadsMultiEx(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	MultiExRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, features);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateRoadsMultiIntEx(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	MultiIntExRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, features);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateRoadsInterpolation(ExFeature &feature) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	IntRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, feature);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateRoadsWarp(ExFeature &feature) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	WarpRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, feature);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateRoadsSmoothWarp(ExFeature &feature) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	SmoothWarpRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, feature);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateRoadsVerySmoothWarp(ExFeature &feature) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	VerySmoothWarpRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, feature);
	generator.generateRoadNetwork(G::getBool("animation"));

	areas.selectedArea()->roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::generateBlocks() {
	/*
	BlockGenerator generator(mainWin);
	generator.run();

	for (int i = 0; i < blocks.size(); ++i) {
		blocks[i]->adaptToTerrain(terrain);
	}
	*/
}

void UrbanGeometry::generateParcels() {
	/*
	ParcelGenerator generator(mainWin);
	generator.run();

	for (int i = 0; i < blocks.size(); ++i) {
		blocks[i]->adaptToTerrain(terrain);
	}
	*/
}

void UrbanGeometry::render(VBORenderManager &vboRenderManager) {
	// draw the road graph
	roads.generateMesh(vboRenderManager,"roadGraph");
	vboRenderManager.renderStaticGeometry("roadGraph");

	/*
	if (waterRenderer != NULL) {
		waterRenderer->renderMe(textureManager);
	}
	
	// draw a terrain
	renderer.render(terrain, textureManager);

	// draw blocks and parcels
	for (int i = 0; i < blocks.size(); ++i) {
		renderer.render(blocks[i], textureManager);

		ParcelGraphVertexIter vi, viEnd;
		for(boost::tie(vi, viEnd) = boost::vertices(blocks[i]->parcels); vi != viEnd; ++vi) {
			renderer.render(blocks[i]->parcels[*vi], textureManager);
		}
	}
	*/

	// draw the area which is now being defined
	if (areaBuilder.selecting()) {
		areaBuilder.adaptToTerrain();//terrain);
		RendererHelper::renderPolyline(vboRenderManager, "area_builder_lines", "area_builder_points", areaBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a hint polyline
	if (hintLineBuilder.selecting()) {
		hintLineBuilder.adaptToTerrain();//terrain);
		RendererHelper::renderPolyline(vboRenderManager, "hintline_builder_lines", "hintline_builder_points", hintLineBuilder.polyline3D(), QColor(255, 0, 0));
	}

	// draw a avenue sketch polyline
	/*
	if (avenueBuilder.selecting()) {
		avenueBuilder.adaptToTerrain(terrain);
		rendererHelper.renderPolyline(avenueBuilder.polyline3D(), QColor(255, 255, 0), GL_LINE_STIPPLE);
	}
	*/

	// draw the areas
	for (int i = 0; i < areas.size(); ++i) {
		QString strLinesN = QString("area_lines%1").arg(i + 1);
		QString strPointsN = QString("area_points%1").arg(i + 1);

		QString strHintLinesN = QString("hint_lines%1").arg(i + 1);
		QString strHintPointsN = QString("hint_points%1").arg(i + 1);

		QColor colorArea(0, 0, 255);
		QColor colorHintLine(255, 0, 0);
		if (i != areas.selectedIndex) {
			colorArea = QColor(196, 196, 255);
			colorHintLine = QColor(255, 196, 196);
		}

		// draw the area and the hint line
		RendererHelper::renderPolyline(vboRenderManager, strLinesN, strPointsN, areas[i]->area3D, colorArea);
		RendererHelper::renderPolyline(vboRenderManager, strHintLinesN, strHintPointsN, areas[i]->hintLine3D, colorHintLine);

		// draw the road graph
		//areas[i]->roads.adaptToTerrain(terrain);
		QString str = QString("roadGraph%1").arg(i + 1);
		areas[i]->roads.generateMesh(vboRenderManager,str);
		vboRenderManager.renderStaticGeometry(str);
	}
}

/**
 * Adapt all geometry objects to terrain.
 */
void UrbanGeometry::adaptToTerrain() {
	roads.adaptToTerrain();//terrain);
}

/**
 * 指定された道路を追加する。
 */
void UrbanGeometry::addRoad(int roadType, const Polyline2D &polyline, int lanes) {
	RoadVertexDesc v1_desc;
	if (!GraphUtil::getVertex(roads, polyline[0], 10.0f, v1_desc)) {
		RoadVertexPtr v1 = RoadVertexPtr(new RoadVertex(polyline[0]));
		v1_desc = boost::add_vertex(roads.graph);
		roads.graph[v1_desc] = v1;
	}

	RoadVertexDesc v2_desc;
	if (!GraphUtil::getVertex(roads, polyline.last(), 10.0f, v2_desc)) {
		RoadVertexPtr v2 = RoadVertexPtr(new RoadVertex(polyline.last()));
		v2_desc = boost::add_vertex(roads.graph);
		roads.graph[v2_desc] = v2;
	}

	RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, v1_desc, v2_desc, roadType, lanes);
	roads.graph[e_desc]->polyline = polyline;

	GraphUtil::planarify(roads);

	roads.adaptToTerrain();//terrain);
}

void UrbanGeometry::mergeRoads() {
	// もともとある道路の頂点は、すべてfixedにしておく
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		roads.graph[*vi]->fixed = true;
	}

	areas.mergeRoads();
	GraphUtil::mergeRoads(roads, areas.roads);

	areas.roads.clear();
	areas.selectedIndex = -1;
	areas.clear();
}

void UrbanGeometry::connectRoads() {
	RoadGeneratorHelper::connectRoads(roads, 200.0f, 0.15f);
	GraphUtil::removeDeadEnd(roads);

	roads.adaptToTerrain();//terrain);
}

/*
void UrbanGeometry::newTerrain(int width, int depth, int cellLength) {
	clear();

	terrain = new mylib::Terrain(width, depth, cellLength);

	if (waterRenderer != NULL) {
		waterRenderer->setWidth(width);
		waterRenderer->setDepth(depth);
	}
}

void UrbanGeometry::loadTerrain(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "MyUrbanGeometry::loadInfoLayers... The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	clear();

	QTextStream in(&file);
	QString line = in.readLine();
	this->width = line.split(" ")[0].toInt();
	this->depth = line.split(" ")[1].toInt();
	int cellLength = line.split(" ")[2].toInt();

	terrain = new mylib::Terrain(width, depth, cellLength);

	for (int i = 0; i < terrain->getNumRows() * terrain->getNumCols(); ++i) {
		line = in.readLine();
		int idxX = line.split(" ").at(1).toInt();
		int idxY = line.split(" ").at(2).toInt();

		line = in.readLine();
		float x = line.split(" ").at(0).toFloat();
		float y = line.split(" ").at(1).toFloat();

		line = in.readLine();
		terrain->setValue(x, y, line.toFloat());
	}

	if (waterRenderer != NULL) {
		waterRenderer->setWidth(width);// * 1.2f);
		waterRenderer->setDepth(depth);// * 1.2f);
	}
}

void UrbanGeometry::saveTerrain(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "MyUrbanGeometry::saveInfoLayers... The file is not writable: " << filename.toUtf8().constData() << endl;
		throw "The file is not writable: " + filename;
	}

	QTextStream out(&file);
	out << terrain->width << " " << terrain->depth << " " << terrain->getCellLength() << endl;
	int count = 0;
	for (int i = 0; i < terrain->getNumCols(); ++i) {
		for (int j = 0; j < terrain->getNumRows(); ++j) {
			out << count++ << " " << i << " " << j << endl;
			float x = terrain->getCell(i, j).getX();
			float y = terrain->getCell(i, j).getY();

			out << x << " " << y << endl;

			out << terrain->getValue(x, y) << endl;
		}
	}
}
*/

void UrbanGeometry::loadRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "MyUrbanGeometry::loadInfoLayers... The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	roads.clear();
	GraphUtil::loadRoads(roads, filename);

	roads.adaptToTerrain();
}

void UrbanGeometry::saveRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "MyUrbanGeometry::loadInfoLayers... The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	GraphUtil::saveRoads(roads, filename);
}

void UrbanGeometry::clearRoads() {
	roads.clear();
	/*
	for (int i = 0; i < blocks.size(); ++i) {
		delete blocks[i];
	}
	blocks.clear();
	*/
}

void UrbanGeometry::loadAreas(const QString &filename) {
	areas.load(filename);

	for (int i = 0; i < areas.size(); ++i) {
		areas[i]->adaptToTerrain();//terrain);
	}
}

void UrbanGeometry::saveAreas(const QString &filename) {
	areas.save(filename);
}
