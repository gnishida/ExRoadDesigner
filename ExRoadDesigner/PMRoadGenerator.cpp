#include "PMRoadGenerator.h"
#include <QTest>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include "global.h"
#include "Util.h"
#include "ConvexHull.h"
#include "GraphUtil.h"
#include "RoadGeneratorHelper.h"
#include "SmallBlockRemover.h"
#include "ShapeDetector.h"
#include <assert.h>

void PMRoadGenerator::generateRoadNetwork() {
	srand(12345);

	// mark all the existing vertices as fixed
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			roads.graph[*vi]->fixed = true;
		}
	}

	// remove all the local streets temporalily
	QMap<RoadEdgeDesc, bool> originalLocalStreets;
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			if (roads.graph[*ei]->type == RoadEdge::TYPE_STREET) {
				originalLocalStreets[*ei] = true;
				roads.graph[*ei]->valid = false;
			}
		}
	}

	std::list<RoadVertexDesc> seeds;

	// Avenueのシードを生成
	generateAvenueSeeds(seeds);

	// Avenueを生成
	std::cout << "Avenue generation started." << std::endl;
	{
		int iter;
		for (iter = 0; !seeds.empty() && iter < G::getInt("numAvenueIterations"); ) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			// 水中なら、伸ばして水中から脱出できるなら伸ばす。
			float z = vboRenderManager->getMinTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < G::getFloat("seaLevel")) {
				// 水中の頂点は、degree=1のはず！！
				assert(GraphUtil::getDegree(roads, desc) == 1);
				if (!RoadGeneratorHelper::extendRoadAcrossRiver(roads, vboRenderManager, targetArea, RoadEdge::TYPE_AVENUE, desc, seeds, 0.3f, G::getFloat("acrossRiverTolerance"))) {
					RoadGeneratorHelper::removeEdge(roads, desc);
				}
				continue;
			}

			// エリアの外なら、スキップする
			if (!targetArea.contains(roads.graph[desc]->pt)) {
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << iter << " (Seed: " << desc << ")" << std::endl;
			attemptExpansion(RoadEdge::TYPE_AVENUE, desc, seeds);

			if (G::getBool("saveRoadImages")) {
				char filename[255];
				sprintf(filename, "road_images/avenues_%d.jpg", iter);
				RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);
			}

			iter++;
		}
	}
	std::cout << "Avenue generation completed." << std::endl;

	seeds.clear();

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		RoadGeneratorHelper::removeDanglingEdges(roads);
	}

	// recover the temporarily removed local streets if they are not intersected with other edges
	{
		for (QMap<RoadEdgeDesc, bool>::iterator it = originalLocalStreets.begin(); it != originalLocalStreets.end(); ++it) {
			if (!GraphUtil::isIntersect(roads, roads.graph[it.key()]->polyline)) {
				roads.graph[it.key()]->valid = true;
			}
		}
	}

	// Local streetを生成
	if (G::getBool("generateLocalStreets")) {
		generateStreetSeeds(seeds);
				
		int iter;
		for (iter = 0; !seeds.empty() && iter < G::getInt("numStreetIterations"); ) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			std::cout << "attemptExpansion (street): " << iter << " (Seed: " << desc << ")" << std::endl;

			// エリアの外なら、スキップする
			if (!targetArea.contains(roads.graph[desc]->pt)) {
				continue;
			}

			float z = vboRenderManager->getMinTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < G::getFloat("seaLevel")) {
				// 水中の頂点は、degree=1のはず！！
				if (GraphUtil::getDegree(roads, desc) > 1) {
					printf("ERROR!!! the vertex %d on the river has degree > 1.\n", desc);
				}
				RoadGeneratorHelper::removeEdge(roads, desc);
				continue;
			}

			attemptExpansion(RoadEdge::TYPE_STREET, desc, seeds);

			if (G::getBool("saveRoadImages")) {
				char filename[255];
				sprintf(filename, "road_images/streets_%d.jpg", iter);
				RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);
			}

			iter++;
		}
	}

	// 指定されたエリアでCropping
	if (G::getBool("cropping")) {
		GraphUtil::extractRoads2(roads, targetArea);
	}

	if (G::getBool("cleanStreets")) {
		RoadGeneratorHelper::removeDanglingEdges(roads);
	}

	GraphUtil::cleanEdges(roads);
}

/**
 * シード頂点を生成する。
 */
void PMRoadGenerator::generateAvenueSeeds(std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	for (int i = 0; i < hintLine.size(); ++i) {
		addAvenueSeed(hintLine[i], i, seeds);
	}
}

/**
 * Avenue用のシードを座標pt付近に追加する。
 * 座標pt付近の、該当するカーネルを捜し、そのカーネルを使ってシードを追加する。
 *
 * @param f				特徴量
 * @param pt			シード座標
 * @param seeds			追加されたシードは、seedsに追加される。
 */
bool PMRoadGenerator::addAvenueSeed(const QVector2D &pt, int group_id, std::list<RoadVertexDesc>& seeds) {
	if (!targetArea.contains(pt)) return false;

	// 頂点を追加し、シードとする
	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	roads.graph[desc]->properties["group_id"] = group_id;
	roads.graph[desc]->generationType = "pm";
	roads.graph[desc]->rotationAngle = G::getFloat("rotationAngle");
	seeds.push_back(desc);

	float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
	if (z < G::getFloat("seaLevel")) {
		G::global()["seaLevel"] = z - 0.1f;
	}

	return true;
}

/**
 * Local Street用のシードを生成する。
 */
void PMRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
	seeds.clear();

	// 頂点自体を、Local streetsのシードにする
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			// The existing roads shoule not be used for the local street genration any more.
			if (roads.graph[*vi]->fixed) continue;

			// エリア外なら、スキップ
			if (!targetArea.contains(roads.graph[*vi]->pt)) continue;

			// 水面下なら、スキップ
			if (vboRenderManager->getMinTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y()) < G::getFloat("seaLevel")) continue;

			if (GraphUtil::getDegree(roads, *vi) == 2) {
				seeds.push_back(*vi);
			}
		}
	}
}

/**
 * このシードを使って、PM方式で道路生成する。
 */
void PMRoadGenerator::attemptExpansion(int roadType, RoadVertexDesc srcDesc, std::list<RoadVertexDesc> &seeds) {
	// 要注意！！
	// 既に標高はチェック済みなので、このチェックは不要のはず！！！
	float z = vboRenderManager->getMinTerrainHeight(roads.graph[srcDesc]->pt.x(), roads.graph[srcDesc]->pt.y());
	if (z < G::getFloat("seaLevel")) {
		printf("ERROR! The current vertex should be above sea leve.\n");
		assert(false);
		return;
	}

	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	// 当該頂点から出るエッジの方向を取得する
	float direction = RoadGeneratorHelper::getFirstEdgeAngle(roads, srcDesc);

	for (int i = 0; i < 4; ++i) {
		direction += M_PI * 0.5f;

		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, direction, roadAngleTolerance)) continue;

		
		BBox bbox = targetArea.envelope();
		float D2 = bbox.area();

		std::vector<float> sigma2;
		sigma2.push_back(SQR(G::getDouble("interpolationSigma1")));
		sigma2.push_back(SQR(G::getDouble("interpolationSigma2")));
		for (int i = 2; i < features.size(); ++i) {
			sigma2.push_back(sigma2[0]);
		}

		// 統計情報から、ステップ数、長さ、曲率を決定する
		float interpolated_num_steps = 0.0f;
		float interpolated_step = 0.0f;
		float interpolated_curvature = 0.0f;
		float total = 0.0f;
		for (int i = 0; i < features.size(); ++i) {
			int num_steps;
			float step, curvature;
			if (roadType == RoadEdge::TYPE_AVENUE) {
				num_steps = ceilf(features[i].avgAvenueLength / features[i].avgStreetLength);
				step = features[i].avgStreetLength;
				curvature = features[i].avgAvenueCurvature;
			} else {
				num_steps = 1;
				step = features[i].avgStreetLength;
				curvature = features[i].avgStreetCurvature;
			}

			interpolated_num_steps += (float)num_steps * sigma2[i];
			interpolated_step += step * sigma2[i];
			interpolated_curvature += curvature * sigma2[i];
			total += sigma2[i];
		}
		interpolated_num_steps = (int)ceilf(interpolated_num_steps / total);
		interpolated_step /= total;
		interpolated_curvature /= total;
	
		// 坂が急なら、キャンセル
		QVector2D pt2 = roads.graph[srcDesc]->pt + QVector2D(cosf(direction), sinf(direction)) * 20.0f;
		float z2 = vboRenderManager->getTerrainHeight(pt2.x(), pt2.y());
		if (z2 - z > tanf(G::getFloat("slopeTolerance")) * 20.0f) {
			if (Util::genRand(0, 1) < 0.8f) return;

			// 急勾配を上昇する場合は、直線道路にする
			//uphill = true;
		}

		growRoadSegment(roadType, srcDesc, interpolated_step, interpolated_num_steps, direction, interpolated_curvature, 1, roadAngleTolerance, seeds);
	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 */
bool PMRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, float step, int num_steps, float angle, float curvature, int lanes, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	const int num_sub_steps = 5;
	float sub_step = step / num_sub_steps;
	
	RoadVertexDesc curDesc = srcDesc;
	QVector2D curPt = roads.graph[srcDesc]->pt;
	RoadVertexDesc tgtDesc;
	bool cancel = false;
	for (int iter = 0; iter < num_steps; ++iter) {
		RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
		new_edge->polyline.push_back(roads.graph[curDesc]->pt);

		bool found = false;
		if (RoadGeneratorHelper::getVertexForSnapping(*vboRenderManager, roads, curDesc, step * 2.0f, G::getFloat("seaLevel"), angle, 0.3f, tgtDesc)) {
			found = true;
		}

		if (found) {
			// もしスナップ先が、シードじゃないなら、エッジ生成をキャンセル
			if (std::find(seeds.begin(), seeds.end(), tgtDesc) == seeds.end()) {
				//（要検討。50%の確率ぐらいにすべきか？)
				if (Util::genRand(0, 1) < 0.5f) {
					cancel = true;
					break;
				}
			}

			// もしスナップ先の頂点が、redundantなエッジを持っているなら、エッジ生成をキャンセル
			Polyline2D snapped_polyline;
			snapped_polyline.push_back(QVector2D(0, 0));
			snapped_polyline.push_back(QVector2D(new_edge->polyline.back() - roads.graph[tgtDesc]->pt));
			if (RoadGeneratorHelper::isRedundantEdge(roads, tgtDesc, snapped_polyline, angleTolerance)) {
				//（とりあえず、ものすごい鋭角の場合は、必ずキャンセル)
				cancel = true;
				break;
			}

			// もし他のエッジに交差するなら、エッジ生成をキャンセル
			// （キャンセルせずに、交差させるべき？）
			QVector2D intPoint;
			RoadEdgeDesc closestEdge;
			new_edge->polyline.push_back(roads.graph[tgtDesc]->pt);
			if (GraphUtil::isIntersect(roads, new_edge->polyline, curDesc, closestEdge, intPoint)) {
				cancel = true;
				break;
			}

			// エッジに沿って、水没チェック
			if (roadType == RoadEdge::TYPE_STREET) {
				if (RoadGeneratorHelper::submerged(vboRenderManager, new_edge->polyline, G::getFloat("seaLevel"))) {
					cancel = true;
					break;
				}
			}

			// エッジを生成
			GraphUtil::addEdge(roads, curDesc, tgtDesc, new_edge);

			return true;
		}

		// snap先がなければ、polylineを１歩伸ばす
		for (int sub_iter = 0; sub_iter < num_sub_steps; ++sub_iter) {
			QVector2D pt = new_edge->polyline.back();
			float z = vboRenderManager->getMinTerrainHeight(pt.x(), pt.y());
			QVector2D pt2 = pt + QVector2D(cosf(angle), sinf(angle)) * sub_step;
			float z2 = vboRenderManager->getMinTerrainHeight(pt2.x(), pt2.y());

			if (fabs(z - z2) > 2.5f) {
				curvature = 0.3f;
			}

			{
				float angle1 =  angle + curvature / (float)num_sub_steps;
				QVector2D pt1 = pt + QVector2D(cosf(angle1), sinf(angle1)) * sub_step;
				float z1 = vboRenderManager->getMinTerrainHeight(pt1.x(), pt1.y());

				float angle2 =  angle - curvature / (float)num_sub_steps;
				QVector2D pt2 = pt + QVector2D(cosf(angle2), sinf(angle2)) * sub_step;
				float z2 = vboRenderManager->getMinTerrainHeight(pt2.x(), pt2.y());

				if (fabs(z - z1) < fabs(z - z2)) {
					new_edge->polyline.push_back(pt1);
					angle = angle1;
				} else {
					new_edge->polyline.push_back(pt2);
					angle = angle2;
				}
			}
		}

		// もし、新規エッジが、既存グラフと交差するなら、エッジ生成をキャンセル
		QVector2D intPoint;
		RoadEdgeDesc closestEdge;
		if (GraphUtil::isIntersect(roads, new_edge->polyline, curDesc, closestEdge, intPoint)) {
			// 水面下なら終了
			float z = vboRenderManager->getMinTerrainHeight(intPoint.x(), intPoint.y());
			if (z < G::getFloat("seaLevel")) {
				cancel = true;
				break;
			}

			// 60%の確率でキャンセル？
			if (Util::genRand(0, 1) < 0.6f) {
				cancel = true;
				break;
			}

			// 交差する箇所で中断させる
			new_edge->polyline.clear();
			new_edge->polyline.push_back(curPt);
			new_edge->polyline.push_back(intPoint);

			// 他のエッジにスナップ
			tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
			roads.graph[tgtDesc]->generationType = "snapped";
			roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
			roads.graph[tgtDesc]->properties.remove("example_desc");

			// エッジを生成
			GraphUtil::addEdge(roads, curDesc, tgtDesc, new_edge);

			return true;
		}

		// streetの場合、水面下ならキャンセル
		if (roadType == RoadEdge::TYPE_STREET) {
			float z = vboRenderManager->getMinTerrainHeight(new_edge->polyline.back().x(), new_edge->polyline.back().y());
			if (z < G::getFloat("seaLevel")) {
				cancel = true;
				break;
			}
		}

		if (!found) {
			// 頂点を追加
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.back()));
			v->generationType = "pm";
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			tgtDesc = GraphUtil::addVertex(roads, v);

			// エリア外なら、onBoundaryフラグをセット
			if (!targetArea.contains(roads.graph[tgtDesc]->pt)) {
				roads.graph[tgtDesc]->onBoundary = true;
			}

			// エッジを作成
			GraphUtil::addEdge(roads, curDesc, tgtDesc, new_edge);

			curDesc = tgtDesc;
			curPt = new_edge->polyline.back();
		}

		// Avenueの場合、水面下ならここでストップ
		if (roadType == RoadEdge::TYPE_AVENUE) {
			float z = vboRenderManager->getMinTerrainHeight(new_edge->polyline.back().x(), new_edge->polyline.back().y());
			if (z < G::getFloat("seaLevel")) {
				break;
			}
		}

		// 遠くにある、近接頂点に向かうよう、方向を変える
		if (RoadGeneratorHelper::getVertexForSnapping(*vboRenderManager, roads, curDesc, step * (num_steps - iter) * 2.0f, G::getFloat("seaLevel"), angle, 0.3f, tgtDesc)) {
			QVector2D dir = roads.graph[tgtDesc]->pt - curPt;
			float a = atan2f(dir.y(), dir.x());
			angle += Util::diffAngle(a, angle, false) * 0.5f;
		}
	}

	if (cancel) {
		while (curDesc != srcDesc) {
			roads.graph[curDesc]->valid = false;
			RoadOutEdgeIter ei, eend;
			boost::tie(ei, eend) = boost::out_edges(curDesc, roads.graph);
			roads.graph[*ei]->valid = false;

			RoadVertexDesc nextDesc = boost::target(*ei, roads.graph);
			curDesc = nextDesc;
		}
		return false;
	} else {
		seeds.push_back(tgtDesc);
		return true;
	}
}

