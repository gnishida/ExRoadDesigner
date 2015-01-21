/************************************************************************************************
 *		Procedural City Generation: Parcel
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOBlock.h"

class VBOPmParcels{
public:

	static bool generateParcels(VBORenderManager& rendManager, std::vector< Block > &blocks);
};
