#pragma once

#include "glew.h"
#include "common.h"
#include <stdio.h>
#include "RoadVertex.h"
#include "RoadEdge.h"
#include "Terrain.h"
#include "VBORenderManager.h"

using namespace boost;

typedef adjacency_list<vecS, vecS, undirectedS, RoadVertexPtr, RoadEdgePtr> BGLGraph;
typedef graph_traits<BGLGraph>::vertex_descriptor RoadVertexDesc;
typedef graph_traits<BGLGraph>::edge_descriptor RoadEdgeDesc;
typedef graph_traits<BGLGraph>::vertex_iterator RoadVertexIter;
typedef graph_traits<BGLGraph>::edge_iterator RoadEdgeIter;
typedef graph_traits<BGLGraph>::out_edge_iterator RoadOutEdgeIter;
typedef graph_traits<BGLGraph>::in_edge_iterator RoadInEdgeIter;

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

	int renderMode;

public:
	RoadGraph();
	~RoadGraph();

	void setModified() { modified = true; }

	void generateMesh(VBORenderManager& renderManger, const QString &linesN, const QString &pointsN);

	void clear();
	//void setZ(float z);
	void adaptToTerrain(Terrain* terrain);

private:
	void _generateMeshVerticesDefault(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN);
	void _generateMeshVerticesTexture(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN);
	void _generateMeshVerticesGroupBy(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN);
	void _generateMeshVerticesGenerationType(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN);
};

typedef boost::shared_ptr<RoadGraph> RoadGraphPtr;
