#include "WarpRoadGenerator.h"
#include <QTest>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include "global.h"
#include "Util.h"
#include "ConvexHull.h"
#include "GraphUtil.h"
#include "RoadGeneratorHelper.h"

void WarpRoadGenerator::generateRoadNetwork(bool animation) {
	srand(12345);

	std::list<RoadVertexDesc> seeds;

	// Avenueのシードを生成
	generateAvenueSeeds(seeds);

	// Avenueを生成
	{
		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numAvenueIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = 0.0f;//terrain->getValue(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < 0.0f || z > 100.0f) {
				std::cout << "attemptExpansion (avenue): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << i << " (Seed: " << desc << ")" << std::endl;
			if (roads.graph[desc]->properties["generation_type"] == "example") {
				attemptExpansion(RoadEdge::TYPE_AVENUE, desc, seeds);
			} else {
				attemptExpansion2(RoadEdge::TYPE_AVENUE, desc, seeds);
			}

			if (animation) {
				mainWin->glWidget->updateGL();
				QTest::qWait(10);
			}
		}
	}

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		GraphUtil::removeSelfIntersectingRoads(roads);
		RoadGeneratorHelper::extendDanglingEdges(roads);
		RoadGeneratorHelper::removeDeadend(roads);
		//GraphUtil::reduce(roads);
		//GraphUtil::removeLoop(roads);

		if (animation) {
			mainWin->glWidget->updateGL();
			QTest::qWait(10);
		}
	}

	seeds.clear();

	// Local streetを生成
	if (G::getBool("generateLocalStreets")) {
		generateStreetSeeds(seeds);
		
		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numStreetIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = 0.0f;//terrain->getValue(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < 0.0f || z > 100.0f) {
				std::cout << "attemptExpansion (street): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (street): " << i << " (Seed: " << desc << ")" << std::endl;
			if (roads.graph[desc]->properties["generation_type"] == "example") {
				attemptExpansion(RoadEdge::TYPE_STREET, desc, seeds);
			} else {
				attemptExpansion2(RoadEdge::TYPE_STREET, desc, seeds);
			}

			if (animation) {
				mainWin->glWidget->updateGL();
				QTest::qWait(10);
			}
		}
	}

	// 指定されたエリアでCropping
	if (G::getBool("cropping")) {
		GraphUtil::extractRoads2(roads, targetArea);
	}

	// Local Streetsをクリーンナップ
	if (G::getBool("cleanStreets")) {
		GraphUtil::removeSelfIntersectingRoads(roads);
		RoadGeneratorHelper::removeDeadend(roads);
	}

	// クリーンアップ
	GraphUtil::removeSelfIntersectingRoads(roads);
	GraphUtil::clean(roads);
	GraphUtil::normalizeLoop(roads);

	RoadGeneratorHelper::check(roads);
}

/**
 * シード頂点を生成する。
 */
void WarpRoadGenerator::generateAvenueSeeds(std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	for (int i = 0; i < hintLine.size(); ++i) {
		float angle;

		if (i > 0 && i < hintLine.size() - 1) {
			QVector2D ex_vec = feature.hintLine[i] - feature.hintLine[i - 1];
			QVector2D vec = hintLine[i] - hintLine[i - 1];

			float angle1 = Util::diffAngle(vec, ex_vec, false);

			ex_vec = feature.hintLine[i + 1] - feature.hintLine[i];
			vec = hintLine[i + 1] - hintLine[i];

			float angle2 = Util::diffAngle(vec, ex_vec, false);

			angle = (angle1 + angle2) * 0.5f;
		} else if (i > 0) {
			QVector2D ex_vec = feature.hintLine[i] - feature.hintLine[i - 1];
			QVector2D vec = hintLine[i] - hintLine[i - 1];

			angle = Util::diffAngle(vec, ex_vec, false);
		} else {
			QVector2D ex_vec = feature.hintLine[i + 1] - feature.hintLine[i];
			QVector2D vec = hintLine[i + 1] - hintLine[i];

			angle = Util::diffAngle(vec, ex_vec, false);
		}

		addAvenueSeed(hintLine[i], feature.hintLine[i], i, angle, seeds);
	}
}

/**
 * Avenue用のシードを座標pt付近に追加する。
 * 座標pt付近の、該当するカーネルを捜し、そのカーネルを使ってシードを追加する。
 *
 * @param area			ターゲット領域
 * @param f				特徴量
 * @param pt			シード座標
 * @param seeds			追加されたシードは、seedsに追加される。
 */
bool WarpRoadGenerator::addAvenueSeed(const QVector2D &pt, const QVector2D &ex_pt, int group_id, float angle, std::list<RoadVertexDesc>& seeds) {
	if (!targetArea.contains(pt)) return false;

	// Avenueカーネルの中で、offsetの位置に最も近いものを探す
	RoadVertexDesc seedDesc = GraphUtil::getVertex(feature.reducedRoads(RoadEdge::TYPE_AVENUE), ex_pt);

	// 頂点を追加し、シードとする
	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	roads.graph[desc]->properties["used"] = true;
	roads.graph[desc]->properties["group_id"] = group_id;
	roads.graph[desc]->properties["generation_type"] = "example";
	roads.graph[desc]->properties["example_desc"] = seedDesc;
	roads.graph[desc]->properties["angle"] = angle;
	seeds.push_back(desc);

	return true;
}

/**
 * Local Street用のシードを生成する。
 */
void WarpRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
	seeds.clear();

	int vertex_num = boost::num_vertices(roads.graph);

	// エッジ上に、Local Street用のシードを生成する
	{
		int i = 0;
		int num = GraphUtil::getNumEdges(roads);
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = edges(roads.graph); ei != eend && i < num; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			i++;

			RoadEdgePtr edge = roads.graph[*ei];
			RoadEdgeDesc e = *ei;
			RoadEdgeDesc e1, e2;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);
		
			if (roads.graph[e]->properties["generation_type"] == "example" && (roads.graph[src]->properties["generation_type"] == "example" || roads.graph[tgt]->properties["generation_type"] == "example")) {
				// ターゲットエリア座標空間から、Example座標空間へのオフセットを計算
				QVector2D offset;
				int group_id;
				float angle;
				QVector2D ex_orig;
				if (roads.graph[src]->properties["generation_type"] == "example") {
					RoadVertexDesc ex_v_desc = roads.graph[src]->properties["example_desc"].toUInt();
					group_id = roads.graph[src]->properties["group_id"].toInt();
					angle = roads.graph[src]->properties["angle"].toFloat();
					ex_orig = feature.reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt;
					offset = feature.reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt - roads.graph[src]->pt;
				} else {
					RoadVertexDesc ex_v_desc = roads.graph[tgt]->properties["example_desc"].toUInt();
					group_id = roads.graph[tgt]->properties["group_id"].toInt();
					angle = roads.graph[src]->properties["angle"].toFloat();
					ex_orig = feature.reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt;
					offset = feature.reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt - roads.graph[tgt]->pt;
				}
			
				while (edge->polyline.size() > 2) {
					// このエッジ上の各点について、ほぼ同じ位置のStreetカーネルが存在するか探す
					int index = -1;
					bool found = false;
					RoadVertexDesc seedDesc;
					for (int p_id = 1; p_id < edge->polyline.size() - 1; ++p_id) {
						found = false;

						// この点が、エリア外なら、スキップ
						if (!targetArea.contains(edge->polyline[p_id])) continue;

						// この点の、Example座標空間での位置を計算する
						BBox bbox;
						QVector2D pt = edge->polyline[p_id] + offset;
						pt = Util::rotate(pt, -angle, ex_orig);
										
						if (GraphUtil::getVertex(feature.reducedRoads(RoadEdge::TYPE_STREET), pt, 1.0f, seedDesc)) {
							found = true;
							index = p_id;
							break;
						}
					}

					// このエッジ上に、ほぼ同じ位置のStreetカーネルが存在しない場合は、終了
					if (!found) break;

					RoadVertexDesc v_desc = GraphUtil::splitEdge(roads, e, edge->polyline[index], e1, e2);

					// 端点に近すぎる場合は、シードとするのを中止する
					if (v_desc == src || v_desc == tgt) break;

					// シードとして追加
					seeds.push_back(v_desc);
					roads.graph[v_desc]->properties["group_id"] = group_id;
					roads.graph[v_desc]->properties["example_street_desc"] = seedDesc;
					roads.graph[v_desc]->properties["generation_type"] = "example";
					roads.graph[v_desc]->properties["angle"] = angle;

					// エッジを更新
					edge = roads.graph[e2];
					e = e2;
				}
			} else {
				// 両端の頂点から、group_idを特定
				int group_id = roads.graph[src]->properties["group_id"].toInt();

				int step;
				if (roads.graph[e]->polyline.length() > feature.length(RoadEdge::TYPE_STREET) * 5) {
					step = roads.graph[e]->polyline.size() / 5;
				} else {
					step = roads.graph[e]->polyline.size() / 2;
				}
				if (step <= 1) continue;
		
				while (step < edge->polyline.size() - step) {
					RoadVertexDesc desc = GraphUtil::splitEdge(roads, e, edge->polyline[step], e1, e2);
					roads.graph[desc]->properties["group_id"] = group_id;
					roads.graph[desc]->properties["generation_type"] = "pm";

					// この点が、エリア内なら、シードとして追加
					if (targetArea.contains(edge->polyline[step])) {
						seeds.push_back(desc);
					}

					edge = roads.graph[e2];
					e = e2;
				}
			}
		}
	}

	// 頂点自体を、Local streetsのシードにする
	{
		int i = 0;
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend && i < vertex_num; ++vi, ++i) {
			if (!roads.graph[*vi]->valid) continue;

			if (roads.graph[*vi]->properties["generation_type"] == "example") {
				RoadVertexDesc ex_v_desc = roads.graph[*vi]->properties["example_desc"].toUInt();

				RoadVertexDesc seedDesc;
				if (GraphUtil::getVertex(feature.reducedRoads(RoadEdge::TYPE_STREET), feature.reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt, 1.0f, seedDesc)) {
					// シードとして追加
					seeds.push_back(*vi);
					roads.graph[*vi]->properties["example_street_desc"] = seedDesc;
				}
			} else {
				if (GraphUtil::getDegree(roads, *vi) <= 2) {
					// シードとして追加
					//seeds.push_back(*vi);
				}
			}
		}
	}
}

/**
 * このシードを使って、道路生成する。
 * Exampleベースで生成する。
 */
void WarpRoadGenerator::attemptExpansion(int roadType, RoadVertexDesc srcDesc, std::list<RoadVertexDesc> &seeds) {
	RoadVertexDesc ex_v_desc;
	if (roadType == RoadEdge::TYPE_AVENUE) {
		ex_v_desc = roads.graph[srcDesc]->properties["example_desc"].toUInt();
	} else {
		ex_v_desc = roads.graph[srcDesc]->properties["example_street_desc"].toUInt();
	}
	float angle = roads.graph[srcDesc]->properties["angle"].toFloat();
	RoadVertexPtr ex_vertex = feature.reducedRoads(roadType).graph[ex_v_desc];

	float roadSnapFactor = G::getFloat("roadSnapFactor");
	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(ex_v_desc, feature.reducedRoads(roadType).graph); ei != eend; ++ei) {
		RoadVertexDesc tgt = boost::target(*ei, feature.reducedRoads(roadType).graph);

		Polyline2D polyline = GraphUtil::orderPolyLine(feature.reducedRoads(roadType), *ei, ex_v_desc);

		QVector2D offset = polyline[0];
		polyline.translate(offset * -1.0f);
		polyline.rotate(-Util::rad2deg(angle));

		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, polyline, 0.3f)) continue;

		growRoadSegment(roadType, srcDesc, polyline, feature.reducedRoads(roadType).graph[*ei]->lanes, tgt, true, roadSnapFactor, roadAngleTolerance, seeds);
	}
}

/**
 * このシードを使って、PM方式で道路生成する。
 */
void WarpRoadGenerator::attemptExpansion2(int roadType, RoadVertexDesc srcDesc, std::list<RoadVertexDesc> &seeds) {
	float snapThreshold;

	if (roadType == RoadEdge::TYPE_AVENUE) {
		snapThreshold = feature.avgAvenueLength * 0.2f;
	} else {
		snapThreshold = feature.avgStreetLength * 0.2f;
	}

	// この頂点に、exampleベースのedgeが１つでもあるか？
	bool exampleEdge = false;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (roads.graph[*ei]->properties["generation_type"] == "example") {
				exampleEdge = true;
				break;
			}
		}
	}

	// 当該シードに、roadTypeよりも上位レベルの道路セグメントが接続されているか、チェックする
	bool isConnectedByUpperLevelRoadSegment = false;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (roads.graph[*ei]->type > roadType) {
				isConnectedByUpperLevelRoadSegment = true;
				break;
			}
		}
	}

	// 当該頂点に接続されたエッジが１つかどうか？また、そのエッジを取得
	bool isConnectedByOneRoadSegment = true;
	RoadEdgeDesc e_desc;
	{
		bool flag = false;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (flag) {
				isConnectedByOneRoadSegment = false;
				break;
			} else {
				e_desc = *ei;
				flag = true;
			}
		}
	}

	if (!exampleEdge && !isConnectedByUpperLevelRoadSegment && isConnectedByOneRoadSegment) {
		// 当該頂点の近くに他の頂点があれば、スナップさせる
		RoadVertexDesc tgtDesc;
		if (RoadGeneratorHelper::canSnapToVertex(roads, srcDesc, snapThreshold, tgtDesc)) {
			GraphUtil::snapVertex(roads, srcDesc, tgtDesc);
			return;
		}

		// 近くに他のエッジがあれば、スナップさせる
		RoadEdgeDesc closeEdge;
		QVector2D closestPt;
		if (RoadGeneratorHelper::canSnapToEdge(roads, srcDesc, snapThreshold, closeEdge, closestPt)) {
			tgtDesc = GraphUtil::splitEdge(roads, closeEdge, closestPt);
			roads.graph[tgtDesc]->properties["generation_type"] = "pm";
			roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closeEdge]->properties["group_id"];
			roads.graph[tgtDesc]->properties["parent"] = srcDesc;

			GraphUtil::snapVertex(roads, srcDesc, tgtDesc);
			return;
		}
	}

	// 道路生成用のカーネルを合成する
	std::vector<RoadEdgePtr> edges;
	synthesizeItem(roadType, srcDesc, edges);
	
	float roadSnapFactor = G::getFloat("roadSnapFactor");
	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	for (int i = 0; i < edges.size(); ++i) {
		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, edges[i]->polyline, roadAngleTolerance)) continue;
		growRoadSegment(roadType, srcDesc, edges[i]->polyline, 1, 0, false, roadSnapFactor, roadAngleTolerance, seeds);
	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 */
bool WarpRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, const Polyline2D &polyline, int lanes, RoadVertexDesc next_ex_v_desc, bool byExample, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	bool intercepted = false;

	// 新しいエッジを生成
	RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
	for (int i = 0; i < polyline.size(); ++i) {
		QVector2D pt = roads.graph[srcDesc]->pt + polyline[i];

		new_edge->polyline.push_back(pt);

		// 水没、または、山の上なら、道路生成をストップ
		float z = 0.0f;//terrain->getValue(pt.x(), pt.y());
		if (z < 0.0f || z > 100.0f) {
			// 最初っから水没している場合は、そもそもエッジ生成をキャンセル
			if (new_edge->polyline.size() <= 1) return false;

			RoadGeneratorHelper::cutEdgeBySteepElevationChange(new_edge->polyline);//, terrain);

			intercepted = true;
			break;
		}

		// 他のエッジと交差したら、道路生成をストップ
		QVector2D intPoint;
		//if (roadType == RoadEdge::TYPE_STREET && GraphUtil::isIntersect(roads, new_edge->polyline, intPoint)) {
		if (GraphUtil::isIntersect(roads, new_edge->polyline, intPoint)) {
			new_edge->polyline.erase(new_edge->polyline.begin() + new_edge->polyline.size() - 1);
			new_edge->polyline.push_back(intPoint);

			intercepted = true;

			// エッジ長が最短thresholdより短い場合は、キャンセル
			if (new_edge->polyline.length() < 30.0f) return false;

			break;
		}
	}

	if (new_edge->polyline.size() == 1) return false;

	RoadVertexDesc tgtDesc;

	if (byExample) {
		//snapFactor = 0.01f;
		snapFactor *= 0.4f;
	}

	// スナップできるか？
	RoadEdgeDesc closestEdge;
	QVector2D intPoint;
	if ((new_edge->polyline.last() - roads.graph[srcDesc]->pt).lengthSquared() < 0.1f) {
		// ループエッジ
		tgtDesc = srcDesc;
	} else if (GraphUtil::getVertex(roads, new_edge->polyline.last(), new_edge->polyline.length() * snapFactor, srcDesc, tgtDesc)) {
		if (byExample && roads.graph[tgtDesc]->properties["generation_type"] == "example" && roads.graph[tgtDesc]->properties["group_id"] == roads.graph[srcDesc]->properties["group_id"]) {
			angleTolerance = 0.3f;
		}

		// 他の頂点にスナップ
		GraphUtil::movePolyline(roads, new_edge->polyline, roads.graph[srcDesc]->pt, roads.graph[tgtDesc]->pt);
		std::reverse(new_edge->polyline.begin(), new_edge->polyline.end());
		if (GraphUtil::hasRedundantEdge(roads, tgtDesc, new_edge->polyline, angleTolerance)) return false;
	} else if (GraphUtil::getEdge(roads, new_edge->polyline.last(), new_edge->polyline.length() * snapFactor, srcDesc, closestEdge, intPoint)) {
		if (byExample && roads.graph[closestEdge]->properties["generation_type"] == "example" && roads.graph[tgtDesc]->properties["group_id"] == roads.graph[srcDesc]->properties["group_id"]) {
			angleTolerance = 0.3f;
		}

		// 他のエッジにスナップ
		tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
		roads.graph[tgtDesc]->properties["generation_type"] = "pm";
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];

		GraphUtil::movePolyline(roads, new_edge->polyline, roads.graph[srcDesc]->pt, roads.graph[tgtDesc]->pt);
		if (GraphUtil::hasRedundantEdge(roads, tgtDesc, new_edge->polyline, angleTolerance)) return false;
	} else {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.last()));
		tgtDesc = GraphUtil::addVertex(roads, v);
		roads.graph[tgtDesc]->properties["parent"] = srcDesc;

		// 新しい頂点にgroup_idを引き継ぐ
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];

		// 新しい頂点のgeneration_typeを設定
		roads.graph[tgtDesc]->properties["generation_type"] = byExample ? "example" : "pm";

		if (targetArea.contains(new_edge->polyline.last())) {
			roads.graph[tgtDesc]->properties["generation_type"] = "pm";

			// シードに追加する
			if (byExample && !intercepted) {	// Exampleベースで、且つ、途中で生成ストップしていない場合（つまり、Exampleそのまま）
				if (roadType == RoadEdge::TYPE_AVENUE || GraphUtil::getDegree(feature.reducedRoads(roadType), next_ex_v_desc) > 1) {
					seeds.push_back(tgtDesc);
				}

				// 対応するExampleが存在する場合は、それを設定する
				if (!feature.reducedRoads(roadType).graph[next_ex_v_desc]->properties.contains("used") && GraphUtil::getDegree(feature.reducedRoads(roadType), next_ex_v_desc) > 1) {
					feature.reducedRoads(roadType).graph[next_ex_v_desc]->properties["used"] = true;
					roads.graph[tgtDesc]->properties["generation_type"] = "example";
					if (roadType == RoadEdge::TYPE_AVENUE) {
						roads.graph[tgtDesc]->properties["example_desc"] = next_ex_v_desc;
					} else {
						roads.graph[tgtDesc]->properties["example_street_desc"] = next_ex_v_desc;
					}
					roads.graph[tgtDesc]->properties["angle"] = roads.graph[srcDesc]->properties["angle"];
				}
			} else {
				seeds.push_back(tgtDesc);
			}
		} else {
			// ターゲットエリアの外に出たら
			if (byExample) {
				roads.graph[tgtDesc]->properties["generation_type"] = "example";
				if (roadType == RoadEdge::TYPE_AVENUE) {
					roads.graph[tgtDesc]->properties["example_desc"] = next_ex_v_desc;
				} else {
					roads.graph[tgtDesc]->properties["example_street_desc"] = next_ex_v_desc;
				}
			} else {
				roads.graph[tgtDesc]->properties["generation_type"] = "pm";
			}

			roads.graph[tgtDesc]->onBoundary = true;
		}

		// Example道路のDeadendは、Deadendとして正式に登録する
		if (byExample && GraphUtil::getDegree(feature.reducedRoads(roadType), next_ex_v_desc) == 1) {
			roads.graph[tgtDesc]->properties["deadend"] = true;
		}
	}

	RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, tgtDesc, new_edge);

	// 新しいエッジにgroup_idを引き継ぐ
	roads.graph[e_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];

	// 新しいエッジのgeneration_typeを設定
	roads.graph[e_desc]->properties["generation_type"] = byExample ? "example" : "pm";

	return true;
}

/**
 * PMに従って、カーネルを合成する
 */
void WarpRoadGenerator::synthesizeItem(int roadType, RoadVertexDesc v_desc, std::vector<RoadEdgePtr> &edges) {
	// 当該頂点から出るエッジをリストアップする
	std::vector<Polyline2D> polylines;
	QList<RoadVertexDesc> neighbors;
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc tgt = boost::target(*ei, roads.graph);
		neighbors.push_back(tgt);

		if ((roads.graph[v_desc]->pt - roads.graph[*ei]->polyline[0]).lengthSquared() > (roads.graph[tgt]->pt - roads.graph[*ei]->polyline[0]).lengthSquared()) {
			std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
		}
		polylines.push_back(roads.graph[*ei]->polyline);
	}

	float direction = 0.0f;

	if (polylines.size() > 0) {
		QVector2D vec = polylines[0][1] - polylines[0][0];
		direction = atan2f(vec.y(), vec.x());
	}

	// 直近の頂点で、example_descを持つ頂点を探し、そのexample空間座標からの相対座標を使って、example空間座標を計算する
	RoadVertexDesc nearest_v_desc = RoadGeneratorHelper::getClosestVertexByExample(roads, v_desc);
	RoadVertexDesc ex_desc = roads.graph[nearest_v_desc]->properties["example_desc"].toUInt();
	QVector2D pt = feature.reducedRoads(roadType).graph[ex_desc]->pt + roads.graph[v_desc]->pt - roads.graph[nearest_v_desc]->pt;

	RoadGeneratorHelper::createFourEdges(feature, roadType, pt, 1, direction, 10.0f, edges);
}
