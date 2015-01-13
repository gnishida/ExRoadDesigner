#include "CircleDetector.h"

std::vector<RoadGraph> detect(RoadGraph& roads) {
	std::vector<RadialFeaturePtr> rfs = detectOneCircleCenter(roads, area, roadType, scale1, sigma, 30.0f, 20.0f, 150.0f);

	std::vector<RadialFeaturePtr> rfs2;
	for (int i = 0; i < rfs.size(); ++i) {
		QMap<RoadEdgeDesc, bool> edges;
		if (findOneRadial(roads, area, roadType, angleThreshold3, votingRatioThreshold, seedDistance, minSeedDirections, extendingAngleThreshold, *rfs[i], edges)) {
			rfs2.push_back(rfs[i]);
		}
	}

	for (int i = 0; i < rfs2.size(); ++i) {
		QMap<RoadEdgeDesc, bool> edges;
		findOneRadial(roads, area, roadType, angleThreshold3, votingRatioThreshold, seedDistance, minSeedDirections, extendingAngleThreshold, *rfs2[i], edges);

		// 残したエッジから周辺のエッジを辿り、方向がほぼ同じなら、候補に登録していく
		extendRadialGroup(roads, area, roadType, *rfs2[i], edges, extendingAngleThreshold, votingRatioThreshold);

		buildRadialArea(roads, edges, *rfs2[i]);

		// 特徴量として正式に登録する
		roadFeature.addFeature(rfs2[i]);

		// 最後に、候補エッジを、実際にグループに登録する
		for (QMap<RoadEdgeDesc, bool>::iterator it = edges.begin(); it != edges.end(); ++it) {
			RoadEdgeDesc e = it.key();
			roads.graph[e]->properties["shapeType"] = RoadEdge::SHAPE_RADIAL;
			roads.graph[e]->properties["group"] = rfs2[i]->group_id;
			roads.graph[e]->properties["gridness"] = 0;
			roads.graph[e]->color = QColor(0, 255, 0);
		}

		roads.setModified();
	}
}


/**
 * Circleを１つ検知し、円の中心を返却する
 * Hough transformにより、円を検知する。
 */
QVector2D RoadSegmentationUtil::detectOneCircleCenter(RoadGraph& roads, Polygon2D& area, float scale, float sigma) {
	HoughTransform ht(area, scale);

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;
		
		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		for (int i = 0; i < roads.graph[*ei]->polyline.size() - 1; i++) {
			ht.line(roads.graph[*ei]->polyline[i], roads.graph[*ei]->polyline[i + 1], sigma);
		}
	}

	return ht.maxPoint();
}