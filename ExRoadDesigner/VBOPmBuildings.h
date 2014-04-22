/************************************************************************************************
 *		Procedural City Generation: Buildings
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOPlaceType.h"
#include "VBOBlock.h"


class VBOPmBuildings{
public:

	static bool generateBuildings(PlaceTypesMainClass &placeTypesIn,std::vector< Block > &blocks);

};

