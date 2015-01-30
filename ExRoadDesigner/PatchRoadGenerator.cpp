#include "PatchRoadGenerator.h"
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

void PatchRoadGenerator::generateRoadNetwork() {
	srand(12345);

	// mark all the existing vertices as fixed
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			roads.graph[*vi]->fixed = true;

			// 既存頂点のexample_descを削除する。
			// だって、今使っているexampleとは違うから、もはや意味が無い。
			roads.graph[*vi]->generationType = "pm";
			roads.graph[*vi]->properties.remove("ex_id");
			roads.graph[*vi]->properties.remove("example_desc");
			roads.graph[*vi]->properties.remove("example_street_desc");

			// ターゲットエリア内の全ての頂点のonBoundaryフラグをfalseにする
			// これにより、dangling edgeは、もしこのエリアで生成された道路と接続されなかったら、clear処理の中で削除される
			if (targetArea.contains(roads.graph[*vi]->pt)) {
				roads.graph[*vi]->onBoundary = false;
			}
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
		// detect interesting shapes
		std::vector<std::vector<RoadEdgeDescs> > shapes;
		std::vector<std::vector<Patch> > patches;
		{
			shapes.resize(features.size());
			patches.resize(features.size());
			for (int i = 0; i < features.size(); ++i) {
				shapes[i] = features[i].shapes(RoadEdge::TYPE_AVENUE, G::getFloat("houghScale"), G::getFloat("avenuePatchDistance"), G::getBool("savePatchImages"));
				patches[i] = RoadGeneratorHelper::convertToPatch(RoadEdge::TYPE_AVENUE, features[i].roads(RoadEdge::TYPE_AVENUE), features[i].roads(RoadEdge::TYPE_AVENUE), shapes[i]);
			}
		}

		int iter;
		for (iter = 0; !seeds.empty() && iter < G::getInt("numAvenueIterations"); ) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			// 既に3つ以上エッジがある場合は、スキップする
			if (GraphUtil::getDegree(roads, desc) >= 3) {
				continue;
			}

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
			
			int ex_id = defineExId(roads.graph[desc]->pt);
			attemptConnect(RoadEdge::TYPE_AVENUE, desc, ex_id, features);
			if (roads.graph[desc]->valid && GraphUtil::getDegree(roads, desc) < 3) { // 上のコネクトで、現在頂点が消える可能性があるので、このチェックが必要
				if (RoadGeneratorHelper::largestAngleBetweenEdges(roads, desc, RoadEdge::TYPE_AVENUE) > M_PI * 1.4f) {
					if (!attemptExpansion(RoadEdge::TYPE_AVENUE, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
						attemptExpansion2(RoadEdge::TYPE_AVENUE, desc, ex_id, features[ex_id], seeds);
					}
				}
			}

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
		
		// detect interesting shapes
		std::vector<std::vector<RoadEdgeDescs> > shapes;
		std::vector<std::vector<Patch> > patches;
		{
			shapes.resize(features.size());
			patches.resize(features.size());
			for (int i = 0; i < features.size(); ++i) {
				shapes[i] = features[i].shapes(RoadEdge::TYPE_STREET, G::getFloat("houghScale") * 0.2f, G::getFloat("streetPatchDistance"), G::getBool("savePatchImages"));
				patches[i] = RoadGeneratorHelper::convertToPatch(RoadEdge::TYPE_STREET, features[i].roads(RoadEdge::TYPE_STREET), features[i].roads(RoadEdge::TYPE_AVENUE), shapes[i]);
			}
		}
		
		std::cout << "Local street generation started." << std::endl;

		int iter;
		for (iter = 0; !seeds.empty() && iter < G::getInt("numStreetIterations");) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			std::cout << "attemptExpansion (street): " << iter << " (Seed: " << desc << ")" << std::endl;

			// 既に3つ以上エッジがある場合は、スキップする
			if (GraphUtil::getDegree(roads, desc) >= 3) {
				continue;
			}

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

			int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			attemptConnect(RoadEdge::TYPE_STREET, desc, ex_id, features);
			if (roads.graph[desc]->valid && GraphUtil::getDegree(roads, desc) < 3) { // 上のコネクトで、現在頂点が消える可能性があるので、このチェックが必要
				if (RoadGeneratorHelper::largestAngleBetweenEdges(roads, desc, RoadEdge::TYPE_STREET) > M_PI * 0.9f) {
					if (!attemptExpansion(RoadEdge::TYPE_STREET, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
						attemptExpansion2(RoadEdge::TYPE_STREET, desc, ex_id, features[ex_id], seeds);
					}
				}
			}

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
void PatchRoadGenerator::generateAvenueSeeds(std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	int numSeedsPerEx = hintLine.size() / features.size();

	for (int i = 0; i <features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;
			addAvenueSeed(features[i], hintLine[index], features[i].hintLine[j], index, i, G::getFloat("rotationAngle"), seeds);
		}
	}
}

/**
 * Avenue用のシードを座標pt付近に追加する。
 * 座標pt付近の、該当するカーネルを捜し、そのカーネルを使ってシードを追加する。
 *
 * @param f				特徴量
 * @param pt			ターゲット領域におけるシード座標
 * @param ex_pt			Exampleでの初期点座標
 * @param group_id		group ID
 * @param ex_id			example ID
 * @param angle			回転角度 [rad]
 * @param seeds			追加されたシードは、seedsに追加される。
 */
bool PatchRoadGenerator::addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, float angle, std::list<RoadVertexDesc>& seeds) {
	if (!targetArea.contains(pt)) return false;

	// Avenueカーネルの中で、offsetの位置に最も近いものを探す
	RoadVertexDesc seedDesc = GraphUtil::getVertex(f.roads(RoadEdge::TYPE_AVENUE), ex_pt);

	// 頂点を追加し、シードとする
	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	roads.graph[desc]->properties["group_id"] = group_id;
	roads.graph[desc]->generationType = "example";
	roads.graph[desc]->properties["ex_id"] = ex_id;
	roads.graph[desc]->properties["example_desc"] = seedDesc;
	roads.graph[desc]->rotationAngle = angle;
	seeds.push_back(desc);

	float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
	if (z < G::getFloat("seaLevel")) {
		z -= 10.0f; // 0.1f;
		if (z < 0) z = 0.0f;
		G::global()["seaLevel"] = z;
	}

	return true;
}

/**
 * Local Street用のシードを生成する。
 */
void PatchRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
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

			if (roads.graph[*vi]->properties.contains("example_desc")) {
				// ターゲットエリア座標空間から、Example座標空間へのオフセットを計算
				int ex_id = roads.graph[*vi]->properties["ex_id"].toInt();
				RoadVertexDesc ex_v_desc = roads.graph[*vi]->properties["example_desc"].toUInt();

				RoadVertexDesc seedDesc;
				if (GraphUtil::getVertex(features[ex_id].roads(RoadEdge::TYPE_STREET), features[ex_id].roads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt, 1.0f, seedDesc)) {
					seeds.push_back(*vi);
					roads.graph[*vi]->properties["example_street_desc"] = seedDesc;
				}
			} else {
				// PMで生成された頂点については、ex_idのみセットして、シードに登録する。
				if (GraphUtil::getDegree(roads, *vi) == 2) {
					int ex_id = defineExId(roads.graph[*vi]->pt);
					roads.graph[*vi]->properties["ex_id"] = ex_id;

					seeds.push_back(*vi);
				}
			}
		}
	}
}

/**
 * 近くに頂点またはエッジがあるなら、コネクトしちゃう。
 * コネクトできた場合は、trueを返却する。
 */
bool PatchRoadGenerator::attemptConnect(int roadType, RoadVertexDesc srcDesc, int ex_id, std::vector<ExFeature>& features) {
	float length = 0.0f;

	if (roadType == RoadEdge::TYPE_AVENUE) {
		length = features[ex_id].avgAvenueLength;
	} else {
		length = features[ex_id].avgStreetLength;
	}

	// スナップする際、許容できる角度
	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	std::vector<RoadEdgePtr> edges;

	// 近接頂点を探し、あればコネクトする
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		float direction = atan2f((polyline[1] - polyline[0]).y(), (polyline[1] - polyline[0]).x());
		direction += M_PI;

		if (attemptConnectToVertex(roadType, srcDesc, features, length, G::getFloat("seaLevel"), direction, 0.78f, roadAngleTolerance)) return true;
	}

	// 近接エッジを探し、あればコネクトする
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		float direction = atan2f((polyline[1] - polyline[0]).y(), (polyline[1] - polyline[0]).x());
		direction += M_PI;
		if (attemptConnectToEdge(roadType, srcDesc, length, G::getFloat("seaLevel"), direction, 0.3f)) return true;
	}

	// 中距離の頂点を探し、あればコネクトする
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		float direction = atan2f((polyline[1] - polyline[0]).y(), (polyline[1] - polyline[0]).x());
		direction += M_PI;

		if (attemptConnectToVertex(roadType, srcDesc, features, length, G::getFloat("seaLevel"), direction, 1.5f, roadAngleTolerance)) return true;
	}

	return false;
}

/**
 * 近くに頂点があるなら、コネクトしちゃう。
 *
 * @param roadType				avenue / local street
 * @param srcDesc				現在の頂点ID
 * @param features				featureリスト
 * @param dist_threshold		距離基準（これより遠い頂点は対象外）
 * @param z_threshold			最低標高（これより低い頂点は対象外）
 * @param direction				コネクトする方向基準
 * @param angle_threshold		方向基準からしきい値以上ずれる頂点は対象外
 * @param roadAngleTolerance	既存エッジとの成す角度がこれ未満だと、キャンセル
 * @return						コネクトしたらtrue、コネクトしなかったらfalseを返却する。
 */
bool PatchRoadGenerator::attemptConnectToVertex(int roadType, RoadVertexDesc srcDesc, std::vector<ExFeature>& features, float dist_threshold, float z_threshold, float direction, float angle_threshold, float roadAngleTolerance) {
	// ものすごい近くに、他の頂点がないかあれば、そことコネクトして終わり。
	// それ以外の余計なエッジは生成しない。さもないと、ものすごい密度の濃い道路網になっちゃう。
	{
		RoadVertexDesc nearestDesc;
		if (RoadGeneratorHelper::getVertexForSnapping(*vboRenderManager, roads, srcDesc, dist_threshold, z_threshold, direction, angle_threshold, nearestDesc)) {
			// もし、既にエッジがあるなら、キャンセル
			// なお、ここではtrueを返却して、これ以上のエッジ生成をさせない。
			if (GraphUtil::hasEdge(roads, srcDesc, nearestDesc)) return true;

			// その頂点のexample_descと、この頂点のexample_descの位置関係と、実際の位置関係が同じ場合、
			// patch適用でピッタリはまるはずなので、connectしない。
			if (roads.graph[srcDesc]->properties.contains("ex_id")) {
				RoadVertexDesc ex_v1_desc;
				RoadVertexDesc ex_v2_desc;
				int pre_ex_id = roads.graph[srcDesc]->properties["ex_id"].toInt();
				bool has_ex_v1 = false;
				bool has_ex_v2 = false;
				if (roadType == RoadEdge::TYPE_AVENUE) {
					if (roads.graph[srcDesc]->properties.contains("example_desc")) {
						has_ex_v1 = true;
						ex_v1_desc = roads.graph[srcDesc]->properties["example_desc"].toUInt();
					}
					if (roads.graph[nearestDesc]->properties.contains("ex_id") && roads.graph[nearestDesc]->properties["ex_id"].toInt() == pre_ex_id) {
						if (roads.graph[nearestDesc]->properties.contains("example_desc")) {
							has_ex_v2 = true;
							ex_v2_desc = roads.graph[nearestDesc]->properties["example_desc"].toUInt();
						}
					}
				} else {
					if (roads.graph[srcDesc]->properties.contains("example_street_desc")) {
						has_ex_v1 = true;
						ex_v1_desc = roads.graph[srcDesc]->properties["example_street_desc"].toUInt();
					}
					if (roads.graph[nearestDesc]->properties.contains("ex_id") && roads.graph[nearestDesc]->properties["ex_id"].toInt() == pre_ex_id) {
						if (roads.graph[nearestDesc]->properties.contains("example_street_desc")) {
							has_ex_v2 = true;
							ex_v2_desc = roads.graph[nearestDesc]->properties["example_street_desc"].toUInt();
						}
					}
				}

				// patchがピッタリはまるはずなので、connectしない
				// なので、trueを返却し、connectを終了させる。
				if (has_ex_v1 && has_ex_v2) {
					QVector2D ex_vec = features[pre_ex_id].roads(roadType).graph[ex_v2_desc]->pt - features[pre_ex_id].roads(roadType).graph[ex_v1_desc]->pt;
					QVector2D vec = roads.graph[nearestDesc]->pt - roads.graph[srcDesc]->pt;
					if ((vec - ex_vec).lengthSquared() <= 0.1f) return true;
				}
			}

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);

			// 自分にとってredundantなら、コネクトしない。
			// 理由：このエッジは、角度を考慮せず、単純に距離で見つけたもの。従って、角度が鋭角すぎるケースがあり得る
			{
				Polyline2D polyline;
				polyline.push_back(QVector2D(0, 0));
				polyline.push_back(roads.graph[nearestDesc]->pt - roads.graph[srcDesc]->pt);
				if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, polyline, roadAngleTolerance)) {
					return false;
				}
			}

			// スナップ先にとってredundantなら、コネクトしないで、終了。
			// つまり、trueを返却して、終わったことにしちゃう。
			{
				Polyline2D e2;
				e2.push_back(QVector2D(0, 0));
				e2.push_back(roads.graph[srcDesc]->pt - roads.graph[nearestDesc]->pt);
				if (RoadGeneratorHelper::isRedundantEdge(roads, nearestDesc, e2, roadAngleTolerance)) {
					// もしdegree=1なら、連なるエッジをごっそり削除
					if (GraphUtil::getDegree(roads, srcDesc) == 1) {
						RoadGeneratorHelper::removeEdge(roads, srcDesc);
					}

					return true;
				}
			}

			// コネクトしてtrueを返却する
			if (!GraphUtil::isIntersect(roads, e->polyline)) {
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);
				return true;
			}
		}
	}

	return false;
}

/**
 * 近くにエッジがあるなら、コネクトしちゃう。
 *
 * @param roadType			avenue / local street
 * @param srcDesc			現在の頂点ID
 * @param features			featureリスト
 * @param dist_threshold	距離基準（これより遠い頂点は対象外）
 * @param z_threshold		最低標高（これより低い頂点は対象外）
 * @param direction			コネクトする方向基準
 * @param angle_threshold	方向基準からしきい値以上ずれる頂点は対象外
 * @return					コネクトしたらtrue、コネクトしなかったらfalseを返却する。
 */
bool PatchRoadGenerator::attemptConnectToEdge(int roadType, RoadVertexDesc srcDesc, float dist_threshold, float z_threshold, float direction, float angle_threshold) {
	// 近くにエッジがあれば、コネクト
	{
		RoadVertexDesc nearestDesc;
		RoadEdgeDesc nearestEdgeDesc;
		QVector2D intPoint;
		if (RoadGeneratorHelper::getEdgeForSnapping(*vboRenderManager, roads, srcDesc, dist_threshold, G::getFloat("seaLevel"), direction, angle_threshold, nearestEdgeDesc, intPoint)) {
			RoadVertexDesc nearestVertexDesc = boost::source(nearestEdgeDesc, roads.graph);

			// エッジにスナップ
			nearestDesc = GraphUtil::splitEdge(roads, nearestEdgeDesc, intPoint);
			roads.graph[nearestDesc]->generationType = "snapped";
			roads.graph[nearestDesc]->properties["group_id"] = roads.graph[nearestVertexDesc]->properties["group_id"];
			roads.graph[nearestDesc]->properties["ex_id"] = roads.graph[nearestVertexDesc]->properties["ex_id"];
			roads.graph[nearestDesc]->properties.remove("example_desc");

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);
			RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);

			return true;
		}
	}

	return false;
}

/**
 * このシードを使って、道路生成する。
 * Exampleベースで生成する。
 */
bool PatchRoadGenerator::attemptExpansion(int roadType, RoadVertexDesc srcDesc, int ex_id, ExFeature& f, std::vector<Patch> &patches, std::list<RoadVertexDesc> &seeds) {
	RoadVertexDesc ex_v_desc;
	bool exampleFound = false;
	if (roadType == RoadEdge::TYPE_AVENUE) {
		if (roads.graph[srcDesc]->properties.contains("example_desc")) {
			if (roads.graph[srcDesc]->properties["ex_id"].toInt() == ex_id) {
				exampleFound = true;
				ex_v_desc = roads.graph[srcDesc]->properties["example_desc"].toUInt();
			}
		}
	} else {
		if (roads.graph[srcDesc]->properties.contains("example_street_desc")) {
			if (roads.graph[srcDesc]->properties["ex_id"].toInt() == ex_id) {
				exampleFound = true;
				ex_v_desc = roads.graph[srcDesc]->properties["example_street_desc"].toUInt();
			}
		}
	}
	float angle = roads.graph[srcDesc]->rotationAngle;

	int patch_id;

	RoadGraph replacementGraph;

	// パッチで生成された頂点でも、元のExampleで境界上の頂点である場合、次に対応するパッチがない。
	// そこで、「パッチなし」とマークする。
	if (exampleFound) {
		if (f.roads(roadType).graph[ex_v_desc]->onBoundary) {
			exampleFound = false;
		}
	}

	// 次の対応パッチが決まっていない場合、
	if (!exampleFound) {
		// ローカルストリートで、且つ、ローカルストリートのエッジが１本もない場合は、PMを使用すること！
		if (roadType == RoadEdge::TYPE_STREET) {
			bool edge_found = false;
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;
				if (roads.graph[*ei]->type == RoadEdge::TYPE_STREET) {
					edge_found = true;
					break;
				}
			}
			if (!edge_found) return false;
		}

		Polyline2D polyline = GraphUtil::getAdjoiningPolyline(roads, srcDesc);

		RoadVertexDesc root_desc;
		RoadVertexDesc connect_desc;

		std::reverse(polyline.begin(), polyline.end());
		
		std::vector<float> similarity_scores;
		for (int i = 0; i < patches.size(); ++i) {
			// 現在の頂点が属するパッチと、同じパッチは、使わない。
			// もし使っちゃったら、同じパッチが並んでしまう。
			if (i == roads.graph[srcDesc]->patchId) {
				similarity_scores.push_back(0.0f);
				continue;
			}

			float dist = patches[i].getMinHausdorffDist(polyline, connect_desc, root_desc);
			similarity_scores.push_back(expf(-dist));
		}

		// probability distributionに従って、パッチを決定する
		int patch_id = Util::sampleFromPdf(similarity_scores);

		// コネクタ、根元を取得
		patches[patch_id].getMinHausdorffDist(polyline, connect_desc, root_desc);

		// streetの場合は、example_street_descにしないといけないか？
		// いや、大丈夫。streetのパッチは、元のgraphの頂点IDを、やはりexample_descに格納しているから。
		ex_v_desc = patches[patch_id].roads.graph[root_desc]->properties["example_desc"].toUInt();


		buildReplacementGraphByExample2(roadType, replacementGraph, srcDesc, ex_id, f.roads(roadType), ex_v_desc, angle, patches[patch_id], patch_id, connect_desc, root_desc);

	} else {
		patch_id = f.roads(roadType).graph[ex_v_desc]->patchId;
		if (patch_id < 0) {
			printf("ERROR! patch_id has to >= 0.");
			assert(false);
		}
	
		buildReplacementGraphByExample(roadType, replacementGraph, srcDesc, ex_id, f.roads(roadType), ex_v_desc, angle, patches[patch_id], patch_id);
	}

	// もしreplacemeng graph が空なら、キャンセル
	if (boost::num_edges(replacementGraph.graph) == 0) return false;

	// 山チェック
	if (RoadGeneratorHelper::maxZ(replacementGraph, vboRenderManager) > 70.0f && RoadGeneratorHelper::diffSlope(replacementGraph, vboRenderManager) > 0.3f) {
		float max_rotation = M_PI * 0.166f;
		float min_slope = std::numeric_limits<float>::max();
		float min_rotation;

		RoadGraph backup;
		GraphUtil::copyRoads(replacementGraph, backup);

		// try the turn in CCW direction
		// Note: th is the rotation angle in addition to the current rotaion angle "angle"
		for (float th = 0; th <= max_rotation; th += 0.1f) {
			GraphUtil::copyRoads(backup, replacementGraph);
			GraphUtil::rotate(replacementGraph, th, roads.graph[srcDesc]->pt);
			float diffSlope = RoadGeneratorHelper::diffSlope(replacementGraph, vboRenderManager);
			if (diffSlope < min_slope) {
				min_slope = diffSlope;
				min_rotation = th;
			}
		}

		// try the turn in CW direction
		// Note: th is the rotation angle in addition to the current rotaion angle "angle"
		for (float th = 0; th >= -max_rotation; th -= 0.1f) {
			GraphUtil::copyRoads(backup, replacementGraph);
			GraphUtil::rotate(replacementGraph, th, roads.graph[srcDesc]->pt);
			float diffSlope = RoadGeneratorHelper::diffSlope(replacementGraph, vboRenderManager);
			if (diffSlope < min_slope) {
				min_slope = diffSlope;
				min_rotation = th;
			}
		}

		// 回転しても駄目なら、exampleを使用しない
		if (min_slope > 0.3f) return false;

		GraphUtil::copyRoads(backup, replacementGraph);
		GraphUtil::rotate(replacementGraph, min_rotation, roads.graph[srcDesc]->pt);

		// rotationAngleを設定
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(replacementGraph.graph); vi != vend; ++vi) {
			replacementGraph.graph[*vi]->rotationAngle = angle + min_rotation;
		}
	}

	// 川チェック
	if (RoadGeneratorHelper::minZ(replacementGraph, vboRenderManager, true) < G::getFloat("seaLevel")) {
		return false;
	}

	// replacementGraphが、既に生成済みのグラフと重なるかどうかチェック
	if (GraphUtil::isIntersect(replacementGraph, roads)) {
		return false;
	}

	// replacementGraphのほとんどが、ターゲットエリア内に入っているかチェック
	// Hack:
	// このチェックはなくても良いのだが、New-Yorkのlocal streetのパッチ抽出バグのせいで、めちゃ長い道路がパッチとして抽出されてしまい、
	// これをしないと、ターゲットエリアからめちゃはみ出た道路が生成されてしまう。
	{
		int cnt = 0;
		int total_cnt = 0;
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(replacementGraph.graph); vi != vend; ++vi) {
			if (!replacementGraph.graph[*vi]->valid) continue;
			if (targetArea.contains(replacementGraph.graph[*vi]->pt)) {
				cnt++;
			}
			total_cnt++;
		}

		if ((float)cnt / (float)total_cnt < 0.5f) return false;
	}
	
	rewrite(roadType, srcDesc, replacementGraph, ex_id, seeds);

	return true;
}

/**
 * replacement グラフを構築する。ただし、現在頂点srcDescから伸びる既存エッジと重複するエッジは削除しておく。
 * 生成されたreplacement グラフの座標は、ターゲットグラフのそれに合わせてある。
 *
 * @param roadType					道路タイプ (Avenue / local street)
 * @param replacementGraph [OUT]	構築されたreplacementグラフ
 * @param srcDesc					ターゲットグラフにおける現在頂点ID
 * @param ex_id						使用するexampleのID
 * @param exRoads					example道路グラフ
 * @param ex_srcDesc				srcDescに対応する、exampleグラフの頂点ID
 * @param angle						回転させるべき角度
 * @param patch						使用するパッチ
 * @param patchId					使用するパッチのID
 */
void PatchRoadGenerator::buildReplacementGraphByExample(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId) {
	replacementGraph.clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexDesc root_desc = 9999999;

	// add vertices of the patch
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(patch.roads.graph); vi != vend; ++vi) {
			// Transformした後の座標を計算
			QVector2D pt = Util::transform(patch.roads.graph[*vi]->pt, exRoads.graph[ex_srcDesc]->pt, angle, roads.graph[srcDesc]->pt);

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*patch.roads.graph[*vi]));
			v->pt = pt;
			v->rotationAngle = angle;
			v->properties["ex_id"] = ex_id;
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			if (v->patchId != patchId) {
				printf("ERROR!!! patchId has not been copied.\n");
				assert(false);
			}

			RoadVertexDesc v_desc = GraphUtil::addVertex(replacementGraph, v);
			conv[*vi] = v_desc;

			if (patch.roads.graph[*vi]->properties["example_desc"].toUInt() == ex_srcDesc) {
				root_desc = v_desc;
			}
		}
	}

	if (root_desc == 9999999) {
		printf("ERROR!!!! root_desc is not found!\n");
		assert(false);
	}

	// patchのエッジを追加
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(patch.roads.graph); ei != eend; ++ei) {
			RoadVertexDesc src = boost::source(*ei, patch.roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, patch.roads.graph);

			RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(*patch.roads.graph[*ei]));

			// エッジを座標変換する
			edge->polyline.translate(-exRoads.graph[ex_srcDesc]->pt);
			edge->polyline.rotate(Util::rad2deg(-angle));
			edge->polyline.translate(roads.graph[srcDesc]->pt);

			RoadEdgeDesc e_desc = GraphUtil::addEdge(replacementGraph, conv[src], conv[tgt], edge);
		}
	}

	// コネクタの付け根の頂点から、既存グラフの頂点srcDescから出るエッジとredundantな方向に伸びるエッジも、削除する。
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);

			RoadOutEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = boost::out_edges(root_desc, replacementGraph.graph); ei2 != eend2; ++ei2) {
				if (!replacementGraph.graph[*ei2]->valid) continue;

				// もしこのエッジがコネクタじゃないなら、削除させない
				if (!replacementGraph.graph[*ei2]->connector) continue;

				Polyline2D polyline2 = GraphUtil::orderPolyLine(replacementGraph, *ei2, root_desc);

				if (Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]) < 0.3f) {
					// この方向に伸びるエッジを削除する
					RoadGeneratorHelper::removeEdge(replacementGraph, root_desc, *ei2);
				}
			}
		}
	}

	GraphUtil::clean(replacementGraph);
}

/**
 * replacement graphを生成する。ただし、既存グラフと接続するコネクタ部分を削除する。
 * 生成されたreplacement グラフの座標は、ターゲットグラフのそれに合わせてある。
 */
void PatchRoadGenerator::buildReplacementGraphByExample2(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId, RoadVertexDesc v_connect, RoadVertexDesc v_root) {
	replacementGraph.clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexDesc connector_desc = 9999999;
	RoadVertexDesc root_desc = 9999999;

	// add vertices of the patch
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(patch.roads.graph); vi != vend; ++vi) {
			// Transformした後の座標を計算
			QVector2D pt = Util::transform(patch.roads.graph[*vi]->pt, exRoads.graph[ex_srcDesc]->pt, angle, roads.graph[srcDesc]->pt);

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*patch.roads.graph[*vi]));
			v->pt = pt;
			v->rotationAngle = angle;
			v->properties["ex_id"] = ex_id;
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			if (v->patchId != patchId) {
				printf("ERROR!!! patchId has not been copied.\n");
				assert(false);
			}

			RoadVertexDesc v_desc = GraphUtil::addVertex(replacementGraph, v);
			conv[*vi] = v_desc;

			if (*vi == v_connect) {
				connector_desc = v_desc;
			}
			if (*vi == v_root) {
				root_desc = v_desc;
			}
		}
	}

	if (root_desc == 9999999) {
		printf("ERROR!!!! root_desc is not found!\n");
		assert(false);
	}
	if (connector_desc == 9999999) {
		printf("ERROR!!!! connector_desc is not found!\n");
		assert(false);
	}

	// patchのエッジを追加
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(patch.roads.graph); ei != eend; ++ei) {
			RoadVertexDesc src = boost::source(*ei, patch.roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, patch.roads.graph);

			RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(*patch.roads.graph[*ei]));

			// エッジを座標変換する
			edge->polyline.translate(-exRoads.graph[ex_srcDesc]->pt);
			edge->polyline.rotate(Util::rad2deg(-angle));
			edge->polyline.translate(roads.graph[srcDesc]->pt);

			RoadEdgeDesc e_desc = GraphUtil::addEdge(replacementGraph, conv[src], conv[tgt], edge);
		}
	}

	// 指定されたコネクタからのエッジを削除
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(connector_desc, replacementGraph.graph); ei != eend; ++ei) {
			if (!replacementGraph.graph[*ei]->valid) continue;

			RoadGeneratorHelper::removeEdge(replacementGraph, connector_desc, *ei);
			break;
		}
	}

	// コネクタの付け根の頂点から、既存グラフの頂点srcDescから出るエッジとredundantな方向に伸びるエッジも、削除する。
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);

			RoadOutEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = boost::out_edges(root_desc, replacementGraph.graph); ei2 != eend2; ++ei2) {
				if (!replacementGraph.graph[*ei2]->valid) continue;

				Polyline2D polyline2 = GraphUtil::orderPolyLine(replacementGraph, *ei2, root_desc);

				if (Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]) < 0.3f) {
					// この方向に伸びるエッジを削除する
					RoadGeneratorHelper::removeEdge(replacementGraph, root_desc, *ei2);
				}
			}
		}
	}

	GraphUtil::clean(replacementGraph);
}

void PatchRoadGenerator::rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, int ex_id, std::list<RoadVertexDesc>& seeds) {
	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// add vertices
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(replacementGraph.graph); vi != vend; ++vi) {
			if (!replacementGraph.graph[*vi]->valid) continue;

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*replacementGraph.graph[*vi]));
			v->generationType = "example";
			RoadVertexDesc v_desc;
			if (!GraphUtil::getVertex(roads, replacementGraph.graph[*vi]->pt, 1.0f, v_desc)) {
				// 頂点を新規追加
				v_desc = GraphUtil::addVertex(roads, v);

				// エリア外なら、onBoundaryフラグをセット
				if (!targetArea.contains(roads.graph[v_desc]->pt)) {
					roads.graph[v_desc]->onBoundary = true;
				} else {
					roads.graph[v_desc]->onBoundary = false;
				}

				// 新規追加された頂点について、もとのreplacementGraphでconnectorなら、シードに追加
				if (replacementGraph.graph[*vi]->connector) {
					seeds.push_back(v_desc);
				}
			} else {
				roads.graph[v_desc]->properties = replacementGraph.graph[*vi]->properties;
			}

			roads.graph[v_desc]->properties["ex_id"] = ex_id;
			roads.graph[v_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			if (roadType == RoadEdge::TYPE_STREET) {
				roads.graph[v_desc]->properties["example_street_desc"] = roads.graph[v_desc]->properties["example_desc"];
			}

			conv[*vi] = v_desc;
		}
	}
	
	// add edges
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(replacementGraph.graph); ei != eend; ++ei) {
			if (!replacementGraph.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, replacementGraph.graph);
			RoadVertexDesc tgt = boost::target(*ei, replacementGraph.graph);

			if (!GraphUtil::hasSimilarEdge(roads, conv[src], conv[tgt], replacementGraph.graph[*ei]->polyline)) {
				RoadEdgePtr e = RoadEdgePtr(new RoadEdge(*replacementGraph.graph[*ei]));
				e->generationType = "example";
				GraphUtil::addEdge(roads, conv[src], conv[tgt], e);
			}
		}
	}
}

/**
 * このシードを使って、PM方式で道路生成する。
 * パッチが使えないケースに使用される。
 * 通常、パッチ間の接着剤として使いたいなぁ。。。
 */
void PatchRoadGenerator::attemptExpansion2(int roadType, RoadVertexDesc srcDesc, int ex_id, ExFeature& f, std::list<RoadVertexDesc> &seeds) {
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

		// 統計情報から、ステップ数、長さ、曲率を決定する
		float step = 0.0f;
		float curvature = 0.0f;
		int num_steps = 1;
		if (roadType == RoadEdge::TYPE_AVENUE) {
			num_steps = ceilf(f.avgAvenueLength / f.avgStreetLength);

			step = Util::genRandNormal(f.avgStreetLength, f.varStreetLength);
			if (step < f.avgStreetLength * 0.5f) step = f.avgStreetLength * 0.5f;

			curvature = f.avgAvenueCurvature;
		} else {
			num_steps = 1;

			step = Util::genRandNormal(f.avgStreetLength, f.varStreetLength);
			if (step < f.avgStreetLength * 0.5f) step = f.avgStreetLength * 0.5f;

			curvature = f.avgStreetCurvature;
		}
	
		// 坂が急なら、キャンセル
		QVector2D pt2 = roads.graph[srcDesc]->pt + QVector2D(cosf(direction), sinf(direction)) * 20.0f;
		float z2 = vboRenderManager->getTerrainHeight(pt2.x(), pt2.y());
		if (z2 - z > tanf(G::getFloat("slopeTolerance")) * 20.0f) {
			if (Util::genRand(0, 1) < 0.8f) return;

			// 急勾配を上昇する場合は、直線道路にする
			curvature = 0.0f;
		}



		growRoadSegment(roadType, srcDesc, step, num_steps, direction, curvature, 1, roadAngleTolerance, ex_id, seeds);
	}
}

/**
 * 指定された方向、ステップ、曲率で、srcDesc頂点からエッジを伸ばす。
 * この関数は、PM方式での道路生成でのみ使用される。
 * 生成された場合はtrueを返却する。
 */
bool PatchRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, float step, int num_steps, float angle, float curvature, int lanes, float angleTolerance, int ex_id, std::list<RoadVertexDesc> &seeds) {
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
				// 交差するはず無い！
				assert(false);
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
		bool turnLeftByDefault = true;
		if (Util::genRand(0, 1) < 0.5f) {
			turnLeftByDefault = false;
		}
		for (int sub_iter = 0; sub_iter < num_sub_steps; ++sub_iter) {
			QVector2D pt = new_edge->polyline.back();
			float z = vboRenderManager->getMinTerrainHeight(pt.x(), pt.y());
			
			float angle1 = angle + tanf(curvature) / num_sub_steps;
			QVector2D pt1 = pt + QVector2D(cosf(angle1), sinf(angle1)) * sub_step;
			float z1 = vboRenderManager->getMinTerrainHeight(pt1.x(), pt1.y());

			float angle2 = angle - tanf(curvature) / num_sub_steps;
			QVector2D pt2 = pt + QVector2D(cosf(angle2), sinf(angle2)) * sub_step;
			float z2 = vboRenderManager->getMinTerrainHeight(pt2.x(), pt2.y());

			if (fabs(z - z1) == fabs(z - z2)) {
				if (turnLeftByDefault) {
					new_edge->polyline.push_back(pt1);
					angle = angle1;
				} else {
					new_edge->polyline.push_back(pt2);
					angle = angle2;
				}
			} else if (fabs(z - z1) < fabs(z - z2)) {
				new_edge->polyline.push_back(pt1);
				angle = angle1;
			} else {
				new_edge->polyline.push_back(pt2);
				angle = angle2;
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
			roads.graph[tgtDesc]->properties["ex_id"] = ex_id;
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
			v->properties["ex_id"] = ex_id;
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			tgtDesc = GraphUtil::addVertex(roads, v);

			// エッジを作成
			GraphUtil::addEdge(roads, curDesc, tgtDesc, new_edge);

			// エリア外なら、onBoundaryフラグをセット
			if (!targetArea.contains(roads.graph[tgtDesc]->pt)) {
				roads.graph[tgtDesc]->onBoundary = true;
				break;
			}

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

/**
 * 指定した点に基づき、どのexampleを使用するか、各exampleのシード点からの距離に基づいて、
 * ガウス分布の乱数を使って、決定する。
 *
 * @param pt		指定した点
 * @return			example ID
 */
int PatchRoadGenerator::defineExId(const QVector2D& pt) {
	// ターゲットエリアの長さを、距離のcanonical formの計算に使用する
	float D = targetArea.envelope().dx();

	std::vector<float> sigma;
	sigma.push_back(G::getDouble("interpolationSigma1"));
	sigma.push_back(G::getDouble("interpolationSigma2"));
	for (int i = 2; i < features.size(); ++i) {
		sigma.push_back(G::getDouble("interpolationSigma2"));
	}

	int numSeedsPerEx = hintLine.size() / features.size();

	std::vector<float> pdf;
	for (int i = 0; i < features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;
			
			// とりあえずblending用に、X軸方向での距離だけを使用してみよう
			float dist = (hintLine[index] - pt).x() / D;
			
			pdf.push_back(expf(-SQR(dist) / SQR(sigma[i]) * 2.0f));
		}
	}

	return Util::sampleFromPdf(pdf) / numSeedsPerEx;
}

