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

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <limits>
#include <QtTest/qtest.h>
#include <common/Util.h>
#include <common/Polygon3D.h>
#include <pm/Block.h>
#include "RoadGraph.h"
#include "BlockGenerator.h"
#include "MainWindow.h"
#include "UrbanGeometry.h"

RoadGraph* roadGraphPtrForBlockGeneration;
std::vector<Block*>* blocksPtr;
Polygon3D blockContourTmp;
Polygon3D blockContourPoints;
std::vector<Polyline3D> blockContourLines;
std::vector<float> blockContourWidths;

//Vertex visitor
struct vertex_output_visitor : public boost::planar_face_traversal_visitor {//output_visitor {
	void begin_face() {
		blockContourTmp.clear();
		blockContourWidths.clear();

		blockContourPoints.clear();
		blockContourLines.clear();
	}

	void end_face() {
		//if (blockContourPoints.size() > 2) {
			blockContourTmp.clear();
			
			for (int i = 0; i < blockContourPoints.size(); ++i) {
				blockContourTmp.push_back(blockContourPoints[i]);

				if ((blockContourLines[i][0] - blockContourPoints[i]).lengthSquared() < (blockContourLines[i].last() - blockContourPoints[i]).lengthSquared()) {
					for (int j = 1; j < blockContourLines[i].size() - 1; ++j) {
						blockContourTmp.push_back(blockContourLines[i][j]);
					}
				} else {
					for (int j = blockContourLines[i].size() - 2; j > 0; --j) {
						blockContourTmp.push_back(blockContourLines[i][j]);
					}
				}
			}

			//if (blockContourTmp.area() > 100.0f) {
			if (blockContourTmp.size() >= 3) {
				Block* newBlock = new Block();
				newBlock->setContour(blockContourTmp);
				newBlock->setRoadWidths(blockContourWidths);
	
				blocksPtr->push_back(newBlock);
			}
		//}
	}

	template <typename Vertex> 
	void next_vertex(Vertex v) {
		blockContourPoints.push_back(roadGraphPtrForBlockGeneration->graph[v]->pt);
	}

	template <typename Edge> 
	void next_edge(Edge e) {
		blockContourLines.push_back(roadGraphPtrForBlockGeneration->graph[e]->polyline3D);

		for (int i = 0; i < roadGraphPtrForBlockGeneration->graph[e]->polyline3D.size() - 1; ++i) {
			blockContourWidths.push_back(0.5f * roadGraphPtrForBlockGeneration->graph[e]->getWidth());
		}
	}
};

BlockGenerator::BlockGenerator(MainWindow* mainWin) {
	this->mainWin = mainWin;
}

BlockGenerator::~BlockGenerator() {
}

/**
* Given a road network, this function extracts the blocks
**/
void BlockGenerator::run() {
	RoadGraph* roads = &mainWin->urbanGeometry->roads;
	std::vector<Block*>* blocks = &mainWin->urbanGeometry->blocks;

	// clean up memory allocated for blocks
	for (int i = 0; i < blocks->size(); ++i) {
		delete blocks->at(i);
	}
	blocks->clear();

	roadGraphPtrForBlockGeneration = roads;
	blocksPtr = blocks;

	bool isPlanar = false;
	bool converges = true;

	//Make sure graph is planar
	typedef std::vector<RoadEdgeDesc > tEdgeDescriptorVector;
	std::vector<tEdgeDescriptorVector> embedding;//(boost::num_vertices(roads->graph));

	// Planarグラフにする
	/*
	int cont=0;
	while (!isPlanar && converges) {	
		if (cont>2) {
			return;
		}
		// Test for planarity		
		if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph = roads->graph, boost::boyer_myrvold_params::embedding = &embedding[0])) {
			isPlanar = true;
		} else {			
			//Remove intersecting edges
			if (!removeIntersectingEdges(roads)) {
				converges = false;
			}
		}
		cont++;
	}

	if (!isPlanar) {
		std::cout << "ERROR: Graph could not be planarized (generateBlocks)\n";
	}
	*/

	// embeddginを手動で作成
	buildEmbedding(*roads, embedding);

	//Create edge index property map?	
	typedef std::map<RoadEdgeDesc, size_t> EdgeIndexMap;
	EdgeIndexMap mapEdgeIdx;
	boost::associative_property_map<EdgeIndexMap> pmEdgeIndex(mapEdgeIdx);		
	RoadEdgeIter ei, ei_end;	
	int edge_count = 0;
	for (boost::tie(ei, ei_end) = boost::edges(roads->graph); ei != ei_end; ++ei) {
		mapEdgeIdx.insert(std::make_pair(*ei, edge_count++));	
	}

	//Extract blocks from road graph using boost graph planar_face_traversal
	vertex_output_visitor v_vis;
	boost::planar_face_traversal(roads->graph, &embedding[0], v_vis, pmEdgeIndex);

	// 最外郭のブロックを削除
	float max_area = 0.0f;
	int max_block_id = -1;
	for (int i = 0; i < blocks->size(); ++i) {
		float area = blocks->at(i)->getContour().area();
		if (area > max_area) {
			max_area = area;
			max_block_id = i;
		}
	}
	if (max_block_id >= 0) {
		blocks->erase(blocks->begin() + max_block_id);
	}

	// insetを計算する
	for (int i = 0; i < blocks->size(); ++i) {
		Polygon3D blockContourInset;
		blocks->at(i)->getContour().computeInset(blocks->at(i)->getRoadWidths(), blockContourInset);

		// 依然として、セルフ交差するケースがあるので、その場合はinsetしない
		// さもないと、セルフ交差したポリゴンはtesellationでエラー発生する
		if (!boost::geometry::intersects(blockContourInset)) {
			blocks->at(i)->setContour(blockContourInset);
		}
	}

	printf("generation done.\n");
}

/**
* Remove intersecting edges of a graph
**/
bool BlockGenerator::removeIntersectingEdges(RoadGraph* roadGraph) {
	/*
	std::vector<RoadGraphEdgeIter> edgesToRemove;

	QVector2D a0, a1, b0, b1;
	QVector2D intPt;
	RoadGraphEdgeIter a_ei, a_ei_end;
	RoadGraphEdgeIter b_ei, b_ei_end;
	float ta0a1, tb0b1;

	for (boost::tie(a_ei, a_ei_end) = boost::edges(roadGraph->getRoadGraph()); a_ei != a_ei_end; ++a_ei) {
		a0 = QVector2D(roadGraph->getSourceVertex(*a_ei)->getPt());
		a1 = QVector2D(roadGraph->getTargetVertex(*a_ei)->getPt());

		for (b_ei = a_ei; b_ei != a_ei_end; ++b_ei) {
			if (b_ei == a_ei) continue;

			b0 = QVector2D(roadGraph->getSourceVertex(*b_ei)->getPt());
			b1 = QVector2D(roadGraph->getTargetVertex(*b_ei)->getPt());

			if (ucore::Util::segmentSegmentIntersectXY(a0, a1, b0, b1, &ta0a1, &tb0b1, true, intPt) ){
				bool addEd=true;
				for (int eN = 0;eN < edgesToRemove.size(); eN++) {
					if (edgesToRemove[eN] == b_ei) {
						addEd = false;
						break;
					}
				}
				if(addEd)
					edgesToRemove.push_back(b_ei);
			}
		}		
	}
	
	for(int i=0; i<edgesToRemove.size(); ++i) {	
		boost::remove_edge(*(edgesToRemove[i]), roadGraph->getRoadGraph());
	}

	if(edgesToRemove.size()>0){
		return true;
	} else {
		return false;
	}
	*/

	return true;
}

void BlockGenerator::buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		std::map<float, RoadEdgeDesc> edges;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
			if ((roads.graph[*ei]->polyline[0] - roads.graph[*vi]->pt).lengthSquared() > (roads.graph[*ei]->polyline.last() - roads.graph[*vi]->pt).lengthSquared()) {
				std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
			}

			QVector2D vec = roads.graph[*ei]->polyline[1] - roads.graph[*ei]->polyline[0];
			edges[-atan2f(vec.y(), vec.x())] = *ei;
		}

		std::vector<RoadEdgeDesc> edge_descs;
		for (std::map<float, RoadEdgeDesc>::iterator it = edges.begin(); it != edges.end(); ++it) {
			edge_descs.push_back(it->second);
		}

		embedding.push_back(edge_descs);
	}
}

