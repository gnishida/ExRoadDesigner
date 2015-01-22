/************************************************************************************************
 *		Procedural City Generation: Parcel
 *		@author igarciad
 ************************************************************************************************/

#pragma once

#include "VBOBlock.h"

class VBOPmParcels{
public:
	static bool generateParcels(VBORenderManager& rendManager, std::vector< Block > &blocks);

private:
	static void subdivideBlockIntoParcels(Block &block);
	static bool subdivideParcel(Block &block, Parcel parcel, float areaMean, float areaMin, float areaVar, float splitIrregularity, std::vector<Parcel> &outParcels);
};
