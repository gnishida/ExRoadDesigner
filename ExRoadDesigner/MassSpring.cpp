#include "MassSpring.h"
#include "VoronoiUtil.h"

MassSpring::MassSpring(RoadGraph &roads) : roads(roads) {
	std::vector<VoronoiVertex> points;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		points.push_back(VoronoiVertex(roads.graph[*vi]->pt, *vi));
	}

	VoronoiUtil::voronoi(points, springs);
}

void MassSpring::move(RoadVertexDesc v_desc, const QVector2D &newPt) {
	
}
