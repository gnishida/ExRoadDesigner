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
#include "GraphUtil.h"
#include "PatchRoadGenerator.h"
#include "WarpRoadGenerator.h"
#include "PMRoadGenerator.h"
#include "AliagaRoadGenerator.h"
#include "RoadGeneratorHelper.h"
#include "MainWindow.h"
#include "Util.h"
#include "BSpline.h"

UrbanGeometry::UrbanGeometry(MainWindow* mainWin) {
	this->mainWin = mainWin;

	//&mainWin->glWidget->vboRenderManager = NULL;

	//waterRenderer = new mylib::WaterRenderer(3000, 3000, -2.0f);

	//loadTerrain("../data/default.trn");

	//selectedAreaIndex = -1;
}

UrbanGeometry::~UrbanGeometry() {
}

void UrbanGeometry::clear() {
	clearGeometry();
}

void UrbanGeometry::clearGeometry() {
	//if (&mainWin->glWidget->vboRenderManager != NULL) delete &mainWin->glWidget->vboRenderManager;

	roads.clear();

	// clean up memory allocated for blocks
	/*
	for (int i = 0; i < blocks.size(); ++i) {
		delete blocks[i];
	}
	blocks.clear();
	*/
}

void UrbanGeometry::generateRoadsEx(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		PatchRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		areas.selectedArea()->roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	} else {
		PatchRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}
}

void UrbanGeometry::generateRoadsWarp(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		WarpRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		areas.selectedArea()->roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	} else {
		WarpRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}
}

void UrbanGeometry::generateRoadsPM(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		PMRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		areas.selectedArea()->roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	} else {
		PMRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}
}

void UrbanGeometry::generateRoadsAliaga(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		AliagaRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		areas.selectedArea()->roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	} else {
		AliagaRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
		roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}
}

void UrbanGeometry::render(VBORenderManager& vboRenderManager) {
	glLineWidth(5.0f);
	glPointSize(10.0f);

	// draw the road graph
	roads.generateMesh(vboRenderManager, "roads_lines", "roads_points");
	vboRenderManager.renderStaticGeometry("roads_lines");
	vboRenderManager.renderStaticGeometry("roads_points");

	// draw the area which is now being defined
	if (areaBuilder.selecting()) {
		areaBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "area_builder_lines", "area_builder_points", areaBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a hint polyline
	if (hintLineBuilder.selecting()) {
		hintLineBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "hintline_builder_lines", "hintline_builder_points", hintLineBuilder.polyline3D(), QColor(255, 0, 0));
	}

	// draw a sketching highways
	if (highwayBuilder.selecting()) {
		highwayBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "sketch_highways_builder_lines", "sketch_highways_builder_points", highwayBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a sketching avenues
	if (avenueBuilder.selecting()) {
		avenueBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "sketch_avenues_builder_lines", "sketch_avenues_builder_points", avenueBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a sketching streets
	if (streetBuilder.selecting()) {
		streetBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "sketch_streets_builder_lines", "sketch_streets_builder_points", streetBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a control points polyline
	if (controlPointsBuilder.selecting()) {
		controlPointsBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, "controlpoints_builder_lines", "controlpoints_builder_points", controlPointsBuilder.polyline3D(), QColor(255, 255, 0));
	}

	RendererHelper::renderPolyline(vboRenderManager, "bspline_control_lines", "bspline_control_points", controlPoints3D, QColor(255, 255, 0));
	RendererHelper::renderPolyline(vboRenderManager, "bspline_lines", "bspline_points", BSpline::spline(controlPoints, 10), QColor(0, 255, 255), 5);

	// draw the detected circles
	for (int i = 0; i < circles.size(); ++i) {
		for (int j = 0; j < circles[i].size(); ++j) {
			QString strPointsN = QString("circle_points%1_%2").arg(i + 1).arg(j + 1);
			QString strLinesN = QString("circle_lines%1_%2").arg(i + 1).arg(j + 1);
			RendererHelper::renderPolyline(vboRenderManager, strLinesN, strPointsN, roads.graph[circles[i][j]]->polyline, QColor(0, 0, 255), 5);
		}
	}

	// draw the detected shapes
	for (int i = 0; i < shapes.size(); ++i) {
		for (int j= 0; j < shapes[i].size(); ++j) {
			QString strPointsN = QString("shape_points%1_%2").arg(i + 1).arg(j + 1);
			QString strLinesN = QString("shape_lines%1_%2").arg(i + 1).arg(j + 1);
			RendererHelper::renderPolyline(vboRenderManager, strLinesN, strPointsN, roads.graph[shapes[i][j]]->polyline, QColor(0, 0, 255), 5);
		}
	}

	// draw the areas
	for (int i = 0; i < areas.size(); ++i) {
		QString strLinesN = QString("area_lines%1").arg(i + 1);
		QString strPointsN = QString("area_points%1").arg(i + 1);

		QString strHintLinesN = QString("hint_lines%1").arg(i + 1);
		QString strHintPointsN = QString("hint_points%1").arg(i + 1);

		QColor colorArea(0, 0, 255);
		QColor colorHintLine(255, 0, 0);
		QColor colorControlPoints(255, 255, 0);
		QColor colorBSpline(0, 255, 255);
		if (i != areas.selectedIndex) {
			colorArea = QColor(196, 196, 255);
			colorHintLine = QColor(255, 196, 196);
		}

		// draw the area and the hint line
		RendererHelper::renderPolyline(vboRenderManager, strLinesN, strPointsN, areas[i]->area3D, colorArea);
		RendererHelper::renderPolyline(vboRenderManager, strHintLinesN, strHintPointsN, areas[i]->hintLine3D, colorHintLine);

		// draw the road graph
		//areas[i]->roads.adaptToTerrain(vboRenderManager);
		QString str1 = QString("roads_lines%1").arg(i + 1);
		QString str2 = QString("roads_points%1").arg(i + 1);
		areas[i]->roads.generateMesh(vboRenderManager, str1, str2);
		vboRenderManager.renderStaticGeometry(str1);
		vboRenderManager.renderStaticGeometry(str2);
	}
}

/**
 * Adapt all geometry objects to &mainWin->glWidget->vboRenderManager.
 */
void UrbanGeometry::adaptToTerrain() {
	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);

	for (int i = 0; i < areas.size(); ++i) {
		areas[i]->adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}

	controlPoints3D.clear();
	for (int i = 0; i < controlPoints.size(); ++i) {
		float z = mainWin->glWidget->vboRenderManager.getTerrainHeight(controlPoints[i].x(), controlPoints[i].y());
		controlPoints3D.push_back(QVector3D(controlPoints[i].x(), controlPoints[i].y(), z + 30));
	}
}

/**
 * add a road edge
 */
void UrbanGeometry::addRoad(int roadType, Polyline2D &polyline, int lanes) {
	// fix all the existing road segments
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			roads.graph[*vi]->fixed = true;
		}
	}

	RoadVertexDesc v1_desc;
	if (!GraphUtil::getVertex(roads, polyline[0], 10.0f, v1_desc)) {
		RoadVertexPtr v1 = RoadVertexPtr(new RoadVertex(polyline[0]));
		v1_desc = boost::add_vertex(roads.graph);
		roads.graph[v1_desc] = v1;
		polyline[0] = roads.graph[v1_desc]->pt;
	}

	RoadVertexDesc v2_desc;
	if (!GraphUtil::getVertex(roads, polyline.back(), 10.0f, v2_desc)) {
		RoadVertexPtr v2 = RoadVertexPtr(new RoadVertex(polyline.last()));
		v2_desc = boost::add_vertex(roads.graph);
		roads.graph[v2_desc] = v2;
		polyline.back() = roads.graph[v2_desc]->pt;
	}

	RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, v1_desc, v2_desc, roadType, lanes);
	roads.graph[e_desc]->polyline = polyline;

	/*
	if (roadType == RoadEdge::TYPE_AVENUE || roadType == RoadEdge::TYPE_STREET) {
		GraphUtil::planarify(roads);

		// remove too short edges
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);
			if (roads.graph[src]->fixed || roads.graph[tgt]->fixed) continue;

			if (roadType == RoadEdge::TYPE_AVENUE) {
				if (roads.graph[*ei]->polyline.length() < 100) roads.graph[*ei]->valid = false;
			} else {
				if (roads.graph[*ei]->polyline.length() < 20) roads.graph[*ei]->valid = false;
			}
		}
	}
	*/

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
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
	//RoadGeneratorHelper::connectRoads(roads, 200.0f, 0.15f);
	RoadGeneratorHelper::connectRoads2(roads, 400.0f);
	GraphUtil::removeDeadEnd(roads);

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::cutRoads() {
	if (areas.selectedIndex < 0) return;
	GraphUtil::subtractRoads(roads, areas.selectedArea()->area, false);
}

void UrbanGeometry::loadRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	roads.clear();
	circles.clear();
	shapes.clear();
	GraphUtil::loadRoads(roads, filename);

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::addRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	circles.clear();
	shapes.clear();

	RoadGraph addRoads;
	GraphUtil::loadRoads(addRoads, filename);
	GraphUtil::mergeRoads(roads, addRoads);

	roads.adaptToTerrain(&mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::saveRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	GraphUtil::saveRoads(roads, filename);
}

void UrbanGeometry::clearRoads() {
	roads.clear();
	circles.clear();
	shapes.clear();
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
		areas[i]->adaptToTerrain(&mainWin->glWidget->vboRenderManager);
	}

	areas.selectedIndex = 0;
}

void UrbanGeometry::saveAreas(const QString &filename) {
	areas.save(filename);
}

void UrbanGeometry::debug() {
	std::cout << "debug" << std::endl;
	bool found = false;
	RoadEdgeDesc closeEdge1;
	RoadEdgeDesc closeEdge2;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = vertices(roads.graph); vi != vend && !found; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = out_edges(*vi, roads.graph); ei != eend && !found; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);
			if (src == tgt) continue;

			Polyline2D polyline = roads.graph[*ei]->polyline;
			if ((polyline[0] - roads.graph[*vi]->pt).lengthSquared() > (polyline.back() - roads.graph[*vi]->pt).lengthSquared()) {
				std::reverse(polyline.begin(), polyline.end());
			}

			RoadOutEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = out_edges(*vi, roads.graph); ei2 != eend2; ++ei2) {
				if (*ei == *ei2) continue;
				if (!roads.graph[*ei]->valid) continue;

				RoadVertexDesc src2 = boost::source(*ei2, roads.graph);
				RoadVertexDesc tgt2 = boost::target(*ei2, roads.graph);
				if (src2 == tgt2) continue;

				Polyline2D polyline2 = roads.graph[*ei2]->polyline;
				if ((polyline2[0] - roads.graph[*vi]->pt).lengthSquared() > (polyline2.back() - roads.graph[*vi]->pt).lengthSquared()) {
					std::reverse(polyline2.begin(), polyline2.end());
				}

				if (Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]) < 0.1f) {
					found = true;

					closeEdge1 = *ei;
					closeEdge2 = *ei2;

					break;
				}
			}
		}
	}

	if (found) {
		RoadVertexDesc src = boost::source(closeEdge1, roads.graph);
		RoadVertexDesc tgt = boost::target(closeEdge1, roads.graph);
		RoadVertexDesc src2 = boost::source(closeEdge2, roads.graph);
		RoadVertexDesc tgt2 = boost::target(closeEdge2, roads.graph);
		std::cout << "Too close edges are found!!!! " << src << "," << tgt << " : " << src2 << "," << tgt2 << std::endl;
		for (int i = 0; i < roads.graph[closeEdge1]->polyline.size(); ++i) {
			std::cout << roads.graph[closeEdge1]->polyline[i].x() << "," << roads.graph[closeEdge1]->polyline[i].y() << std::endl;
		}
		std::cout << std::endl;
		for (int i = 0; i < roads.graph[closeEdge2]->polyline.size(); ++i) {
			std::cout << roads.graph[closeEdge2]->polyline[i].x() << "," << roads.graph[closeEdge2]->polyline[i].y() << std::endl;
		}

		mainWin->glWidget->edgeSelected = true;
		mainWin->glWidget->selectedEdge = roads.graph[closeEdge1];
		mainWin->glWidget->selectedEdgeDesc = closeEdge1;
		mainWin->glWidget->updateGL();

	} else {
		std::cout << "No close edge is found." << std::endl;
	}
}
