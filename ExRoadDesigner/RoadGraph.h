#pragma once

#include "glew.h"
#include "common.h"
#include <stdio.h>
#include "RoadVertex.h"
#include "RoadEdge.h"
#include "VBORenderManager.h"

using namespace boost;

typedef adjacency_list<vecS, vecS, undirectedS, RoadVertexPtr, RoadEdgePtr> BGLGraph;
typedef graph_traits<BGLGraph>::vertex_descriptor RoadVertexDesc;
typedef graph_traits<BGLGraph>::edge_descriptor RoadEdgeDesc;
typedef graph_traits<BGLGraph>::vertex_iterator RoadVertexIter;
typedef graph_traits<BGLGraph>::edge_iterator RoadEdgeIter;
typedef graph_traits<BGLGraph>::out_edge_iterator RoadOutEdgeIter;
typedef graph_traits<BGLGraph>::in_edge_iterator RoadInEdgeIter;


typedef std::vector<RoadEdgeDesc> RoadEdgeDescs;
typedef std::vector<RoadVertexDesc> RoadVertexDescs;

class Terrain;

class RoadGraph {
public:
	static enum { RENDER_DEFAULT = 0, RENDER_TEXTURE, RENDER_GROUPBY, RENDER_GENERATION_TYPE };

public:
	bool modified;
	BGLGraph graph;

	// for rendering (These variables should be updated via setZ() function only!!
	float highwayHeight;
	float avenueHeight;
	float widthBase;
	float curbRatio;

	QColor colorHighway;
	QColor colorBoulevard;
	QColor colorAvenue;
	QColor colorStreet;
	bool showHighways;
	bool showBoulevards;
	bool showAvenues;
	bool showLocalStreets;

public:
	RoadGraph();
	~RoadGraph();

	void setModified() { modified = true; }

	void clear();
	//void adaptToTerrain(VBORenderManager* vboRenderManager);

public:
	void _generateMeshVerticesDefault(VBORenderManager& renderManager);
	float getBridgeElevationFromClosestNeighbors(RoadVertexDesc srcDesc, VBORenderManager &renderManager, float seaLevel);
	float getBridgeElevationFromClosestNeighbors(RoadEdgeDesc edge_desc, const Polyline2D &polyline, int index, VBORenderManager &renderManager, float seaLevel);
};

typedef boost::shared_ptr<RoadGraph> RoadGraphPtr;
