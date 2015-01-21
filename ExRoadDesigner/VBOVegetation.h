#pragma once

#include "VBOBlock.h"
#include "VBOModel_StreetElements.h"

class VBORenderManager;

class VBOVegetation
{
public:
	VBOVegetation(void);

	~VBOVegetation(void);

	static bool generateVegetation(VBORenderManager& rendManager, std::vector< Block > &blocks);

};
