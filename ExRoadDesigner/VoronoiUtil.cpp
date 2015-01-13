#include "VoronoiUtil.h"
#include "Util.h"

/**
 * Based on the existing road graph and the selected road graph, build a voronoi diagram.
 */
void VoronoiUtil::voronoi(std::vector<VoronoiVertex> &points, std::vector<std::pair<RoadVertexDesc, RoadVertexDesc> > &delaunay_tri_edges) {
	// Construction of the Voronoi Diagram.
	boost::polygon::voronoi_diagram<double> vd;
	construct_voronoi(points.begin(), points.end(), &vd);

	// ボロノイ図の各セルについて
	for (boost::polygon::voronoi_diagram<double>::const_cell_iterator it = vd.cells().begin(); it != vd.cells().end(); ++it) {
		const boost::polygon::voronoi_diagram<double>::cell_type& cell = *it;
		const boost::polygon::voronoi_diagram<double>::edge_type* edge = cell.incident_edge();

	    std::size_t cell_index = it->source_index();
		VoronoiVertex v = points[cell_index];

		// ボロノイ図の各セルについて、エッジのサイクルを辿る（反時計回り？）
		Polygon2D face;
		do {
			if (!edge->is_primary()) continue;
				
			VoronoiVertex v2 = points[edge->twin()->cell()->source_index()];

			delaunay_tri_edges.push_back(std::make_pair(v.desc, v2.desc));

			edge = edge->next();
		} while (edge != cell.incident_edge());
	}
}

