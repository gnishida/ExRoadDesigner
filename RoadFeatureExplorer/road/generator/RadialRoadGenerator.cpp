#include "../../common/Util.h"
#include "../GraphUtil.h"
#include "RadialRoadGenerator.h"

RadialRoadGenerator::RadialRoadGenerator() {
}

RadialRoadGenerator::~RadialRoadGenerator() {
}

/**
 * Radialパターンの道路網を生成する
 */
void RadialRoadGenerator::generateRoadNetwork(RoadGraph &roads, const Polygon2D &area, const RadialFeature& rf) {
	// 円形の道路を生成すると共に、シードを生成する
	std::list<RoadVertexDesc> seeds;
	generateCircleAvenues(roads, area, rf, seeds);

	// シードから放射線状にAvenueを生成する
	while (!seeds.empty()) {
		RoadVertexDesc desc = seeds.front();
		seeds.pop_front();
		expandRadialAvenues(roads, area, rf, desc);
	}
}

/**
 * 指定されたRadial Featureに基づき、円形の道路を生成する。
 */
void RadialRoadGenerator::generateCircleAvenues(RoadGraph& roads, const Polygon2D& area, const RadialFeature& rf, std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	QVector2D center = area.centroid();

	float theta = Util::uniform_rand() * M_PI * 2.0f;
	float theta_step = M_PI * 2.0f / rf.numDirections;

	std::vector<RoadVertexDesc> first_descs;
	std::vector<RoadVertexDesc> prev_descs;
	first_descs.resize(rf.radii.size());
	prev_descs.resize(rf.radii.size());

	for (int i = 0; i < rf.numDirections; ++i, theta += theta_step) {
		for (int j = 0; j < rf.radii.size(); ++j) {
			// 頂点を追加
			QVector2D pt = center + QVector2D(cosf(theta), sinf(theta)) * rf.radii[j];
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
			RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
			roads.graph[desc]->angles.push_back(Util::normalizeAngle(theta));

			if (j > 0) {
				// １つ内側の頂点との間に、エッジを生成
				GraphUtil::addEdge(roads, desc - 1, desc, 2, 1);
			}

			if (j == rf.radii.size() - 1) {
				// 一番外の頂点を、シードに追加
				seeds.push_back(desc);
			}

			if (i == 0) {
				// 最初の頂点を保存しておく
				first_descs[j] = desc;
			} else {
				// 時計方向に１つ前の頂点との間に、エッジを生成
				GraphUtil::addEdge(roads, prev_descs[j], desc, 2, 1);
			}

			prev_descs[j] = desc;
		}
	}

	// 最後の頂点と最初の頂点を結ぶエッジを追加
	for (int i = 0; i < rf.radii.size(); ++i) {
		GraphUtil::addEdge(roads, prev_descs[i], first_descs[i], 2, 1);
	}
}

/** 
 * 指定された頂点から、道路を延長していく。
 * この道路は、放射線上の道路のうちの１つの線を構成する。
 */
void RadialRoadGenerator::expandRadialAvenues(RoadGraph& roads, const Polygon2D& area, const RadialFeature& rf, RoadVertexDesc desc) {
	RoadVertexDesc prev_desc = desc;

	while (true) {
		float theta = roads.graph[prev_desc]->angles[0];
		QVector2D pt = roads.graph[prev_desc]->pt + QVector2D(cosf(theta), sinf(theta)) * 200.0f;

		// エリア内かどうかチェック
		if (!area.contains(pt)) {
			// エリア外周との交点を求める
			QVector2D intPoint;
			area.intersects(roads.graph[prev_desc]->pt, pt, intPoint);

			// 外周上に頂点を追加
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(intPoint));
			RoadVertexDesc desc = GraphUtil::addVertex(roads, v);

			// １つ前の頂点との間にエッジを生成
			GraphUtil::addEdge(roads, prev_desc, desc, 2, 1);

			break;
		}

		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
		RoadVertexDesc desc = GraphUtil::addVertex(roads, v);

		// とりあえず、角度は１つ前のものをコピーしておく
		roads.graph[desc]->angles.push_back(theta);

		// １つ前の頂点との間にエッジを生成
		GraphUtil::addEdge(roads, prev_desc, desc, 2, 1);

		prev_desc = desc;
	}
}
