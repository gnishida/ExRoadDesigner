#include "AliagaRoadGenerator.h"
#include "GraphUtil.h"
#include "Util.h"

void AliagaRoadGenerator::generateRoadNetwork() {
	// 全ての頂点をExからコピー
	for (int i = 0; i < features.size(); ++i) {
		// avenue exからコピー
		{
			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(features[i].roads(RoadEdge::TYPE_AVENUE).graph); vi != vend; ++vi) {
				if (!features[i].roads(RoadEdge::TYPE_AVENUE).graph[*vi]->valid) continue;

				QVector2D pt = features[i].roads(RoadEdge::TYPE_AVENUE).graph[*vi]->pt - features[i].hintLine[0] + hintLine[i];

				// エリア外ならスキップ
				if (!targetArea.contains(pt)) continue;

				// 他のシードの方が近い場合は、スキップ
				{
					bool skip = false;
					for (int j = 0; j < hintLine.size(); ++j) {
						if (i == j) continue;

						if ((hintLine[j] - pt).lengthSquared() < 
							(hintLine[i] - pt).lengthSquared()) {
								skip = true;
						}
					}

					if (skip) continue;
				}

				// 頂点を生成
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));// + QVector2D(Util::genRand(-20, 20), Util::genRand(-20, 20))));
				v->properties["ex_id"] = i;
				v->properties["example_desc"] = *vi;
				v->type = RoadEdge::TYPE_AVENUE;
				GraphUtil::addVertex(roads, v);
			}
		}

		// street exからコピー
		/*
		{
			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(features[i].roads(RoadEdge::TYPE_STREET).graph); vi != vend; ++vi) {
				if (!features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->valid) continue;

				QVector2D pt = features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->pt - features[i].hintLine[0] + hintLine[i];

				// エリア外ならスキップ
				if (!targetArea.contains(pt)) continue;

				// 他のシードの方が近い場合は、スキップ
				{
					bool skip = false;
					for (int j = 0; j < hintLine.size(); ++j) {
						if (i == j) continue;

						if ((hintLine[j] - pt).lengthSquared() < 
							(hintLine[i] - pt).lengthSquared()) {
								skip = true;
						}
					}

					if (skip) continue;
				}

				// 既に同じ場所に頂点があれば、example_street_descをセットして、スキップ
				RoadVertexDesc desc;
				if (GraphUtil::getVertex(roads, pt, 1.0f, desc)) {
					if (roads.graph[desc]->properties["ex_id"].toInt() == i) {
						roads.graph[desc]->properties["example_street_desc"] = *vi;
					}
					continue;
				}

				// 頂点を生成
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
				v->properties["ex_id"] = i;
				v->properties["example_street_desc"] = *vi;
				v->type = RoadEdge::TYPE_STREET;
				GraphUtil::addVertex(roads, v);
			}
		}
		*/
	}

	// エッジを生成
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			int ex_id = roads.graph[*vi]->properties["ex_id"].toInt();

			bool avenue = false;
			RoadVertexDesc ex_desc;
			if (roads.graph[*vi]->properties.contains("example_desc")) {
				ex_desc = roads.graph[*vi]->properties["example_desc"].toUInt();
				avenue = true;
			}

			bool street = false;
			RoadVertexDesc ex_street_desc;
			if (roads.graph[*vi]->properties.contains("example_street_desc")) {
				ex_street_desc = roads.graph[*vi]->properties["example_street_desc"].toUInt();
				street = true;
			}

			if (avenue) {
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(ex_desc, features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph); ei != eend; ++ei) {
					RoadVertexDesc ex_tgt = boost::target(*ei, features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph);

					if (GraphUtil::getDegree(roads, *vi) >= 4) break;

					QVector2D dir = features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph[ex_tgt]->pt - features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph[ex_desc]->pt;
					float length = features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph[*ei]->polyline.length();

					RoadVertexDesc nearestDesc;
					float angle = atan2f(dir.y(), dir.x()) + Util::genRandNormal(0.0f, 0.08f);

					if (!GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 0.3f, nearestDesc)) {
						if (!GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 0.7f, nearestDesc)) {
							GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 3.0f, nearestDesc);
						}
					}

					{
						if (GraphUtil::hasEdge(roads, *vi, nearestDesc)) continue;

						if (GraphUtil::getDegree(roads, nearestDesc) >= 4) continue;

						// polylineの生成
						RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(RoadEdge::TYPE_AVENUE, 1));
						edge->polyline = generatePolyline(roads.graph[*vi]->pt, roads.graph[nearestDesc]->pt, length / dir.length());

						// 他の既存エッジと交差するなら、キャンセル
						RoadEdgeDesc nearestEdge;
						QVector2D intPoint;
						if (GraphUtil::isIntersect(roads, edge->polyline, *vi, nearestEdge, intPoint)) {
							continue;
						}
						

						// エッジ生成
						GraphUtil::addEdge(roads, *vi, nearestDesc, edge);
					}
				}
			}

			/*
			if (street) {
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(ex_street_desc, features[ex_id].roads(RoadEdge::TYPE_STREET).graph); ei != eend; ++ei) {
					RoadVertexDesc ex_tgt = boost::target(*ei, features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph);

					QVector2D dir = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_tgt]->pt - features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_street_desc]->pt;
					float length = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[*ei]->polyline.length();

					RoadVertexDesc nearestDesc;
					if (GraphUtil::getVertex(roads, *vi, dir.length() * 1.2f, atan2f(dir.y(), dir.x()), 0.3f, nearestDesc)) {
						if (GraphUtil::hasEdge(roads, *vi, nearestDesc)) continue;

						// polylineの生成
						RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(RoadEdge::TYPE_STREET, 1));
						edge->polyline = generatePolyline(roads.graph[*vi]->pt, roads.graph[nearestDesc]->pt, length);

						// 他の既存エッジと交差するなら、そこにスナップ
						RoadEdgeDesc nearestEdge;
						QVector2D intPoint;
						if (GraphUtil::isIntersect(roads, edge->polyline, *vi, nearestEdge, intPoint)) {
							edge->polyline = Polyline2D();
							edge->polyline.push_back(roads.graph[*vi]->pt);
							edge->polyline.push_back(intPoint);
						}
						

						// エッジ生成
						GraphUtil::addEdge(roads, *vi, nearestDesc, edge);
					}
				}
			}
			*/
		}
	}


	/////////////////////////////////////////////////
	// local street
	// 全ての頂点をstreet Exからコピー
	for (int i = 0; i < features.size(); ++i) {
		{
			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(features[i].roads(RoadEdge::TYPE_STREET).graph); vi != vend; ++vi) {
				if (!features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->valid) continue;

				QVector2D pt = features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->pt - features[i].hintLine[0] + hintLine[i];

				// エリア外ならスキップ
				if (!targetArea.contains(pt)) continue;

				// 他のシードの方が近い場合は、スキップ
				{
					bool skip = false;
					for (int j = 0; j < hintLine.size(); ++j) {
						if (i == j) continue;

						if ((hintLine[j] - pt).lengthSquared() < 
							(hintLine[i] - pt).lengthSquared()) {
								skip = true;
						}
					}

					if (skip) continue;
				}

				// 既に同じ場所に頂点があれば、example_street_descをセットして、スキップ
				RoadVertexDesc desc;
				if (GraphUtil::getVertex(roads, pt, 1.0f, desc)) {
					if (roads.graph[desc]->properties["ex_id"].toInt() == i) {
						roads.graph[desc]->properties["example_street_desc"] = *vi;
					}
					continue;
				}

				// 頂点を生成
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));// + QVector2D(Util::genRand(-4, 4), Util::genRand(-4, 4))));
				v->properties["ex_id"] = i;
				v->properties["example_street_desc"] = *vi;
				v->type = RoadEdge::TYPE_STREET;
				GraphUtil::addVertex(roads, v);
			}
		}
		
		/*
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(features[i].roads(RoadEdge::TYPE_STREET).graph); vi != vend; ++vi) {
			if (!features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->valid) continue;

			QVector2D pt = features[i].roads(RoadEdge::TYPE_STREET).graph[*vi]->pt - features[i].hintLine[0] + hintLine[i];

			// エリア外ならスキップ
			if (!targetArea.contains(pt)) continue;

			// 他のシードの方が近い場合は、スキップ
			{
				bool skip = false;
				for (int j = 0; j < hintLine.size(); ++j) {
					if (i == j) continue;

					if ((hintLine[j] - pt).lengthSquared() < 
						(hintLine[i] - pt).lengthSquared()) {
							skip = true;
					}
				}

				if (skip) continue;
			}

			// 既に同じ場所に頂点があれば、example_street_descをセットして、スキップ
			RoadVertexDesc desc;
			if (GraphUtil::getVertex(roads, pt, 1.0f, desc)) {
				if (roads.graph[desc]->properties["ex_id"].toInt() == i) {
					roads.graph[desc]->properties["example_street_desc"] = *vi;
				}
				continue;
			}

			// 頂点を生成
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
			v->properties["ex_id"] = i;
			v->properties["example_street_desc"] = *vi;
			v->type = RoadEdge::TYPE_STREET;
			GraphUtil::addVertex(roads, v);
		}
		*/
	}

	// エッジを生成
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			int ex_id = roads.graph[*vi]->properties["ex_id"].toInt();

			bool street = false;
			RoadVertexDesc ex_street_desc;
			if (roads.graph[*vi]->properties.contains("example_street_desc")) {
				ex_street_desc = roads.graph[*vi]->properties["example_street_desc"].toUInt();
				street = true;
			}

			if (street) {
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(ex_street_desc, features[ex_id].roads(RoadEdge::TYPE_STREET).graph); ei != eend; ++ei) {
					RoadVertexDesc ex_tgt = boost::target(*ei, features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph);

					if (GraphUtil::getDegree(roads, *vi) >= 4) break;

					QVector2D dir = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_tgt]->pt - features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_street_desc]->pt;
					float length = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[*ei]->polyline.length();

					RoadVertexDesc nearestDesc;
					float angle = atan2f(dir.y(), dir.x()) + Util::genRandNormal(0.0f, 0.08f);

					if (!GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 0.3f, nearestDesc)) {
						if (!GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 0.7f, nearestDesc)) {
							GraphUtil::getVertex(roads, *vi, dir.length() * 2.0f, angle, 3.0f, nearestDesc);
						}
					}

					{
						if (GraphUtil::hasEdge(roads, *vi, nearestDesc)) continue;

						if (GraphUtil::getDegree(roads, nearestDesc) >= 4) continue;

						// polylineの生成
						RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(RoadEdge::TYPE_STREET, 1));
						edge->polyline = generatePolyline(roads.graph[*vi]->pt, roads.graph[nearestDesc]->pt, length / dir.length());

						// 他の既存エッジと交差するなら、キャンセル
						RoadEdgeDesc nearestEdge;
						QVector2D intPoint;
						if (GraphUtil::isIntersect(roads, edge->polyline, *vi, nearestEdge, intPoint)) {
							continue;
						}
						

						// エッジ生成
						GraphUtil::addEdge(roads, *vi, nearestDesc, edge);
					}
				}
			}

			/*
			if (street) {
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(ex_street_desc, features[ex_id].roads(RoadEdge::TYPE_STREET).graph); ei != eend; ++ei) {
					RoadVertexDesc ex_tgt = boost::target(*ei, features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph);

					QVector2D dir = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_tgt]->pt - features[ex_id].roads(RoadEdge::TYPE_STREET).graph[ex_street_desc]->pt;
					float length = features[ex_id].roads(RoadEdge::TYPE_STREET).graph[*ei]->polyline.length();

					RoadVertexDesc nearestDesc;
					if (GraphUtil::getVertex(roads, *vi, dir.length() * 1.2f, atan2f(dir.y(), dir.x()), 0.3f, nearestDesc)) {
						if (GraphUtil::hasEdge(roads, *vi, nearestDesc)) continue;

						// polylineの生成
						RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(RoadEdge::TYPE_STREET, 1));
						edge->polyline = generatePolyline(roads.graph[*vi]->pt, roads.graph[nearestDesc]->pt, length);

						// 他の既存エッジと交差するなら、そこにスナップ
						RoadEdgeDesc nearestEdge;
						QVector2D intPoint;
						if (GraphUtil::isIntersect(roads, edge->polyline, *vi, nearestEdge, intPoint)) {
							edge->polyline.back() = intPoint;
						}
						

						// エッジ生成
						GraphUtil::addEdge(roads, *vi, nearestDesc, edge);
					}
				}
			}
			*/
		}
	}
}

Polyline2D AliagaRoadGenerator::generatePolyline(const QVector2D& p1, const QVector2D& p2, float length_ratio) {
	float length = (p1 - p2).length() * length_ratio;

	Polyline2D min_polyline;
	min_polyline.push_back(p1);
	min_polyline.push_back(p1 * 0.66666f + p2 * 0.33333f);
	min_polyline.push_back(p1 * 0.33333f + p2 * 0.66666f);
	min_polyline.push_back(p2);

	QVector2D dir = (p2 - p1).normalized();
	QVector2D pdir;
	pdir.setX(-dir.y());
	pdir.setY(dir.x());

	float min_diff = fabs(min_polyline.length() - length);

	for (int loop = 0; loop < 100; ++loop) {
		QVector2D pt2 = p1 * 0.66666f + p2 * 0.33333f;
		QVector2D pt3 = p1 * 0.33333f + p2 * 0.66666f;

		if (Util::genRand(0, 1) > 0.5f) {
			pt2 += pdir * Util::genRand(0, length * 0.3f);
			pt3 += pdir * Util::genRand(0, length * 0.3f);
		} else {
			pt2 += pdir * Util::genRand(-length * 0.3f, 0);
			pt3 += pdir * Util::genRand(-length * 0.3f, 0);
		}

		Polyline2D polyline;
		polyline.push_back(p1);
		polyline.push_back(pt2);
		polyline.push_back(pt3);
		polyline.push_back(p2);
		if (fabs(polyline.length() - length) < min_diff) {
			min_diff = fabs(polyline.length() - length);
			min_polyline[1] = pt2;
			min_polyline[2] = pt3;
		}
	}

	return min_polyline;
}


