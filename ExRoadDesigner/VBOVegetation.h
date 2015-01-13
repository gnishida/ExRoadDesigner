#pragma once

#include "VBOBlock.h"
#include "VBOPlaceType.h"
#include "VBOModel_StreetElements.h"


	class VBORenderManager;

	class VBOVegetation
	{
	public:
		VBOVegetation(void);

		~VBOVegetation(void);

		static bool generateVegetation(VBORenderManager& rendManager,PlaceTypesMainClass &placeTypesIn,std::vector< Block > &blocks);

	};

