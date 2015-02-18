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
#include "RoadMeshGenerator.h"
#include "BlockMeshGenerator.h"
#include "VBOPm.h"
#include "VBOPmBlocks.h"
#include "VBOPmParcels.h"
#include "VBOPmBuildings.h"
#include "VBOVegetation.h"

UrbanGeometry::UrbanGeometry(MainWindow* mainWin) {
	this->mainWin = mainWin;

	//&mainWin->glWidget->vboRenderManager = NULL;

	//waterRenderer = new mylib::WaterRenderer(3000, 3000, -2.0f);

	//loadTerrain("../data/default.trn");

	//selectedAreaIndex = -1;
}

void UrbanGeometry::clear() {
	clearGeometry();
}

void UrbanGeometry::clearGeometry() {
	//if (&mainWin->glWidget->vboRenderManager != NULL) delete &mainWin->glWidget->vboRenderManager;

	roads.clear();
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateRoadsEx(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		PatchRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	} else {
		PatchRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	}
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateRoadsWarp(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		WarpRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	} else {
		WarpRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	}
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateRoadsPM(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		PMRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	} else {
		PMRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	}
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateRoadsAliaga(std::vector<ExFeature> &features) {
	if (areas.selectedIndex == -1) return;
	if (areas.selectedArea()->hintLine.size() == 0) return;

	if (G::getBool("useLayer")) {
		AliagaRoadGenerator generator(mainWin, areas.selectedArea()->roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	} else {
		AliagaRoadGenerator generator(mainWin, roads, areas.selectedArea()->area, areas.selectedArea()->hintLine, &mainWin->glWidget->vboRenderManager, features);
		generator.generateRoadNetwork();
	}
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateContourRoads() {
	cv::Mat grayImg;
	mainWin->glWidget->vboRenderManager.vboTerrain.layerData.convertTo(grayImg, CV_8UC1);
	cv::threshold(grayImg, grayImg, 60, 255, 0);

	cv::Mat element = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9, 9), cv::Point(7, 7));
	cv::erode(grayImg, grayImg, element);

	std::vector<std::vector<cv::Point> > contours;
	cv::imwrite("test.png", grayImg);
	cv::vector<cv::Vec4i> hierarchy;
	cv::findContours(grayImg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );

	std::vector<RoadVertexDesc> newVertices;

	for (int i = 0; i< contours.size(); ++i) {
		Polyline2D polyline;
		for (int j = 0; j < contours[i].size(); ++j) {
			float x = contours[i][j].x * mainWin->glWidget->vboRenderManager.side / mainWin->glWidget->vboRenderManager.vboTerrain.layerData.cols - mainWin->glWidget->vboRenderManager.side * 0.5;
			float y = contours[i][j].y * mainWin->glWidget->vboRenderManager.side / mainWin->glWidget->vboRenderManager.vboTerrain.layerData.rows - mainWin->glWidget->vboRenderManager.side * 0.5;

			polyline.push_back(QVector2D(x, y));
		}
		polyline.push_back(polyline[0]);

		Polyline2D polyline2;
		boost::geometry::simplify(polyline, polyline2, 20);
		polyline2 = GraphUtil::finerEdge(polyline2, 30);

		RoadVertexDesc prevDesc;
		for (int j = 0; j <= polyline2.size(); ++j) {
			int index = j;
			if (j == polyline2.size()) index = 0;

			RoadVertexDesc desc;
			if (!GraphUtil::getVertex(roads, QVector2D(polyline2[index].x(), polyline2[index].y()), 1.0f, desc)) {
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(QVector2D(polyline2[index].x(), polyline2[index].y())));
				v->type = RoadVertex::TYPE_CONTOUR;
				desc = GraphUtil::addVertex(roads, v);
				newVertices.push_back(desc);
			}

			if (j > 0) {
				if (((roads.graph[prevDesc]->pt.x() > -mainWin->glWidget->vboRenderManager.side * 0.5 + 40 && roads.graph[prevDesc]->pt.x() < mainWin->glWidget->vboRenderManager.side * 0.5 - 40) ||
					(roads.graph[desc]->pt.x() > -mainWin->glWidget->vboRenderManager.side * 0.5 + 40 && roads.graph[desc]->pt.x() < mainWin->glWidget->vboRenderManager.side * 0.5 - 40)) &&
					((roads.graph[prevDesc]->pt.y() > -mainWin->glWidget->vboRenderManager.side * 0.5 + 40 && roads.graph[prevDesc]->pt.y() < mainWin->glWidget->vboRenderManager.side * 0.5 - 40) ||
					(roads.graph[desc]->pt.y() > -mainWin->glWidget->vboRenderManager.side * 0.5 + 40 && roads.graph[desc]->pt.y() < mainWin->glWidget->vboRenderManager.side * 0.5 - 40))) {

					RoadEdgePtr e = RoadEdgePtr(new RoadEdge(RoadEdge::TYPE_AVENUE, 1));
					e->polyline.push_back(roads.graph[prevDesc]->pt);
					e->polyline.push_back(roads.graph[desc]->pt);
					GraphUtil::addEdge(roads, prevDesc, desc, e);
				}
			}

			prevDesc = desc;
		}
	}

	for (int i = 0; i < newVertices.size(); ++i) {
		int d = GraphUtil::getDegree(roads, newVertices[i]);
		if (d == 0) {
			roads.graph[newVertices[i]]->valid = false;
		} else if (d == 2) {
			std::vector<QVector2D> neighbor_points;
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(newVertices[i], roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				RoadVertexDesc tgt = boost::target(*ei, roads.graph);
				neighbor_points.push_back(roads.graph[tgt]->pt);
			}
			roads.graph[newVertices[i]]->pt = roads.graph[newVertices[i]]->pt * 6 / 8 + neighbor_points[0] / 8 + neighbor_points[1] / 8;
			for (boost::tie(ei, eend) = boost::out_edges(newVertices[i], roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, newVertices[i]);
				polyline[0] = roads.graph[newVertices[i]]->pt;
				roads.graph[*ei]->polyline = polyline;
			}
		}
	}
}

void UrbanGeometry::generateBlocks() {
	VBOPmBlocks::generateBlocks(&mainWin->glWidget->vboRenderManager, roads, blocks);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateParcels() {
	VBOPmParcels::generateParcels(mainWin->glWidget->vboRenderManager, blocks.blocks);
	VBOPmBuildings::generateBuildings(mainWin->glWidget->vboRenderManager, blocks.blocks);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateBuildings() {
	VBOPmBuildings::generateBuildings(mainWin->glWidget->vboRenderManager, blocks.blocks);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateVegetation() {
	VBOVegetation::generateVegetation(mainWin->glWidget->vboRenderManager, blocks.blocks);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::generateAll() {
	VBOPmBlocks::generateBlocks(&mainWin->glWidget->vboRenderManager, roads, blocks);
	VBOPmParcels::generateParcels(mainWin->glWidget->vboRenderManager, blocks.blocks);
	VBOPmBuildings::generateBuildings(mainWin->glWidget->vboRenderManager, blocks.blocks);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::render(VBORenderManager& vboRenderManager) {
	glLineWidth(5.0f);
	glPointSize(10.0f);

	// draw the area which is now being defined
	if (areaBuilder.selecting()) {
		areaBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, areaBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a hint polyline
	if (hintLineBuilder.selecting()) {
		hintLineBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, hintLineBuilder.polyline3D(), QColor(255, 0, 0));
	}

	// draw a sketching highways
	if (highwayBuilder.selecting()) {
		highwayBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, highwayBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a sketching avenues
	if (avenueBuilder.selecting()) {
		avenueBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, avenueBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a sketching streets
	if (streetBuilder.selecting()) {
		streetBuilder.adaptToTerrain(&vboRenderManager);
		RendererHelper::renderPolyline(vboRenderManager, streetBuilder.polyline3D(), QColor(0, 0, 255));
	}

	// draw a control points polyline
	if (controlPointsBuilder.selecting()) {
		controlPointsBuilder.adaptToTerrain(&vboRenderManager);
		vboRenderManager.addPolyline("lines", controlPointsBuilder.polyline3D(), QColor(255, 255, 0));
	}

	RendererHelper::renderPolyline(vboRenderManager, controlPoints3D, QColor(255, 255, 0));

	Polyline2D polyline = BSpline::spline(controlPoints, 10);
	Polyline3D polyline3;
	for (int i = 0; i < polyline.size(); ++i) {
		polyline3.push_back(QVector3D(polyline[i].x(), polyline[i].y(), 5));
	}
	RendererHelper::renderPolyline(vboRenderManager, polyline3, QColor(0, 255, 255));

	// draw the areas
	for (int i = 0; i < areas.size(); ++i) {
		areas[i]->adaptToTerrain(&vboRenderManager);
		QColor colorArea(0, 0, 255);
		QColor colorHintLine(255, 0, 0);
		if (i != areas.selectedIndex) {
			colorArea = QColor(196, 196, 255);
			colorHintLine = QColor(255, 196, 196);
		}

		// draw the area and the hint line
		RendererHelper::renderPolyline(vboRenderManager, areas[i]->area3D, colorArea);
		RendererHelper::renderPolyline(vboRenderManager, areas[i]->hintLine3D, colorHintLine);
	}

	vboRenderManager.removeStaticGeometry("lines");
}

/**
 * 道路、歩道、区画、ビル、木のジオミトリを作成しなおす。
 * この関数を頻繁に呼ぶべきではない。道路が更新/生成された時、2D/3D表示の変更、PMメニューから新規にジオミトリを生成した時だけ。
 */
void UrbanGeometry::update(VBORenderManager& vboRenderManager) {
	printf("updating geometry.\n");

	// 地面が変わっている可能性などがあるので、ビルなどのジオミトリも一旦削除してしまう。
	// 道路以外のジオミトリは、別途、PMメニューから作成すること
	vboRenderManager.removeStaticGeometry("3d_blocks");
	vboRenderManager.removeStaticGeometry("3d_parks");
	vboRenderManager.removeStaticGeometry("3d_parcels");
	vboRenderManager.removeStaticGeometry("3d_roads");
	vboRenderManager.removeStaticGeometry("3d_roads_inter");
	vboRenderManager.removeStaticGeometry("3d_roads_interCom");
	vboRenderManager.removeStaticGeometry("3d_building");
	vboRenderManager.removeStaticGeometry("3d_building_fac");
	vboRenderManager.removeStaticGeometry("tree");
	vboRenderManager.removeStaticGeometry("streetLamp");

	if (G::getBool("shader2D")) {
		RoadMeshGenerator::generate2DRoadMesh(vboRenderManager, roads);
		BlockMeshGenerator::generate2DParcelMesh(vboRenderManager, blocks);
	} else {
		RoadMeshGenerator::generateRoadMesh(vboRenderManager, roads);
		BlockMeshGenerator::generateBlockMesh(vboRenderManager, blocks);
		BlockMeshGenerator::generateParcelMesh(vboRenderManager, blocks);
		VBOPm::generateBuildings(mainWin->glWidget->vboRenderManager, blocks);
		VBOVegetation::generateVegetation(mainWin->glWidget->vboRenderManager, blocks.blocks);
	}

	for (int i = 0; i < areas.size(); ++i) {
		if (G::getBool("shader2D")) {
			RoadMeshGenerator::generate2DRoadMesh(vboRenderManager, areas[i]->roads);
		} else {
			RoadMeshGenerator::generateRoadMesh(vboRenderManager, areas[i]->roads);
		}
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

	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::cutRoads() {
	if (areas.selectedIndex < 0) return;
	GraphUtil::subtractRoads(roads, areas.selectedArea()->area, false);
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::loadRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	roads.clear();
	GraphUtil::loadRoads(roads, filename);

	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::addRoads(const QString &filename) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	RoadGraph addRoads;
	GraphUtil::loadRoads(addRoads, filename);
	GraphUtil::mergeRoads(roads, addRoads);

	update(mainWin->glWidget->vboRenderManager);
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
	update(mainWin->glWidget->vboRenderManager);
}

void UrbanGeometry::loadAreas(const QString &filename) {
	areas.load(filename);

	update(mainWin->glWidget->vboRenderManager);

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
