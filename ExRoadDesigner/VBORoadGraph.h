#pragma once

//#include "misctools\misctools.h"
#include "RoadGraph.h"
#include "VBOUtil.h"



class VBORenderManager;

class VBORoadGraph{

public:

	static void clearVBORoadGraph(VBORenderManager& rendManager);
	static void updateRoadGraph(VBORenderManager& rendManager,RoadGraph &roadGraph);
	static void renderRoadGraphLines(VBORenderManager& rendManager);

};

