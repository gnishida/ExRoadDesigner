#pragma once

#include "glew.h"
#include "common.h"
#include <stdio.h>
#include "RoadVertex.h"
#include "RoadEdge.h"
/*
#include "../render/Renderable.h"
#include "../render/GeometryObject.h"
#include "../render/RenderableQuadList.h"
*/
//#include "../render/Terrain.h"
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

class RoadGraph {//: public mylib::GeometryObject {
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

	void generateMesh(VBORenderManager& renderManger,QString meshName);
	//void addMeshFromEdge(RenderablePtr renderable, RoadEdgePtr edge, float widthBase, QColor color, float height);
	//void addMeshFromVertex(RenderablePtr renderable, RoadVertexPtr vertex, QColor color, float height);

	void clear();
	void setZ(float z);
	void adaptToTerrain();
	//void add3DMeshOfEdge(mylib::RenderableQuadList* renderable, RoadEdgePtr edge, float width, QColor color, float heightOffset = 0.0f);

	/*
	void generate2DMesh();
	void add2DMeshOfEdge(mylib::RenderablePtr renderable, RoadEdgePtr edge, float widthBase, QColor color, float height);
	void add2DMeshOfVertex(mylib::RenderablePtr renderable, RoadVertexPtr vertex, QColor color, float height);
	*/
private:
	/*
	void _generateMeshVertices(mylib::TextureManager* textureManager);
	void _generateMeshVerticesDefault(mylib::TextureManager* textureManager);
	void _generateMeshVerticesTexture(mylib::TextureManager* textureManager);
	*/
	void _generateMeshVerticesGroupBy(VBORenderManager& renderManger,QString meshName);
	/*
	void _generateMeshVerticesGenerationType(mylib::TextureManager* textureManager);
	*/
};

typedef boost::shared_ptr<RoadGraph> RoadGraphPtr;
