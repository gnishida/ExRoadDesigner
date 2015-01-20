#include "RoadGraph.h"
#include <QGLWidget>
#include "GraphUtil.h"
#include "Util.h"
#include "global.h"
#include "RoadMeshGenerator.h"

RoadGraph::RoadGraph() {
	showHighways = true;
	showBoulevards = true;
	showAvenues = true;
	showLocalStreets = true;

	modified = false;
}

RoadGraph::~RoadGraph() {
}

void RoadGraph::clear() {
	graph.clear();
	modified = true;
}



/**
 * adapt this road graph to the vboRenderManager.
 */
/*
void RoadGraph::adaptToTerrain(VBORenderManager* vboRenderManager) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		float z = 0.0f;
		if (!G::getBool("shader2D")) {
			z = vboRenderManager->getTerrainHeight(graph[*vi]->pt.x(), graph[*vi]->pt.y());
			if (z < G::getFloat("seaLevelForStreet")) {
				graph[*vi]->properties["bridge"] = true;
				z = getBridgeElevationFromClosestNeighbors(*vi, *vboRenderManager, G::getFloat("seaLevelForStreet"));
			}
		}
		graph[*vi]->pt3D = QVector3D(graph[*vi]->pt.x(), graph[*vi]->pt.y(), z + 2);
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, graph);
		RoadVertexDesc tgt = boost::target(*ei, graph);
		graph[*ei]->polyline3D.clear();

		bool bridge = false;
		Polyline2D polyline = GraphUtil::finerEdge(graph[*ei]->polyline, 10.0f);
		for (int i = 0; i < polyline.size(); ++i) {
			float z = 0.0f;
			if (!G::getBool("shader2D")) {
				z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y());
				if (z < G::getFloat("seaLevelForStreet")) {
					bridge = true;
					z = getBridgeElevationFromClosestNeighbors(*ei, polyline, i, *vboRenderManager, G::getFloat("seaLevelForStreet"));
				}
			}
			graph[*ei]->polyline3D.push_back(QVector3D(polyline[i].x(), polyline[i].y(), z + 2));
		}
		
		graph[*ei]->properties["bridge"] = bridge;
	}

	setModified();
}
*/

float RoadGraph::getBridgeElevationFromClosestNeighbors(RoadVertexDesc srcDesc, VBORenderManager &renderManager, float seaLevel) {
	QList<RoadVertexDesc> queue;
	queue.push_back(srcDesc);

	QMap<RoadVertexDesc, bool> visited;

	while (!queue.empty()) {
		RoadVertexDesc desc = queue.front();
		queue.pop_front();

		if (visited[desc]) continue;
		visited[desc] = true;

		float z = renderManager.getTerrainHeight(graph[desc]->pt.x(), graph[desc]->pt.y());
		if (z >= seaLevel) {
			return z;
		}

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, graph);
			if (visited[tgt]) continue;
			queue.push_back(tgt);
		}
	}

	return seaLevel;
}

float RoadGraph::getBridgeElevationFromClosestNeighbors(RoadEdgeDesc edge_desc, const Polyline2D &polyline, int index, VBORenderManager &renderManager, float seaLevel) {
	RoadVertexDesc src = boost::source(edge_desc, graph);
	RoadVertexDesc tgt = boost::target(edge_desc, graph);

	/*
	float z = renderManager.getTerrainHeight(polyline[index].x(), polyline[index].y(), true);
	if (z >= seaLevel) {
		return z;
	}
	*/

	// find the one height
	float elevation1 = getBridgeElevationFromClosestNeighbors(src, renderManager, seaLevel);
	float dist1;
	{
		if ((graph[src]->pt - polyline[0]).lengthSquared() < (graph[tgt]->pt - polyline[0]).lengthSquared()) {
			dist1 = polyline.length(index);
		} else {
			dist1 = polyline.length() - polyline.length(index);
		}
	}

	// find another height
	float elevation2 = getBridgeElevationFromClosestNeighbors(tgt, renderManager, seaLevel);
	float dist2;
	{
		if ((graph[src]->pt - polyline[0]).lengthSquared() < (graph[tgt]->pt - polyline[0]).lengthSquared()) {
			dist2 = polyline.length() - polyline.length(index);
		} else {
			dist2 = polyline.length(index);
		}
	}

	return (elevation1 * dist2 + elevation2 * dist1) / (dist1 + dist2);
}
