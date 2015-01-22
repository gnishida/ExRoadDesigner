/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "VBOBuilding.h"
#include "BlockSet.h"

class VBORenderManager;

class VBOPm {
public:
	static bool generateBuildings(VBORenderManager& rendManager, BlockSet& blocks);
	static bool generateVegetation(VBORenderManager& rendManager, BlockSet& blocks);
};


