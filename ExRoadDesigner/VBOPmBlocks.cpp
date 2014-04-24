/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPmBlocks.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>

///////////////////////////////////////////////////////////////
// INIT
///////////////////////////////////////////////////////////////
bool VBOPmBlocks::initialized=false;
std::vector<QString> VBOPmBlocks::sideWalkFileNames;
std::vector<QVector3D> VBOPmBlocks::sideWalkScale;

void VBOPmBlocks::init(){
	/////////////////////////////////////////
	// INIT SIDEWALK TEX
	QString pathName="../data/textures/LC";
	// 3. sidewalk
	QDir directorySW(pathName+"/sidewalk/");
	QStringList nameFilter;
	nameFilter << "*.png" << "*.jpg" << "*.gif";
	QStringList list = directorySW.entryList( nameFilter, QDir::Files );
	for(int lE=0;lE<list.size();lE++){
		if(QFile::exists(pathName+"/sidewalk/"+list[lE])){
			sideWalkFileNames.push_back(pathName+"/sidewalk/"+list[lE]);
			QStringList scaleS=list[lE].split("_");
			if(scaleS.size()!=4)
				sideWalkScale.push_back(QVector3D(1.0f,1.0f,0));
			else{
				sideWalkScale.push_back(QVector3D(scaleS[1].toFloat(),scaleS[2].toFloat(),0));
			}
		}
	}
	printf("-->Initialized sidewalk tex %d\n",sideWalkFileNames.size());
	initialized=true;
}//

///////////////////////////////////////////////////////////////
// BLOCKS
///////////////////////////////////////////////////////////////
RoadGraph * roadGraphPtr;
std::vector< Block > * blocksPtr;
//Loop3D blockContourTmp;

Polygon3D blockContourTmp;
Polygon3D blockContourPoints;
std::vector<Polyline3D> blockContourLines;

std::vector< float > blockContourWidths;
bool isFirstVertexVisited;

bool goodVertexPlaceTypeFound;
int maxPlaceTypeIdx;

int curRandSeed;
int curPlaceTypeIdx;

struct output_visitor : public boost::planar_face_traversal_visitor
{
	void begin_face()
	{
		/*
		blockContourTmp.clear();
		blockContourWidths.clear();
		isFirstVertexVisited = true;
		curRandSeed = 0;
		*/
		blockContourTmp.clear();
		blockContourWidths.clear();

		blockContourPoints.clear();
		blockContourLines.clear();
	}

	void end_face()
	{
		/*
		if(blockContourTmp.size() > 2){
			Block newBlock;
			newBlock.blockContour.contour = blockContourTmp;		
			newBlock.blockContourRoadsWidths = blockContourWidths;
			newBlock.randSeed = curRandSeed;	
			blocksPtr->push_back(newBlock);//!!!! UPDATEE
		}
		*/
		blockContourTmp.clear();
			
		for (int i = 0; i < blockContourPoints.contour.size(); ++i) {
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
		if (blockContourTmp.contour.size() >= 3 && blockContourWidths.size() >= 3) {
			Block newBlock;
			newBlock.blockContour = blockContourTmp;
			newBlock.blockContourRoadsWidths = blockContourWidths;
			while (newBlock.blockContour.contour.size() > newBlock.blockContourRoadsWidths.size()) {
				newBlock.blockContourRoadsWidths.push_back(newBlock.blockContourRoadsWidths.back());
			}
	
			blocksPtr->push_back(newBlock);
		}
	}
};

//Vertex visitor
struct vertex_output_visitor : public output_visitor
{
	template <typename Vertex> 
	void next_vertex(Vertex v) 
	{ 	
		//std::cout << v << " 
		/*
		if(  v >= 0 && v < boost::num_vertices(roadGraphPtr->graph) ){
			blockContourTmp.push_back( (roadGraphPtr->graph)[v]->pt );

			//initialize block random seed from first street node random seed
			if(isFirstVertexVisited){
				isFirstVertexVisited = false;
				curRandSeed = ( (roadGraphPtr->graph)[v]->randSeed*4096 + 150889) % 714025;
			}
		}*/
		blockContourPoints.push_back(roadGraphPtr->graph[v]->pt);
	}

	template <typename Edge> 
	void next_edge(Edge e) 
	{ 
		/*
		int sIdx, tIdx;
		sIdx = boost::source(e, roadGraphPtr->graph);
		tIdx = boost::target(e, roadGraphPtr->graph);

		if(  sIdx >= 0 && sIdx < boost::num_vertices(roadGraphPtr->graph) &&
			tIdx >= 0 && tIdx < boost::num_vertices(roadGraphPtr->graph) ){			

				blockContourWidths.push_back( 0.5f*7.0f);///!!!!! UPDATEE //roadGraphPtr->graph)[e]->wid);	//half of the width	
		}
		*/
		blockContourLines.push_back(roadGraphPtr->graph[e]->polyline3D);

		for (int i = 0; i < roadGraphPtr->graph[e]->polyline3D.size() - 1; ++i) {
			blockContourWidths.push_back(0.5f * roadGraphPtr->graph[e]->getWidth());
		}

	}
};

//
// Remove intersecting edges of a graph
//
bool removeIntersectingEdges(RoadGraph &roadGraph)
{
	//QSet<RoadGraph::roadGraphEdgeIter*> edgesToRemove2;
	std::vector<RoadEdgeIter> edgesToRemove;

	QVector2D a0, a1, b0, b1;
	QVector2D intPt;
	RoadEdgeIter a_ei, a_ei_end;
	RoadEdgeIter b_ei, b_ei_end;
	float ta0a1, tb0b1;

	for(boost::tie(a_ei, a_ei_end) = boost::edges(roadGraph.graph); a_ei != a_ei_end; ++a_ei){
		a0 = QVector2D(roadGraph.graph[boost::source(*a_ei,roadGraph.graph)]->pt);
		a1 = QVector2D(roadGraph.graph[boost::target(*a_ei,roadGraph.graph)]->pt);

		//for(tie(b_ei, b_ei_end) = boost::edges(roadGraph.graph); b_ei != b_ei_end; ++b_ei){
		for(b_ei = a_ei; b_ei != a_ei_end; ++b_ei){			

			if(b_ei != a_ei){
				b0 = QVector2D(roadGraph.graph[boost::source(*b_ei,roadGraph.graph)]->pt);
				b1 = QVector2D(roadGraph.graph[boost::target(*b_ei,roadGraph.graph)]->pt);

				if(Polygon3D::segmentSegmentIntersectXY(a0, a1, b0, b1, &ta0a1, &tb0b1, true, intPt) ){
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

//
// Given a road network, this function extracts the blocks
//
bool VBOPmBlocks::generateBlocks(
	PlaceTypesMainClass &placeTypesIn,
	RoadGraph &roadGraph,
	std::vector< Block > &blocks)
{
	if(initialized==false){
		init();
	}
	//printf("b1.1\n");
	roadGraphPtr = &roadGraph;
	blocksPtr = &blocks;
	blocksPtr->clear();
	//printf("b1.2\n");
	//std::cout << "Init num blocks is: " << blocksPtr->size() << std::endl;

	bool isPlanar = false;
	bool converges = true;

	maxPlaceTypeIdx =G::global().getInt("num_place_types");//placeTypesIn.myPlaceTypes.size();

	//Make sure graph is planar
	typedef std::vector< RoadEdgeDesc > tEdgeDescriptorVector;
	std::vector<tEdgeDescriptorVector> embedding(boost::num_vertices(roadGraph.graph));
	//printf("b1.3\n");
	int cont=0;
	while(!isPlanar && converges)
	{	
		if(cont>2){
			return false;
		}
		// Test for planarity		
		if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph =roadGraph.graph,
			boost::boyer_myrvold_params::embedding = &embedding[0]) ){
				//std::cout << "Input graph is planar" << std::endl;
				isPlanar = true;
		}
		else {			
			//std::cout << "Input graph is not planar" << std::endl;
			//Remove intersecting edges
			if(!removeIntersectingEdges(roadGraph) ){
				converges = false;
			}
		}
		cont++;
	}

	if(!isPlanar){
		std::cout << "ERROR: Graph could not be planarized (generateBlocks)\n";
		return false;
	}

	// build embedding manually
	buildEmbedding(roadGraph, embedding);

	//Create edge index property map?	
	typedef std::map<RoadEdgeDesc, size_t> EdgeIndexMap;
	EdgeIndexMap mapEdgeIdx;
	boost::associative_property_map<EdgeIndexMap> pmEdgeIndex(mapEdgeIdx);		
	RoadEdgeIter ei, ei_end;	
	int edge_count = 0;
	for(boost::tie(ei, ei_end) = boost::edges(roadGraph.graph); ei != ei_end; ++ei){
		mapEdgeIdx.insert(std::make_pair(*ei, edge_count++));	
	}

	//std::cout << "1..\n"; fflush(stdout);

	//Extract blocks from road graph using boost graph planar_face_traversal
	//std::cout << std::endl << "Vertices on the faces: " << std::endl;
	vertex_output_visitor v_vis;	
	boost::planar_face_traversal(roadGraph.graph, &embedding[0], v_vis, pmEdgeIndex);

	//Misc postprocessing operations on blocks =======
	int maxVtxCount = 0;
	int maxVtxCountIdx = -1; 
	float avgInsetArea = 0.0f;
	int numBadBlocks = 0;
	std::vector<float> blockAreas;

	Loop3D blockContourInset;
	for(int i=0; i<blocks.size(); ++i){
		//assign default place type
		blocks[i].setMyPlaceTypeIdx(-1);
		//Reorient faces
		if(Polygon3D::reorientFace(blocks[i].blockContour.contour)){
			std::reverse(blocks[i].blockContourRoadsWidths.begin(),
				blocks[i].blockContourRoadsWidths.end() - 1);
			//std::cout << "reorient\n";
		}

		/*
		//fix block geometry before calling function...
		Loop3D cleanPgon;
		Polygon3D::cleanLoop(blocks[i].blockContour.contour, cleanPgon, 5.0f);		

		//update widths			
		if(blocks[i].blockContour.contour.size() != cleanPgon.size()){
			//std::cout << "clean\n";
			int cleanPgonSz = cleanPgon.size();
			std::vector<float> cleanWidths(cleanPgonSz);

			for(int j=0; j<cleanPgonSz; ++j){
				//find element j in from clean polygon in original polygon
				//if element IS there, add to clean width array
				for(int k=0; k<blocks[i].blockContour.contour.size(); ++k){
					if( cleanPgon[j] == blocks[i].blockContour.contour[k] ){
						cleanWidths[(j-1+cleanPgonSz)%cleanPgonSz] = blocks[i].blockContourRoadsWidths[k];
						//std::cout << blocks[i].blockContourRoadsWidths[k] << " ";
						break;
					}			
				}
			}
			blocks[i].blockContour.contour = cleanPgon;
			blocks[i].blockContourRoadsWidths = cleanWidths;
			//std::cout << cleanPgon.size() << " " << cleanWidths.size() << "\n";
			blocks[i].myColor = QVector3D(0.5f, 0.7f, 0.8f);		
		}
		*/

		if( blocks[i].blockContour.contour.size() != blocks[i].blockContourRoadsWidths.size() ){
			std::cout << "Error: " << blocks[i].blockContour.contour.size() << " " << blocks[i].blockContourRoadsWidths.size() << "\n";
			blocks[i].blockContour.contour.clear();
			blockAreas.push_back(0.0f);
			numBadBlocks++;
			continue;
		}

		if(blocks[i].blockContour.contour.size() < 3){
			blockAreas.push_back(0.0f);
			numBadBlocks++;
			continue;
		}

		//Compute block offset	
		float insetArea = blocks[i].blockContour.computeInset(
			blocks[i].blockContourRoadsWidths,
			blockContourInset);


		blocks[i].blockContour.contour = blockContourInset;
		blocks[i].blockContour.getBBox3D(blocks[i].bbox.minPt, blocks[i].bbox.maxPt);

		avgInsetArea += insetArea;
		blockAreas.push_back(insetArea);

		//assign place type to block ------------
		int validClosestPlaceTypeIdx = -1;

		//if(blocks.size() > 5)

		float distToValidClosestPlaceType = FLT_MAX;
		QVector3D testPt;
		testPt = blocks.at(i).bbox.midPt();

		//NEW WAY!
		for(int k=G::global().getInt("num_place_types")-1; k>=0; --k){			
			if( placeTypesIn.myPlaceTypes.at(k).containsPoint(testPt) ){				
				validClosestPlaceTypeIdx = k;
			}			
		}

		blocks[i].setMyPlaceTypeIdx( validClosestPlaceTypeIdx );

		//---------------------------------------
	}
	avgInsetArea = avgInsetArea/ ( (float)(blockAreas.size() - numBadBlocks));

	//Remove the largest block
	float maxArea = -FLT_MAX;
	int maxAreaIdx = -1;
	for(int i=0; i<blocks.size(); ++i){
		if(blocks[i].blockContour.contour.size() < 3){
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
	}
	return true;
}//

void VBOPmBlocks::buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding) {
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

