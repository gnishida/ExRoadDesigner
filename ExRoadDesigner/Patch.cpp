#include "Patch.h"
#include "GraphUtil.h"

Patch::Patch() {
}

/**
 * 指定されたpolylineに最も似ているコネクタを探し、
 * その付け根にあたる頂点IDをv_descに格納する。
 * また、非類似度（polylineの差の距離）を返却する。
 */
float Patch::getMinCost(const Polyline2D& polyline, RoadVertexDesc& v_connector, RoadVertexDesc& v_root) {
	float min_cost = std::numeric_limits<float>::max();

	for (int i = 0; i < connectors.size(); ++i) {
		RoadVertexDesc v;
		float cost = getCost(polyline, connectors[i], v);
		if (cost < min_cost) {
			min_cost = cost;
			v_connector = connectors[i];
			v_root = v;
		}
	}

	return min_cost;
}

float Patch::getCost(const Polyline2D& polyline, RoadVertexDesc connector, RoadVertexDesc& root_desc) {
	Polyline2D p = GraphUtil::getAdjoiningPolyline(roads, connector, root_desc);

	return (p.back() - p.front() + polyline.front() - polyline.back()).lengthSquared();
}

