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

/**
 * 当該パッチの各コネクタについて、指定されたpolylineとのHausdorff distanceを計算し、最小値を返却する。
 * また、最小値を実現するコネクタの付け根にあたる頂点IDをv_rootに、先端の頂点IDをv_connectorに格納する。
 *
 * @param polyline			このpolylineとのHausdorf distanceを計算する
 * @param v_connector [OUT]	最小のHausdorff distanceを得るコネクタの付け根にあたる頂点ID
 * @param v_root [OUT]		最小のHausdorff distanceを得るコネクタの先端の頂点ID
 * @return					Hausdorff distanceの最小値
 */
float Patch::getMinHausdorffDist(const Polyline2D& polyline, RoadVertexDesc& v_connector, RoadVertexDesc& v_root) {
	float min_dist = std::numeric_limits<float>::max();

	for (int i = 0; i < connectors.size(); ++i) {
		RoadVertexDesc v;
		float dist = getHausdorffDist(polyline, connectors[i], v);
		if (dist < min_dist) {
			min_dist = dist;
			v_connector = connectors[i];
			v_root = v;
		}
	}

	return min_dist;
}

float Patch::getCost(const Polyline2D& polyline, RoadVertexDesc connector, RoadVertexDesc& root_desc) {
	Polyline2D p = GraphUtil::getAdjoiningPolyline(roads, connector, root_desc);

	return (p.back() - p.front() + polyline.front() - polyline.back()).lengthSquared();
}

/**
 * 指定されたpolylineと、指定された頂点から伸びるエッジのpolylineとのHausdorff distanceを計算して返却する。
 * ただし、polyline[0]と、指定された頂点の座標をそろえる。
 * また、指定された頂点から伸びるエッジの根元の頂点IDをroot_descに格納して返却する。
 *
 * @param polyline			指定されたpolyline
 * @param connector			指定された頂点
 * @param root_desc [OUT]	指定された頂点から伸びるエッジの根元の頂点ID
 * @reutnr					Hausdorff distance
 */
float Patch::getHausdorffDist(const Polyline2D& polyline, RoadVertexDesc connector, RoadVertexDesc& root_desc) {
	Polyline2D p = GraphUtil::getAdjoiningPolyline(roads, connector, root_desc);

	QVector2D offset = p[0] - polyline[0];

	float max_d = 0.0f;
	for (int i = 0; i < p.size(); ++i) {
		float min_d = std::numeric_limits<float>::max();
		for (int j = 0; j < polyline.size(); ++j) {
			float d = (p[i] - polyline[j] - offset).length();
			if (d < min_d) min_d = d;
		}

		if (min_d > max_d) max_d = min_d;
	}
	for (int i = 0; i < polyline.size(); ++i) {
		float min_d = std::numeric_limits<float>::max();
		for (int j = 0; j < p.size(); ++j) {
			float d = (p[j] - polyline[i] - offset).length();
			if (d < min_d) min_d = d;
		}

		if (min_d > max_d) max_d = min_d;
	}

	return max_d;
}
