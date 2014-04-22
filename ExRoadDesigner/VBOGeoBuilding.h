/************************************************************************************************
 *		Procedural City Generation: Buildings geometry
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOBuilding.h"

class VBORenderManager;

class VBOGeoBuilding {
public:
	static void generateBuilding(VBORenderManager& rendManager,Building& building);
	static bool bldgInitialized;
	static void initBuildingsTex();
};
