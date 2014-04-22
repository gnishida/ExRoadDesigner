/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>

#include "VBOBlock.h"
#include "VBOParcel.h"
#include "VBOBuilding.h"
#include "VBOPlaceType.h"
#include "RoadGraph.h"


class VBORenderManager;

class VBOPm
{
public:

	static bool initialized;
	static void init();

	static PlaceTypesMainClass placeTypes;

	static bool generateGeometry(VBORenderManager& rendManager,RoadGraph &roadGraph);

private:

};


