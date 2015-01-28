/************************************************************************************************
 *		Procedural City Generation: Blocks
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "VBOBlock.h"
#include "VBOParcel.h"
#include "VBOBuilding.h"
#include "RoadGraph.h"
#include "BlockSet.h"

class VBORenderManager;

class VBOPmBlocks
{
public:

	//Generate Blocks
	static bool generateBlocks(VBORenderManager* renderManager, RoadGraph &roadGraph, BlockSet &blocks);

	static void buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding);
	static void generateSideWalk(VBORenderManager* renderManager, BlockSet& blocks);
};


