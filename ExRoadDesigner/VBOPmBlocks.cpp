/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPmBlocks.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>
#include "GraphUtil.h"
#include "Util.h"
#include "VBOPmParcels.h"
#include "global.h"

///////////////////////////////////////////////////////////////
// BLOCKS
///////////////////////////////////////////////////////////////
RoadGraph * roadGraphPtr;
std::vector< Block > * blocksPtr;

Polygon3D sidewalkContourTmp;
Polygon2D sidewalkContourPoints;
std::vector<Polyline2D> sidewalkContourLines;

std::vector< float > sidewalkContourWidths;
bool isFirstVertexVisited;

int curRandSeed;
int curPlaceTypeIdx;

std::vector<RoadVertexDesc> visitedVs;

int face_index = 0;
bool vertex_output_visitor_invalid = false;

struct output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face()
	{
		face_index++;
		sidewalkContourTmp.clear();
		sidewalkContourWidths.clear();

		sidewalkContourPoints.clear();
		sidewalkContourLines.clear();

		vertex_output_visitor_invalid = false;
		visitedVs.clear();
	}

	void end_face()
	{
		if (vertex_output_visitor_invalid) {
			printf("INVALID end face\n");
			return;
		}

		sidewalkContourTmp.clear();
			
		for (int i = 0; i < sidewalkContourPoints.size(); ++i) {
			if (i >= sidewalkContourLines.size()) break;
			if (sidewalkContourLines[i].size() == 0) continue;
			sidewalkContourTmp.push_back(sidewalkContourPoints[i]);
			//sidewalkContourTmp.contour.back().setZ(0);//forze height =0

			if ((sidewalkContourLines[i][0] - sidewalkContourPoints[i]).lengthSquared() < (sidewalkContourLines[i].last() - sidewalkContourPoints[i]).lengthSquared()) {
				for (int j = 1; j < sidewalkContourLines[i].size() - 1; ++j) {
					sidewalkContourTmp.push_back(sidewalkContourLines[i][j]);
					//blockContourTmp.contour.back().setZ(0);//forze height =0
				}
			} else {
				for (int j = sidewalkContourLines[i].size() - 2; j > 0; --j) {
					sidewalkContourTmp.push_back(sidewalkContourLines[i][j]);
					//blockContourTmp.contour.back().setZ(0);//forze height =0
				}
			}
		}

		if (vertex_output_visitor_invalid){ 
			printf("INVALID end face\n");

			BBox3D bbox;
			sidewalkContourTmp.getBBox3D(bbox.minPt, bbox.maxPt);
			char filename[255];
			sprintf(filename, "block_images/block_%d.jpg", face_index);
			VBOPmBlocks::saveBlockImage(*roadGraphPtr, sidewalkContourTmp, filename);

			return;
		}

		//if (blockContourTmp.area() > 100.0f) {
		if (sidewalkContourTmp.contour.size() >= 3 && sidewalkContourWidths.size() >= 3) {
			Block newBlock;
			newBlock.sidewalkContour = sidewalkContourTmp;
			newBlock.sidewalkContourRoadsWidths = sidewalkContourWidths;
			while (newBlock.sidewalkContour.contour.size() > newBlock.sidewalkContourRoadsWidths.size()) {
				newBlock.sidewalkContourRoadsWidths.push_back(newBlock.sidewalkContourRoadsWidths.back());
			}
	
			blocksPtr->push_back(newBlock);
			//printf("CREATE block %d: %d\n",blocksPtr->size(),blocksPtr->back().blockContour.contour.size());
		}else{
			printf("Contour %d widths %d\n",sidewalkContourTmp.contour.size(),sidewalkContourWidths.size());
		}
	}
};

//Vertex visitor
struct vertex_output_visitor : public output_visitor
{
	template <typename Vertex> 
	void next_vertex(Vertex v) 
	{
		if (v >= boost::num_vertices(roadGraphPtr->graph)) {
			vertex_output_visitor_invalid = true;
			return;
		}

		if (!vertex_output_visitor_invalid) {
			visitedVs.push_back(v);
			if (v >= boost::num_vertices(roadGraphPtr->graph)) {
				vertex_output_visitor_invalid = true;
				printf("INVALID vertex\n");
				return;
			}
			sidewalkContourPoints.push_back(roadGraphPtr->graph[v]->pt);

		}
	}

	template <typename Edge> 
	void next_edge(Edge e) 
	{ 
		if (!vertex_output_visitor_invalid) {
			RoadVertexDesc src = boost::source(e, roadGraphPtr->graph);
			RoadVertexDesc tgt = boost::target(e, roadGraphPtr->graph);

			if (src == tgt) {
				vertex_output_visitor_invalid = true;
				return;
			}
			if (visitedVs.size() > 0 && src != visitedVs.back() && tgt != visitedVs.back()) {
				vertex_output_visitor_invalid = true;
				return;
			}
			if (src >= boost::num_vertices(roadGraphPtr->graph) || tgt >= boost::num_vertices(roadGraphPtr->graph)) {
				vertex_output_visitor_invalid = true;
				printf("INVALID edge\n");
				return;
			}

			sidewalkContourLines.push_back(roadGraphPtr->graph[e]->polyline);

			for (int i = 0; i < roadGraphPtr->graph[e]->polyline.size() - 1; ++i) {
				sidewalkContourWidths.push_back(0.5f * roadGraphPtr->graph[e]->getWidth());
			}
		}
	}
};

/**
* Remove intersecting edges.
*/
bool removeIntersectingEdges(RoadGraph &roadGraph) {
	std::vector<RoadEdgeIter> edgesToRemove;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roadGraph.graph); ei != eend; ++ei) {
		RoadEdgeIter ei2;
		for (ei2 = ei; ei2 != eend; ++ei2) {
			if (ei2 != ei) {
				if (GraphUtil::isIntersect(roadGraph, roadGraph.graph[*ei]->polyline, roadGraph.graph[*ei2]->polyline)) {
					if (std::find(edgesToRemove.begin(), edgesToRemove.end(), ei2) == edgesToRemove.end()) {
						edgesToRemove.push_back(ei2);

						RoadVertexDesc src = boost::source(*ei2, roadGraph.graph);
						RoadVertexDesc tgt = boost::target(*ei2, roadGraph.graph);
						printf("remove edge: (%lf, %lf) - (%lf, %lf)\n", roadGraph.graph[src]->pt.x(), roadGraph.graph[src]->pt.y(), roadGraph.graph[tgt]->pt.x(), roadGraph.graph[tgt]->pt.y());
					}
				}
			}
		}		
	}

	for(int i=0; i<edgesToRemove.size(); ++i){
		boost::remove_edge(*(edgesToRemove[i]), roadGraph.graph);
	}

	if(edgesToRemove.size()>0){
		printf("Edge removed %d\n", edgesToRemove.size());
		return true;
	} else {
		return false;
	}
}


/**
 * 道路網から、Block情報を抽出する。
 */
bool VBOPmBlocks::generateBlocks(VBORenderManager* renderManager, RoadGraph &roadGraph, BlockSet &blocks) {
	GraphUtil::normalizeLoop(roadGraph);

	roadGraphPtr = &roadGraph;
	blocksPtr = &blocks.blocks;
	blocksPtr->clear();

	bool isPlanar = false;
	bool converges = true;

	//GraphUtil::planarify(roadGraph);
	//GraphUtil::clean(roadGraph);
	
	//Make sure graph is planar
	typedef std::vector< RoadEdgeDesc > tEdgeDescriptorVector;
	std::vector<tEdgeDescriptorVector> embedding(boost::num_vertices(roadGraph.graph));

	int cont=0;

	/*
	// Test for planarity
	while (cont<2) {
		if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph =roadGraph.graph,
			boost::boyer_myrvold_params::embedding = &embedding[0]) 
			){
				isPlanar = true;
				break;
		} else {
			std::cout << "Input graph is not planar trying removeIntersectingEdges" << std::endl;
			// No planar: Remove intersecting edges and check again
			removeIntersectingEdges(roadGraph);
			cont++;
		}
	}

	if (!isPlanar) {
		std::cout << "ERROR: Graph could not be planarized: (generateBlocks)\n";
		return false;
	}
	*/
	
	// build embedding manually
	//embedding.clear();
	//embedding.resize(boost::num_vertices(roadGraph.graph));
	buildEmbedding(roadGraph, embedding);
	printf("embedding was built.\n");

	//Create edge index property map?	
	typedef std::map<RoadEdgeDesc, size_t> EdgeIndexMap;
	EdgeIndexMap mapEdgeIdx;
	boost::associative_property_map<EdgeIndexMap> pmEdgeIndex(mapEdgeIdx);		
	RoadEdgeIter ei, ei_end;	
	int edge_count = 0;
	for (boost::tie(ei, ei_end) = boost::edges(roadGraph.graph); ei != ei_end; ++ei) {
		mapEdgeIdx.insert(std::make_pair(*ei, edge_count++));	
	}

	//Extract blocks from road graph using boost graph planar_face_traversal
	vertex_output_visitor v_vis;	
	boost::planar_face_traversal(roadGraph.graph, &embedding[0], v_vis, pmEdgeIndex);

	printf("roads graph was traversed. %d blocks were extracted.\n", blocks.size());

	//Misc postprocessing operations on blocks =======
	int maxVtxCount = 0;
	int maxVtxCountIdx = -1;
	std::vector<float> blockAreas;

	Loop3D sidewalkContourInset;
	for (int i = 0; i < blocks.size(); ++i) {
		//Reorient faces
		if (Polygon3D::reorientFace(blocks[i].sidewalkContour.contour)) {
			std::reverse(blocks[i].sidewalkContourRoadsWidths.begin(), blocks[i].sidewalkContourRoadsWidths.end() - 1);
		}

		if( blocks[i].sidewalkContour.contour.size() != blocks[i].sidewalkContourRoadsWidths.size() ){
			std::cout << "Error: contour" << blocks[i].sidewalkContour.contour.size() << " widhts " << blocks[i].sidewalkContourRoadsWidths.size() << "\n";
			blocks[i].sidewalkContour.clear();
			blocks[i].valid = false;
			blockAreas.push_back(0.0f);
			continue;
		}

		if(blocks[i].sidewalkContour.contour.size() < 3){
			std::cout << "Error: Contour <3 " << "\n";
			blocks[i].valid = false;
			blockAreas.push_back(0.0f);
			continue;
		}

		//Compute block offset	
		float insetArea = blocks[i].sidewalkContour.computeInset(blocks[i].sidewalkContourRoadsWidths,sidewalkContourInset);
		
		blocks[i].sidewalkContour.contour = sidewalkContourInset;
		//blocks[i].sidewalkContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
		
		blockAreas.push_back(insetArea);
	}

	// Remove the largest block
	float maxArea = -FLT_MAX;
	int maxAreaIdx = -1;
	for (int i = 0; i < blocks.size(); ++i) {
		if (blocks[i].sidewalkContour.contour.size() < 3) {
			continue;
		}
		if (blockAreas[i] > maxArea) {
			maxArea = blockAreas[i];
			maxAreaIdx = i;
		}
	}
	if (maxAreaIdx != -1) {
		blocks[maxAreaIdx].valid = false;
		//blocks.blocks.erase(blocks.blocks.begin()+maxAreaIdx);
	}

	// GEN: remove the blocks whose edges are less than 3
	// This problem is caused by the computeInset() function.
	// ToDo: fix the computeInset function.
	for (int i = 0; i < blocks.size(); ++i) {
		if (blocks[i].sidewalkContour.contour.size() < 3) {
			blocks[i].valid = false;
			//blocks.blocks.erase(blocks.blocks.begin() + i);
		}
	}

	// assign a zone to each block
	generateSideWalk(renderManager, blocks);

	return true;
}

/*void VBOPmBlocks::buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding) {
	for (int i = 0; i < embedding.size(); ++i) {
		QMap<float, RoadEdgeDesc> edges;

		for (int j = 0; j < embedding[i].size(); ++j) {
			Polyline2D polyline = GraphUtil::orderPolyLine(roads, embedding[i][j], i);
			if ((polyline[0] - QVector2D(1817, 356)).length() < 2) {
				int xxx = 0;
			}



			QVector2D vec = polyline[1] - polyline[0];
			edges[-atan2f(vec.y(), vec.x())] = embedding[i][j];
		}

		std::vector<RoadEdgeDesc> edge_descs;
		for (QMap<float, RoadEdgeDesc>::iterator it = edges.begin(); it != edges.end(); ++it) {
			edge_descs.push_back(it.value());

			RoadEdgePtr e = roads.graph[it.value()];
			Polyline2D pl = e->polyline;
		}

		embedding[i] = edge_descs;
	}
}*/

void VBOPmBlocks::buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding) {
	embedding.clear();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		QMap<float, RoadEdgeDesc> edges;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
			Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, *vi);
			QVector2D vec = polyline[1] - polyline[0];
			edges[-atan2f(vec.y(), vec.x())] = *ei;
		}

		std::vector<RoadEdgeDesc> edge_descs;
		for (QMap<float, RoadEdgeDesc>::iterator it = edges.begin(); it != edges.end(); ++it) {
			edge_descs.push_back(it.value());

			RoadEdgePtr e = roads.graph[it.value()];
			Polyline2D pl = e->polyline;
		}

		embedding.push_back(edge_descs);
	}
}

/**
 * 全部のブロックに、ゾーンプランに基づいてゾーンタイプを割り当てる。
 * ゾーンタイプによって、各ブロックの歩道の幅も決まる。
 * なので、当然ながら、既存の区画は無効となる。
 * （現状、区画をクリアはしていない。クリアした方がよいか？）
 * 必要なら、この関数の後で、区画生成を実行してください。
 */
void VBOPmBlocks::generateSideWalk(VBORenderManager* renderManager, BlockSet& blocks) {
	for (int i = 0; i < blocks.size(); ++i) {
		if (!blocks[i].valid) continue;

		BBox3D bbox;
		blocks[i].sidewalkContour.getBBox3D(bbox.minPt, bbox.maxPt);

		// ブロックが細すぎる場合は、使用不可ブロックとする
		if (blocks[i].sidewalkContour.isTooNarrow(8.0f, 18.0f) || blocks[i].sidewalkContour.isTooNarrow(1.0f, 3.0f)) {
			blocks[i].valid = false;
			continue;
		}

		// Hack:
		// 円を公園にする
		/*if (SQR(bbox.midPt().x() + 968) + SQR(bbox.midPt().y() - 210) < 1000) {
			blocks[i].isPark = true;
			continue;
		}*/
		if (SQR(bbox.midPt().x() - 920) + SQR(bbox.midPt().y() + 500) < 1000) {
			blocks[i].isPark = true;
			continue;
		}
		if (SQR(bbox.midPt().x() - 1907) + SQR(bbox.midPt().y() + 506) < 1000) {
			blocks[i].isPark = true;
			continue;
		}

		// 高さが40m以下、または、高さの差が20m以上なら、使用不可ブロックとする
		float min_z = std::numeric_limits<float>::max();
		float max_z = 0.0f;
		for (int pi = 0; pi < blocks[i].sidewalkContour.contour.size(); ++pi) {
			int next_pi = (pi + 1) % blocks[i].sidewalkContour.contour.size();
			for (int k = 0; k <= 20; ++k) {
				QVector3D pt = blocks[i].sidewalkContour.contour[pi] * (float)(20 - k) * 0.05 + blocks[i].sidewalkContour.contour[next_pi] * (float)k * 0.05;
				float z = renderManager->getTerrainHeight(pt.x(), pt.y());
				min_z = std::min(min_z, z);
				max_z = std::max(max_z, z);
			}
			//float z = renderManager->getTerrainHeight(blocks[i].sidewalkContour.contour[pi].x(), blocks[i].sidewalkContour.contour[pi].y());
			//min_z = std::min(min_z, z);
			//max_z = std::max(max_z, z);
		}
		if (min_z < 40.0f) {
			blocks[i].valid = false;
			continue;
		} else if (max_z - min_z > 20.0f) {
			blocks[i].isPark = true;
			continue;
		}

		// 面積が大きすぎる場合は公園にする
		if (blocks[i].sidewalkContour.area() > 120000) {
			blocks[i].isPark = true;
			continue;
		}

	}

	// 歩道の分を確保するため、ブロックを縮小する。
	for (int i = 0; i < blocks.size(); ++i) {
		if (!blocks[i].valid) continue;
		//if (blocks[i].isPark) continue;

		Loop3D blockContourInset;
		float sidewalk_width = G::getFloat("sidewalk_width");
		blocks[i].sidewalkContour.computeInset(sidewalk_width, blockContourInset, false);
		blocks[i].blockContour.contour = blockContourInset;
		//blocks[i].blockContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);
	}
}

void VBOPmBlocks::saveBlockImage(RoadGraph& roads, Polygon3D& contour, const char* filename) {
	BBox bbox = GraphUtil::getAABoundingBox(roads, true);
	cv::Mat img(bbox.dy() + 1, bbox.dx() + 1, CV_8UC3, cv::Scalar(255, 255, 255));

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		for (int pl = 0; pl < roads.graph[*ei]->polyline.size() - 1; ++pl) {
			int x1 = roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x();
			int y1 = img.rows - (roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y());
			int x2 = roads.graph[*ei]->polyline[pl + 1].x() - bbox.minPt.x();
			int y2 = img.rows - (roads.graph[*ei]->polyline[pl + 1].y() - bbox.minPt.y());
			cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(224, 224, 224), 3);
		}
	}

	for (int i = 0; i < contour.contour.size(); ++i) {
		int next = (i + 1) % contour.contour.size();
		int x1 = contour.contour[i].x() - bbox.minPt.x();
		int y1 = img.rows - (contour.contour[i].y() - bbox.minPt.y());
		int x2 = contour.contour[next].x() - bbox.minPt.x();
		int y2 = img.rows - (contour.contour[next].y() - bbox.minPt.y());
		cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 0), 3);
	}

	cv::imwrite(filename, img);
}