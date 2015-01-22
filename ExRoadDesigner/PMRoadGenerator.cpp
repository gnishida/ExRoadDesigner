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

	int video_frame_id = 0;

	// Avenueのシードを生成
	generateAvenueSeeds(seeds);

	// Avenueを生成
	{
		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numAvenueIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < G::getFloat("seaLevel")) {
				std::cout << "attemptExpansion (avenue): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << i << " (Seed: " << desc << ")" << std::endl;
			attemptExpansion(RoadEdge::TYPE_AVENUE, desc, seeds);
		}
	}

	seeds.clear();

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		RoadGeneratorHelper::extendDanglingEdges(roads, 300.0f);
		RoadGeneratorHelper::removeAllDeadends(roads);
		//GraphUtil::reduce(roads);
		//GraphUtil::removeLoop(roads);
	}

	if (G::getBool("removeSmallBlocks")) {
		SmallBlockRemover::remove(roads, G::getFloat("minBlockSize"));
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
				
		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numStreetIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < G::getFloat("seaLevelForStreet")) {
				std::cout << "attemptExpansion (street): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (street): " << i << " (Seed: " << desc << ")" << std::endl;
			attemptExpansion(RoadEdge::TYPE_STREET, desc, seeds);
		}
	}

	// 指定されたエリアでCropping
	if (G::getBool("cropping")) {
		GraphUtil::extractRoads2(roads, targetArea);
	}

	if (G::getBool("cleanStreets")) {
		RoadGeneratorHelper::removeAllDeadends(roads);
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
	roads.graph[desc]->properties["rotation_angle"] = G::getFloat("rotationAngle");
	seeds.push_back(desc);

	return true;
}

/**
 * Local Street用のシードを生成する。
 */
void PMRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
	seeds.clear();

	// compute the interpolated length
	float interpolated_length = 0.0f;
	{
		std::vector<float> sigma2;
		sigma2.push_back(SQR(G::getDouble("interpolationSigma1")));
		sigma2.push_back(SQR(G::getDouble("interpolationSigma2")));
		for (int i = 2; i < features.size(); ++i) {
			sigma2.push_back(sigma2[0]);
		}
		
		float total = 0.0f;
		for (int i = 0; i < features.size(); ++i) {
			float length = features[i].avgStreetLength;

			interpolated_length += length * sigma2[i];
			total += sigma2[i];
		}
		interpolated_length /= total;
	}

	int vertex_num = boost::num_vertices(roads.graph);

	// エッジ上に、Local Street用のシードを生成する
	{
		int i = 0;
		int num = GraphUtil::getNumEdges(roads);
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = edges(roads.graph); ei != eend && i < num; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			// 両端頂点がエリア外なら、スキップ
			if (!targetArea.contains(roads.graph[src]->pt) && !targetArea.contains(roads.graph[tgt]->pt)) continue;

			std::cout << "Initial seed generation for the local street (i: " << i << ")" << std::endl;
			++i;

			RoadEdgePtr edge = roads.graph[*ei];
			RoadEdgeDesc e = *ei;
			RoadEdgeDesc e1, e2;

			// 両端の頂点から、group_idを特定
			int group_id = roads.graph[src]->properties["group_id"].toInt();

			int num = roads.graph[e]->polyline.length() / interpolated_length;
			if (num == 0) continue;

			int step = (roads.graph[e]->polyline.size() - 1) / num;

			if (step <= 1) continue;
		
			while (step < edge->polyline.size() - step) {
				RoadVertexDesc desc = GraphUtil::splitEdge(roads, e, edge->polyline[step], e1, e2);
				roads.graph[desc]->properties["group_id"] = group_id;
				roads.graph[desc]->properties["generation_type"] = "pm";

				float z = vboRenderManager->getTerrainHeight(edge->polyline[step].x(), edge->polyline[step].y());

				// この点が、エリア内なら、シードとして追加
				if (targetArea.contains(edge->polyline[step]) && z >= G::getFloat("seaLevelForStreet")) {
					seeds.push_back(desc);
				}

				edge = roads.graph[e2];
				e = e2;
			}

		}
	}
}

/**
 * このシードを使って、PM方式で道路生成する。
 */
void PMRoadGenerator::attemptExpansion(int roadType, RoadVertexDesc srcDesc, std::list<RoadVertexDesc> &seeds) {
	std::vector<RoadEdgePtr> edges;
	synthesizeItem(roadType, srcDesc, edges);
	
	float roadSnapFactor = 0.6f;
	float roadAngleTolerance = 0.52f;

	for (int i = 0; i < edges.size(); ++i) {
		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, edges[i]->polyline, roadAngleTolerance)) continue;
		growRoadSegment(roadType, srcDesc, edges[i]->polyline, edges[i]->lanes, roadSnapFactor, roadAngleTolerance, seeds);
	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 */
bool PMRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, const Polyline2D &polyline, int lanes, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	float sigma2 = SQR(G::getFloat("interpolationSigma1"));
	float threshold1 = G::getFloat("interpolationThreshold1");

	bool intercepted = false;

	float z0 = vboRenderManager->getTerrainHeight(roads.graph[srcDesc]->pt.x(), roads.graph[srcDesc]->pt.y());

	// 新しいエッジを生成
	RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
	QVector2D pt = roads.graph[srcDesc]->pt;
	for (int i = 0; i < polyline.size(); ++i) {
		if (i > 0) {
			pt += polyline[i] - polyline[i - 1];
		}
		//QVector2D pt = roads.graph[srcDesc]->pt + polyline[i];

		new_edge->polyline.push_back(pt);

		// 水没、または、山の上なら、道路生成をストップ
		float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
		if ((roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForAvenue")) || (roadType == RoadEdge::TYPE_STREET && z < G::getFloat("seaLevelForStreet"))) {
			// 最初っから水没している場合は、そもそもエッジ生成をキャンセル
			if (new_edge->polyline.size() <= 1) return false;

			RoadGeneratorHelper::bendEdgeBySteepElevationChange(roadType, new_edge->polyline, vboRenderManager);
			pt = new_edge->polyline.back();

			intercepted = true;
		}
		
		// if the slope is too steep, bend the direction to avoid climbing up
		if (z0 >= 70.0f && fabs(z - z0) > 7.0f) {
			RoadGeneratorHelper::bendEdgeBySteepElevationChange(new_edge->polyline, z0, vboRenderManager);
			pt = new_edge->polyline.back();
			z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
			z0 = z;
		}

		// 他のエッジと交差したら、道路生成をストップ
		QVector2D intPoint;
		//if (roadType == RoadEdge::TYPE_STREET && GraphUtil::isIntersect(roads, new_edge->polyline, intPoint)) {
		if (GraphUtil::isIntersect(roads, new_edge->polyline, srcDesc, intPoint)) {
			new_edge->polyline.erase(new_edge->polyline.begin() + new_edge->polyline.size() - 1);
			new_edge->polyline.push_back(intPoint);

			intercepted = true;

			// エッジ長が最短thresholdより短い場合は、キャンセル
			if (new_edge->getLength() < 30.0f) return false;

			// エッジがgoingStraightではなく、且つ、もとのエッジ長の半分未満にinterceptされた場合は、キャンセル
			//if (!goingStraight && new_edge->getLength() < polyline.length() * 0.5f) return false;
		}

		if (intercepted) break;
	}

	if (new_edge->polyline.size() == 1) return false;

	RoadVertexDesc tgtDesc;

	bool snapTried = false;
	bool snapDone = false;
	RoadEdgeDesc closestEdge;
	QVector2D intPoint;

	// snap to myself (loop edge)
	if ((new_edge->polyline.last() - roads.graph[srcDesc]->pt).lengthSquared() < 0.1f) {
		tgtDesc = srcDesc;
		snapTried = true;
		snapDone = true;
	}
	
	if (!snapDone && GraphUtil::getVertex(roads, new_edge->polyline.last(), (new_edge->polyline.back() - new_edge->polyline[0]).length() * snapFactor, srcDesc, tgtDesc)) {
		snapTried = true;

		// 他の頂点にスナップ
		Polyline2D new_polyline = new_edge->polyline;
		GraphUtil::movePolyline(roads, new_polyline, roads.graph[srcDesc]->pt, roads.graph[tgtDesc]->pt);

		if (RoadGeneratorHelper::submerged(roadType, new_polyline, vboRenderManager)) {
			new_polyline = new_edge->polyline;
			new_polyline.push_back(roads.graph[tgtDesc]->pt);
		}

		// 他のエッジと交差するか
		if (!GraphUtil::isIntersect(roads, new_polyline)) {
			std::reverse(new_polyline.begin(), new_polyline.end());
			if (!GraphUtil::hasRedundantEdge(roads, tgtDesc, new_polyline, angleTolerance)) {
				new_edge->polyline = new_polyline;
				snapDone = true;
			}
		}
	}

	if (!snapDone && GraphUtil::getEdge(roads, new_edge->polyline.last(), (new_edge->polyline.back() - new_edge->polyline[0]).length() * snapFactor, srcDesc, closestEdge, intPoint)) {
		snapTried = true;

		// 他のエッジにスナップ
		tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
		roads.graph[tgtDesc]->properties["generation_type"] = "snapped";
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
		roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
		
		GraphUtil::movePolyline(roads, new_edge->polyline, roads.graph[srcDesc]->pt, roads.graph[tgtDesc]->pt);
		if (!GraphUtil::isIntersect(roads, new_edge->polyline)) {
			if (!GraphUtil::hasRedundantEdge(roads, tgtDesc, new_edge->polyline, angleTolerance)) {
				snapDone = true;
				roads.graph[tgtDesc]->properties["parent"] = srcDesc;
			}
		}
	}

	if (snapTried && !snapDone) return false;

	if (!snapDone) {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.last()));
		tgtDesc = GraphUtil::addVertex(roads, v);
		roads.graph[tgtDesc]->properties["parent"] = srcDesc;

		// 新しい頂点にgroup_id、ex_idを引き継ぐ
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];

		// 新しい頂点のgeneration_typeを設定
		roads.graph[tgtDesc]->properties["generation_type"] = "pm";

		if (targetArea.contains(new_edge->polyline.last())) {
			// シードに追加する
			seeds.push_back(tgtDesc);
		} else {
			// ターゲットエリアの外に出たら
			roads.graph[tgtDesc]->properties["generation_type"] = "pm";
			roads.graph[tgtDesc]->onBoundary = true;
		}
	}

	RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, tgtDesc, new_edge);

	// 新しいエッジにgroup_id、ex_idを引き継ぐ
	roads.graph[e_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];

	// 新しいエッジのgeneration_typeを設定
	roads.graph[e_desc]->properties["generation_type"] = "pm";

	return true;
}

/**
 * PMに従って、カーネルを合成する
 */
void PMRoadGenerator::synthesizeItem(int roadType, RoadVertexDesc v_desc, std::vector<RoadEdgePtr> &edges) {
	BBox bbox = targetArea.envelope();
	float D2 = bbox.area();

	std::vector<float> sigma2;
	sigma2.push_back(SQR(G::getDouble("interpolationSigma1")));
	sigma2.push_back(SQR(G::getDouble("interpolationSigma2")));
	for (int i = 2; i < features.size(); ++i) {
		sigma2.push_back(sigma2[0]);
	}

	// 当該頂点から出るエッジをリストアップし、基底の方向を決定する
	float direction = 0.0f;
	if (roads.graph[v_desc]->properties.contains("rotation_angle")) {
		direction = roads.graph[v_desc]->properties["rotation_angle"].toFloat();
	}
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline  = GraphUtil::orderPolyLine(roads, *ei, v_desc);
			QVector2D vec = polyline[1] - polyline[0];
			direction = atan2f(vec.y(), vec.x());
			break;
		}
	}

	// interpolate the parameters
	float interpolated_length = 0.0f;
	float interpolated_curvature = 0.0f;
	float total = 0.0f;
	for (int i = 0; i < features.size(); ++i) {
		float length, curvature;
		if (roadType == RoadEdge::TYPE_AVENUE) {
			length = features[i].avgAvenueLength;
			curvature = features[i].avgAvenueCurvature;
		} else {
			length = features[i].avgStreetLength;
			curvature = features[i].avgStreetCurvature;
		}

		interpolated_length += length * sigma2[i];
		interpolated_curvature += curvature * sigma2[i];
		total += sigma2[i];
	}
	interpolated_length /= total;
	interpolated_curvature /= total;

	// 50%の確率で、どっちに曲がるか決定
	if (Util::genRand() >= 0.5f) {
		interpolated_curvature = -interpolated_curvature;
	}
	

	// generate four edges
	for (int i = 0; i < 4; ++i) {
		RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(roadType, 1));
		edge->polyline = RoadGeneratorHelper::createOneEdge(roadType, interpolated_length, interpolated_curvature, direction, 10.0f);
		edges.push_back(edge);

		direction += M_PI * 0.5f;
	}
}

