#include "SmallBlockRemover.h"
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include "Polygon2D.h"
#include "GraphUtil.h"
#include "Util.h"

RoadGraph * sbrRoadsPtr;
std::vector< Polygon2D > * sbrBlocksPtr;
std::vector<std::vector<RoadEdgeDesc> > * sbrBlocksEdges;
//Loop3D blockContourTmp;

Polygon2D sbrBlockContourTmp;
Polygon2D sbrBlockContourPoints;
std::vector<Polyline2D> sbrBlockContourLines;
std::vector<RoadEdgeDesc> sbrBlockContourEdges;


//int face_index = 0;

struct sbr_output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face()
	{
		//std::cout << "face: " << face_index << std::endl;
		//face_index++;

		sbrBlockContourTmp.clear();

		sbrBlockContourPoints.clear();
		sbrBlockContourLines.clear();
		sbrBlockContourEdges.clear();
	}

	void end_face()
	{
		sbrBlockContourTmp.clear();
			
		for (int i = 0; i < sbrBlockContourPoints.size(); ++i) {
			sbrBlockContourTmp.push_back(sbrBlockContourPoints[i]);

			if ((sbrBlockContourLines[i][0] - sbrBlockContourPoints[i]).lengthSquared() < (sbrBlockContourLines[i].last() - sbrBlockContourPoints[i]).lengthSquared()) {
				for (int j = 1; j < sbrBlockContourLines[i].size() - 1; ++j) {
					sbrBlockContourTmp.push_back(sbrBlockContourLines[i][j]);
				}
			} else {
				for (int j = sbrBlockContourLines[i].size() - 2; j > 0; --j) {
					sbrBlockContourTmp.push_back(sbrBlockContourLines[i][j]);
				}
			}
		}

		//if (blockContourTmp.area() > 100.0f) {
		if (sbrBlockContourTmp.size() >= 3) {
			sbrBlocksEdges->push_back(sbrBlockContourEdges);	
			sbrBlocksPtr->push_back(sbrBlockContourTmp);
		}
	}
};

//Vertex visitor
struct sbr_vertex_output_visitor : public sbr_output_visitor
{
	template <typename Vertex> 
	void next_vertex(Vertex v) 
	{ 	
		sbrBlockContourPoints.push_back(sbrRoadsPtr->graph[v]->pt);
	}

	template <typename Edge> 
	void next_edge(Edge e) 
	{
		RoadVertexDesc src = boost::source(e, sbrRoadsPtr->graph);
		RoadVertexDesc tgt = boost::target(e, sbrRoadsPtr->graph);
		if (src >= boost::num_vertices(sbrRoadsPtr->graph) || tgt >= boost::num_vertices(sbrRoadsPtr->graph)) return;
		sbrBlockContourLines.push_back(sbrRoadsPtr->graph[e]->polyline);
		sbrBlockContourEdges.push_back(e);
	}
};

//
// Remove intersecting edges of a graph
//
bool SmallBlockRemover::removeIntersectingEdges(RoadGraph &roadGraph) {
	//QSet<RoadGraph::roadGraphEdgeIter*> edgesToRemove2;
	std::vector<RoadEdgeIter> edgesToRemove;

	QVector2D a0, a1, b0, b1;
	QVector2D intPt;
	RoadEdgeIter a_ei, a_ei_end;
	RoadEdgeIter b_ei, b_ei_end;
	float ta0a1, tb0b1;

	for(boost::tie(a_ei, a_ei_end) = boost::edges(roadGraph.graph); a_ei != a_ei_end; ++a_ei){
		if (!roadGraph.graph[*a_ei]->valid) continue;

		a0 = QVector2D(roadGraph.graph[boost::source(*a_ei,roadGraph.graph)]->pt);
		a1 = QVector2D(roadGraph.graph[boost::target(*a_ei,roadGraph.graph)]->pt);

		//for(tie(b_ei, b_ei_end) = boost::edges(roadGraph.graph); b_ei != b_ei_end; ++b_ei){
		for(b_ei = a_ei; b_ei != a_ei_end; ++b_ei){			
			if (!roadGraph.graph[*b_ei]->valid) continue;

			if(b_ei != a_ei){
				b0 = QVector2D(roadGraph.graph[boost::source(*b_ei,roadGraph.graph)]->pt);
				b1 = QVector2D(roadGraph.graph[boost::target(*b_ei,roadGraph.graph)]->pt);

				if(Util::segmentSegmentIntersectXY(a0, a1, b0, b1, &ta0a1, &tb0b1, true, intPt) ){
					bool addEd=true;
					for(int eN=0;eN<edgesToRemove.size();eN++){
						if(edgesToRemove[eN]==b_ei){
							addEd=false;
							break;
						}
					}
					if(addEd)
						edgesToRemove.push_back(b_ei);
					//edgesToRemove2.insert(&b_ei);
				}
			}
		}		
	}

	for(int i=0; i<edgesToRemove.size(); ++i){	
		boost::remove_edge(*(edgesToRemove[i]),roadGraph.graph);
	}

	if(edgesToRemove.size()>0){
		return true;
	} else {
		return false;
	}
}//

void SmallBlockRemover::remove(RoadGraph &roads, float minArea) {
	GraphUtil::clean(roads);


	std::vector<Polygon2D> blocks;
	std::vector<std::vector<RoadEdgeDesc> > blocksEdges;

	sbrRoadsPtr = &roads;
	sbrBlocksPtr = &blocks;
	sbrBlocksPtr->clear();
	sbrBlocksEdges = &blocksEdges;

	//printf("b1.2\n");
	//std::cout << "Init num blocks is: " << blocksPtr->size() << std::endl;

	bool isPlanar = false;
	bool converges = true;

	//Make sure graph is planar
	typedef std::vector< RoadEdgeDesc > tEdgeDescriptorVector;
	std::vector<tEdgeDescriptorVector> embedding(boost::num_vertices(roads.graph));

	int cont=0;
	while(!isPlanar && converges)
	{	
		if(cont>2){
			std::cout << "ERROR: Graph could not be planarized (generateBlocks)\n";
			return;
		}
		// Test for planarity		
		if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph =roads.graph,
			boost::boyer_myrvold_params::embedding = &embedding[0]) ){
				//std::cout << "Input graph is planar" << std::endl;
				isPlanar = true;
		}
		else {			
			//std::cout << "Input graph is not planar" << std::endl;
			//Remove intersecting edges
			if(!removeIntersectingEdges(roads) ){
				converges = false;
			}
		}
		cont++;
	}

	// build embedding manually
	GraphUtil::buildEmbedding(roads, embedding);

	//Create edge index property map?	
	typedef std::map<RoadEdgeDesc, size_t> EdgeIndexMap;
	EdgeIndexMap mapEdgeIdx;
	boost::associative_property_map<EdgeIndexMap> pmEdgeIndex(mapEdgeIdx);		
	RoadEdgeIter ei, ei_end;	
	int edge_count = 0;
	for(boost::tie(ei, ei_end) = boost::edges(roads.graph); ei != ei_end; ++ei){
		mapEdgeIdx.insert(std::make_pair(*ei, edge_count++));	
	}

	//std::cout << "1..\n"; fflush(stdout);

	//Extract blocks from road graph using boost graph planar_face_traversal
	//std::cout << std::endl << "Vertices on the faces: " << std::endl;
	sbr_vertex_output_visitor v_vis;	
	boost::planar_face_traversal(roads.graph, &embedding[0], v_vis, pmEdgeIndex);

	//Misc postprocessing operations on blocks =======
	int maxVtxCount = 0;
	int maxVtxCountIdx = -1; 
	int numBadBlocks = 0;
	std::vector<float> blockAreas;

	for(int i=0; i<blocks.size(); ++i){
		if(blocks[i].size() < 3){
			blockAreas.push_back(0.0f);
			numBadBlocks++;
			continue;
		}

		//Compute block offset	
		float insetArea = blocks[i].area();
	
		blockAreas.push_back(insetArea);
	}

	//Remove the largest block
	float maxArea = -FLT_MAX;
	int maxAreaIdx = -1;
	for(int i=0; i<blocks.size(); ++i){
		if(blocks[i].size() < 3){
			continue;
		}
		//std::cout << "area: " << blockAreas[i] << "\n";
		if(blockAreas[i] > maxArea){
			maxArea = blockAreas[i];
			maxAreaIdx = i;
		}
	}

	if(maxAreaIdx != -1){
		blocks.erase(blocks.begin()+maxAreaIdx);
		blockAreas.erase(blockAreas.begin()+maxAreaIdx);

		blocksEdges.erase(blocksEdges.begin() + maxAreaIdx);
	}

	bool removed = true;
	while (removed) {
		removed = false;

		for (int i = 0; i < blocksEdges.size(); ++i) {
			if (blockAreas[i] < minArea) {
				if (removeEdge(roads, blocksEdges[i])) {
					removed = true;

					blocksEdges.erase(blocksEdges.begin() + i);
					blockAreas.erase(blockAreas.begin() + i);
					blocks.erase(blocks.begin() + i);

					break;
				}
			}
		}
	}
}

/**
 * 指定されたエッジ群で囲まれたブロックをなくすため、エッジの１つを削除する。
 * ちゃんと削除できたら、trueを返却する。
 */
bool SmallBlockRemover::removeEdge(RoadGraph &roads, std::vector<RoadEdgeDesc> &edges) {
	// 両端の頂点がT字型のエッジを削除する
	for (int i = 0; i < edges.size(); ++i) {
		// exampleのエッジは対象からはずす
		//if (roads.graph[edges[i]]->properties["generation_type"] == "example") continue;
		if (roads.graph[edges[i]]->properties.contains("shape")) continue;

		RoadVertexDesc src = boost::source(edges[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edges[i], roads.graph);

		if (GraphUtil::tshape(roads, src, edges[i]) && GraphUtil::tshape(roads, tgt, edges[i])) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}
	}

	// 両端の頂点のdegreeが3以上で、且つ、片方がT字型、もう片方が隣接エッジと180度をなさない、ようなエッジを削除する
	for (int i = 0; i < edges.size(); ++i) {
		// exampleのエッジは対象からはずす
		//if (roads.graph[edges[i]]->properties["generation_type"] == "example") continue;
		if (roads.graph[edges[i]]->properties.contains("shape")) continue;

		RoadVertexDesc src = boost::source(edges[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edges[i], roads.graph);

		if (GraphUtil::getDegree(roads, src) < 3 || GraphUtil::getDegree(roads, tgt) < 3) continue;

		if (GraphUtil::tshape(roads, src, edges[i]) && !GraphUtil::isStraightEdge(roads, tgt, edges[i])) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}	
		
		if (GraphUtil::tshape(roads, tgt, edges[i]) && !GraphUtil::isStraightEdge(roads, src, edges[i])) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}
	}

	// 両端の頂点のdegreeが3以上で、且つ、片方がdegree4以上で、//且つ、とも、180度をなすエッジが他に存在しない
	for (int i = 0; i < edges.size(); ++i) {
		// exampleのエッジは対象からはずす
		//if (roads.graph[edges[i]]->properties["generation_type"] == "example") continue;
		if (roads.graph[edges[i]]->properties.contains("shape")) continue;

		RoadVertexDesc src = boost::source(edges[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edges[i], roads.graph);

		if (GraphUtil::getDegree(roads, src) < 3 || GraphUtil::getDegree(roads, tgt) < 3) continue;

		//if (GraphUtil::isPotentiallyStraightEdge(roads, src, edges[i])) continue;
		//if (GraphUtil::isPotentiallyStraightEdge(roads, tgt, edges[i])) continue;

		if (GraphUtil::getDegree(roads, src) >= 4 || GraphUtil::getDegree(roads, tgt) >= 4) {
		//if (GraphUtil::tshape(roads, src, edges[i]) && GraphUtil::getDegree(roads, tgt) >= 4) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}

		/*
		if (GraphUtil::tshape(roads, tgt, edges[i]) && GraphUtil::getDegree(roads, src) >= 4) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}
		*/
	}

	return false;

	// 両端のdegreeが3以上のエッジを削除する
	for (int i = 0; i < edges.size(); ++i) {
		// exampleのエッジは対象からはずす
		if (roads.graph[edges[i]]->properties["generation_type"] == "example") continue;

		RoadVertexDesc src = boost::source(edges[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edges[i], roads.graph);

		if (GraphUtil::getDegree(roads, src) >= 3 && GraphUtil::getDegree(roads, tgt) >= 3) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}
	}

	return false;

	// 両端のdegreeが3以上で、且つ、片方のdegreeが3のエッジを削除する
	for (int i = 0; i < edges.size(); ++i) {
		// exampleのエッジは対象からはずす
		if (roads.graph[edges[i]]->properties["generation_type"] == "example") continue;

		RoadVertexDesc src = boost::source(edges[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edges[i], roads.graph);

		if (GraphUtil::getDegree(roads, src) < 3 || GraphUtil::getDegree(roads, tgt) < 3) continue;

		if (GraphUtil::getDegree(roads, src) == 3 || GraphUtil::getDegree(roads, tgt) == 3) {
		//if (GraphUtil::tshape(roads, src, edges[i]) || GraphUtil::tshape(roads, tgt, edges[i])) {
			std::cout << "removed: " << src << " - " << tgt << std::endl;

			roads.graph[edges[i]]->valid = false;
			return true;
		}
	}


	return false;
}
