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
				shapes[i] = features[i].shapes(RoadEdge::TYPE_AVENUE, G::getFloat("houghScale"), G::getFloat("avenuePatchDistance"));
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

			// エリアの外なら、スキップする
			if (!targetArea.contains(roads.graph[desc]->pt)) {
				continue;
			}

			// 水中なら、伸ばして水中から脱出できるなら伸ばす。
			float z = vboRenderManager->getMinTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y());
			if (z < G::getFloat("seaLevel")) {
				// 水中の頂点は、degree=1のはず！！
				assert(GraphUtil::getDegree(roads, desc) == 1);
				if (!extendRoadAcrossRiver(RoadEdge::TYPE_AVENUE, desc, seeds, 200.0f)) {
					RoadOutEdgeIter ei, eend;
					boost::tie(ei, eend) = boost::out_edges(desc, roads.graph);
					RoadGeneratorHelper::removeEdge(roads, desc, *ei);
				}
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << iter << " (Seed: " << desc << ")" << std::endl;
			
			int ex_id = defineExId(roads.graph[desc]->pt);
			attemptConnect(RoadEdge::TYPE_AVENUE, desc, ex_id, features);
			if (RoadGeneratorHelper::largestAngleBetweenEdges(roads, desc, RoadEdge::TYPE_AVENUE) > M_PI * 1.4f) {
				if (!attemptExpansion(RoadEdge::TYPE_AVENUE, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
					attemptExpansion2(RoadEdge::TYPE_AVENUE, desc, features[ex_id], seeds);
				}
			}

			char filename[255];
			sprintf(filename, "road_images/avenues_%d.jpg", iter);
			//RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);

			check();

			iter++;
		}
	}
	std::cout << "Avenue generation completed." << std::endl;

	seeds.clear();

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		RoadGeneratorHelper::removeAllDeadends(roads);
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
		
		// detect interesting shapes
		std::vector<std::vector<RoadEdgeDescs> > shapes;
		std::vector<std::vector<Patch> > patches;
		{
			shapes.resize(features.size());
			patches.resize(features.size());
			for (int i = 0; i < features.size(); ++i) {
				shapes[i] = features[i].shapes(RoadEdge::TYPE_STREET, G::getFloat("houghScale") * 0.2f, G::getFloat("streetPatchDistance"));
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
				//assert(GraphUtil::getDegree(roads, desc) == 1);

				RoadOutEdgeIter ei, eend;
				boost::tie(ei, eend) = boost::out_edges(desc, roads.graph);
				RoadGeneratorHelper::removeEdge(roads, desc, *ei);
				
				continue;
			}

			int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			//attemptConnect(RoadEdge::TYPE_STREET, desc, features[ex_id], seeds);
			if (RoadGeneratorHelper::largestAngleBetweenEdges(roads, desc, RoadEdge::TYPE_STREET) > M_PI * 1.4f) {
				if (!attemptExpansion(RoadEdge::TYPE_STREET, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
					attemptExpansion2(RoadEdge::TYPE_STREET, desc, features[ex_id], seeds);
				}
			}

			char filename[255];
			sprintf(filename, "road_images/streets_%d.jpg", iter);
			//RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);

			iter++;
		}
		std::cout << "Local street generation completed." << std::endl;
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
void PatchRoadGenerator::generateAvenueSeeds(std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	int numSeedsPerEx = hintLine.size() / features.size();

	for (int i = 0; i <features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;
			addAvenueSeed(features[i], hintLine[index], features[i].hintLine[j], index, i, seeds);
		}
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
bool PatchRoadGenerator::addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, std::list<RoadVertexDesc>& seeds) {
	if (!targetArea.contains(pt)) return false;

	RoadVertexDesc seedDesc = GraphUtil::getVertex(f.roads(RoadEdge::TYPE_AVENUE), ex_pt);

	// 頂点を追加し、シードとする
	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	roads.graph[desc]->properties["group_id"] = group_id;
	roads.graph[desc]->generationType = "example";
	roads.graph[desc]->properties["ex_id"] = ex_id;
	roads.graph[desc]->properties["example_desc"] = seedDesc;
	roads.graph[desc]->rotationAngle = G::getFloat("rotationAngle");//0.0f;
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
void PatchRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
	seeds.clear();

	// 頂点自体を、Local streetsのシードにする
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			std::cout << "Initial seed generation for the local street (vertex: " << (*vi) << ")" << std::endl;

			if (!roads.graph[*vi]->valid) continue;

			// The surrounding roads shoule not be used for the local street genration any more.
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
 * これ以上、この頂点からエッジを生成させたくない場合は、trueを返却する。
 * さらい生成させたい場合は、falseを返却する。
 */
bool PatchRoadGenerator::attemptConnect(int roadType, RoadVertexDesc srcDesc, int ex_id, std::vector<ExFeature>& features) {
	float length = 0.0f;

	if (roadType == RoadEdge::TYPE_AVENUE) {
		length = features[ex_id].avgAvenueLength;
	} else {
		length = features[ex_id].avgStreetLength;
	}

	// スナップする際、許容できる角度
	// attemptConnectでは、許容できる角度を厳しくしてみる。
	float roadAngleTolerance = G::getFloat("roadAngleTolerance") * 2.0f;

	std::vector<RoadEdgePtr> edges;

	// 近接頂点を探し、あればコネクトする
	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		float direction = atan2f((polyline[1] - polyline[0]).y(), (polyline[1] - polyline[0]).x());
		direction += M_PI;

		if (attemptConnectToVertex(roadType, srcDesc, features, length, G::getFloat("seaLevel"), direction, 0.3f, roadAngleTolerance)) return true;
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
			{
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
					if (roads.graph[nearestDesc]->properties["ex_id"].toInt() == pre_ex_id) {
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
					if (roads.graph[nearestDesc]->properties["ex_id"].toInt() == pre_ex_id) {
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
			if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, e->polyline, roadAngleTolerance)) {
				return false;
			}

			// スナップ先にとってredundantなら、コネクトしないで、終了。
			// つまり、trueを返却して、終わったことにしちゃう。
			/*
			if (roadType == RoadEdge::TYPE_AVENUE) {
				Polyline2D e2;
				e2.push_back(QVector2D(0, 0));
				e2.push_back(roads.graph[srcDesc]->pt - roads.graph[nearestDesc]->pt);
				if (RoadGeneratorHelper::isRedundantEdge(roads, nearestDesc, e2, roadAngleTolerance)) {
					// もしdegree=1なら、連なるエッジをごっそり削除
					if (roadType == RoadEdge::TYPE_AVENUE && GraphUtil::getDegree(roads, srcDesc) == 1) {
						RoadOutEdgeIter ei, eend;
						for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
							removeEdge(roads, srcDesc, *ei);
						}
					}

					return true;
				}
			}
			*/

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
			// エッジにスナップ
			nearestDesc = GraphUtil::splitEdge(roads, nearestEdgeDesc, intPoint);
			roads.graph[nearestDesc]->generationType = "snapped";
			roads.graph[nearestDesc]->properties["group_id"] = roads.graph[nearestEdgeDesc]->properties["group_id"];
			roads.graph[nearestDesc]->properties["ex_id"] = roads.graph[nearestEdgeDesc]->properties["ex_id"];
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
	//RoadVertexPtr ex_vertex = f.roads(roadType).graph[ex_v_desc];

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
		Polyline2D polyline = GraphUtil::getAdjoiningPolyline(roads, srcDesc);

		float min_cost = std::numeric_limits<float>::max();
		RoadVertexDesc root_desc;
		RoadVertexDesc connect_desc;

		std::reverse(polyline.begin(), polyline.end());
		for (int i = 0; i < patches.size(); ++i) {
			// 現在の頂点が属するパッチと、同じパッチは、使わない。
			// もし使っちゃったら、同じパッチが並んでしまう。
			if (i == roads.graph[srcDesc]->patchId) continue;

			RoadVertexDesc v_connector;
			RoadVertexDesc v_root;
			float cost = patches[i].getMinCost(polyline, v_connector, v_root);
			if (cost < min_cost) {
				min_cost = cost;
				patch_id = i;
				connect_desc = v_connector;
				root_desc = v_root;
			}
		}

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

	// 山チェック
	if (RoadGeneratorHelper::maxZ(replacementGraph, vboRenderManager) > 70.0f && RoadGeneratorHelper::diffSlope(replacementGraph, vboRenderManager) > 0.3f) {
		float max_rotation = G::getFloat("rotationForSteepSlope");
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
	
	rewrite(roadType, srcDesc, replacementGraph, seeds);

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

void PatchRoadGenerator::rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, std::list<RoadVertexDesc>& seeds) {
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
				}

				// 新規追加された頂点について、もとのreplacementGraphでconnectorなら、シードに追加
				if (replacementGraph.graph[*vi]->connector) {
					seeds.push_back(v_desc);
				}
			} else {
				roads.graph[v_desc]->properties = replacementGraph.graph[*vi]->properties;
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

			if (!GraphUtil::hasEdge(roads, conv[src], conv[tgt])) {
				RoadEdgePtr e = RoadEdgePtr(new RoadEdge(*replacementGraph.graph[*ei]));
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
void PatchRoadGenerator::attemptExpansion2(int roadType, RoadVertexDesc srcDesc, ExFeature& f, std::list<RoadVertexDesc> &seeds) {
	float length = 0.0f;

	if (roadType == RoadEdge::TYPE_AVENUE) {
		length = f.avgAvenueLength;
	} else {
		length = f.avgStreetLength;
	}

	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	std::vector<RoadEdgePtr> edges;

	// 当該頂点から出るエッジの方向を取得する
	float direction = RoadGeneratorHelper::getFirstEdgeAngle(roads, srcDesc);

	// 既にあるエッジと正反対の方向を計算
	direction += 3.141592653;

	// 道路生成用のカーネルを合成する
	synthesizeItem(roadType, srcDesc, length, 5.0f, edges);

	// 要注意！！
	// 既に標高はチェック済みなので、このチェックは不要のはず！！！
	float z = vboRenderManager->getMinTerrainHeight(roads.graph[srcDesc]->pt.x(), roads.graph[srcDesc]->pt.y());
	if (z < G::getFloat("seaLevel")) {
		printf("ERROR! The current vertex should be above sea leve.\n");
		assert(false);
		return;
	}

	int cnt = 0;
	for (int i = 0; i < edges.size(); ++i) {
		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, edges[i]->polyline, roadAngleTolerance)) continue;
		if (growRoadSegment(roadType, srcDesc, f, edges[i]->polyline, edges[i]->lanes, roadAngleTolerance, seeds)) {
			cnt++;
		}

		// hack: to avoid creating an intersection on the river.
		if (roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevel")) {
			if (cnt >= 1) break;
		}

	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 * この関数は、PM方式での道路生成でのみ使用される。
 */
bool PatchRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature& f, const Polyline2D &polyline, int lanes, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	float step = polyline.length() / 5.0f;
	float angle = atan2f(polyline[1].y() - polyline[0].y(), polyline[1].x() - polyline[0].x());

	RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
	new_edge->polyline.push_back(roads.graph[srcDesc]->pt);

	RoadVertexDesc tgtDesc;
	bool found = false;
	{
		if (RoadGeneratorHelper::getVertexForSnapping(*vboRenderManager, roads, srcDesc, polyline.length() * 2.0f, G::getFloat("seaLevel"), angle, 0.3f, tgtDesc)) {
			found = true;
		}

		if (found) {
			// もしスナップ先が、シードじゃないなら、エッジ生成をキャンセル
			if (std::find(seeds.begin(), seeds.end(), tgtDesc) == seeds.end()) {
				//（要検討。50%の確率ぐらいにすべきか？)
				if (Util::genRand(0, 1) < 0.5f) return false;
			}

			// もしスナップ先の頂点が、redundantなエッジを持っているなら、エッジ生成をキャンセル
			Polyline2D snapped_polyline;
			snapped_polyline.push_back(QVector2D(0, 0));
			snapped_polyline.push_back(QVector2D(roads.graph[srcDesc]->pt - roads.graph[tgtDesc]->pt));
			if (RoadGeneratorHelper::isRedundantEdge(roads, tgtDesc, snapped_polyline, 0.4f)) {
				//（とりあえず、ものすごい鋭角の場合は、必ずキャンセル)
				return false;
			}

			// もし他のエッジに交差するなら、エッジ生成をキャンセル
			// （キャンセルせずに、交差させるべき？）
			QVector2D intPoint;
			RoadEdgeDesc closestEdge;
			new_edge->polyline.push_back(roads.graph[tgtDesc]->pt);
			if (GraphUtil::isIntersect(roads, new_edge->polyline, srcDesc, closestEdge, intPoint)) {
				// もし交点が水面下ならキャンセル
				if (vboRenderManager->getMinTerrainHeight(intPoint.x(), intPoint.y()) < G::getFloat("seaLevel")) {
					return false;
				}

				if (Util::genRand(0, 1) < 0.5f) return false;

				new_edge->polyline[1] = intPoint;

				// 他のエッジにスナップ
				tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
				roads.graph[tgtDesc]->generationType = "snapped";
				roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
				roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
				roads.graph[tgtDesc]->properties.remove("example_desc");
			}
		}
	}

	if (!found) {
		// snap先がなければ、指定されたpolylineに従ってエッジを生成する。
		new_edge->polyline.push_back(roads.graph[srcDesc]->pt + polyline[1]);

		// もし、新規エッジが、既存グラフと交差するなら、エッジ生成をキャンセル
		QVector2D intPoint;
		RoadEdgeDesc closestEdge;
		if (GraphUtil::isIntersect(roads, new_edge->polyline, srcDesc, closestEdge, intPoint)) {
			// 60%の確率でキャンセル？
			if (Util::genRand(0, 1) < 0.6f) return false;

			// 交差する箇所で中断させる
			new_edge->polyline[1] = intPoint;

			// 他のエッジにスナップ
			tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
			roads.graph[tgtDesc]->generationType = "snapped";
			roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
			roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
			roads.graph[tgtDesc]->properties.remove("example_desc");

			found = true;
		}
	}

	if (!found) {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.last()));
		v->generationType = "pm";
		tgtDesc = GraphUtil::addVertex(roads, v);

		bool toAddSeed = true;

		// 水面下なら頂点の座標を岸ぎりぎりに移動する
		if (roadType == RoadEdge::TYPE_STREET) {
			float z = vboRenderManager->getMinTerrainHeight(roads.graph[tgtDesc]->pt.x(), roads.graph[tgtDesc]->pt.y());
			if (z < G::getFloat("seaLevel")) {
				RoadGeneratorHelper::cutEdgeByWater(new_edge->polyline, *vboRenderManager, G::getFloat("seaLevel"), 5.0f);
				roads.graph[tgtDesc]->pt = new_edge->polyline[1];

				// とりあえず、ローカルストリートについては、川の手間でストップさせて、シードに入れない
				// 要検討。。。
				toAddSeed = false;
			}
		}

		// エリア外なら、onBoundaryフラグをセット
		if (!targetArea.contains(roads.graph[tgtDesc]->pt)) {
			roads.graph[tgtDesc]->onBoundary = true;
		}

		// シードに追加
		// NOTE: エリア外でもとりあえずシードに追加する。
		// 理由: シード頂点へのスナップさせたい時があるので。
		if (toAddSeed) {
			seeds.push_back(tgtDesc);
		}
	}
	
	// エッジを追加
	// エッジをstepサイズに分割し、分割点に頂点を追加する。この頂点は、後でlocal street生成の初期シードとして使用する。
	{
		angle = atan2f(new_edge->polyline[1].y() - new_edge->polyline[0].y(), new_edge->polyline[1].x() - new_edge->polyline[0].x());

		RoadVertexDesc curDesc = srcDesc;
		QVector2D curPt = roads.graph[srcDesc]->pt;
		for (float len = step; len < new_edge->polyline.length() - step; len += step) {
			QVector2D nextPt = curPt + QVector2D(cosf(angle) * step, sinf(angle) * step);

			// 頂点を作成
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(nextPt));
			v->generationType = "pm";
			RoadVertexDesc nextDesc = GraphUtil::addVertex(roads, v);

			// エッジを作成
			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, lanes));
			e->polyline.push_back(curPt);
			e->polyline.push_back(nextPt);
			GraphUtil::addEdge(roads, curDesc, nextDesc, e);

			curPt = nextPt;
			curDesc = nextDesc;
		}

		// 最後のエッジを作成
		RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, lanes));
		e->polyline.push_back(curPt);
		e->polyline.push_back(roads.graph[tgtDesc]->pt);
		GraphUtil::addEdge(roads, curDesc, tgtDesc, e);
	}

	return true;
}

/**
 * 道路を延長し、川を越えさせる。指定された長さ伸ばして、依然として川の中なら、延長をキャンセルする。
 *
 * @return			延長したらtrueを返却する
 */
bool PatchRoadGenerator::extendRoadAcrossRiver(int roadType, RoadVertexDesc v_desc, std::list<RoadVertexDesc> &seeds, float max_length) {
	// 既存のエッジから方向を決定する
	QVector2D dir;
	int lanes;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			lanes = roads.graph[*ei]->lanes;
			Polyline2D polyline  = GraphUtil::orderPolyLine(roads, *ei, v_desc);
			dir = polyline[1] - polyline[0];
			break;
		}
	}
	dir.normalize();

	QVector2D pt = roads.graph[v_desc]->pt - dir * max_length;

	float z = vboRenderManager->getMinTerrainHeight(pt.x(), pt.y());
	if (z < G::getFloat("seaLevel")) return false;

	// エッジ生成
	RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, lanes));
	e->polyline.push_back(roads.graph[v_desc]->pt);
	e->polyline.push_back(pt);

	// もし、新規エッジが、既存グラフと交差するなら、エッジ生成をキャンセル
	RoadVertexDesc tgtDesc;
	QVector2D intPoint;
	RoadEdgeDesc closestEdge;
	if (GraphUtil::isIntersect(roads, e->polyline, v_desc, closestEdge, intPoint)) {
		if (vboRenderManager->getMinTerrainHeight(intPoint.x(), intPoint.y()) < G::getFloat("seaLevel")) {
			return false;
		}

		// 60%の確率でキャンセル？
		if (Util::genRand(0, 1) < 0.6f) return false;

		// 交差する箇所で中断させる
		e->polyline[1] = intPoint;

		// 他のエッジにスナップ
		tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
		roads.graph[tgtDesc]->generationType = "snapped";
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
		roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
		roads.graph[tgtDesc]->properties.remove("example_desc");
	} else {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
		v->generationType = "pm";
		tgtDesc = GraphUtil::addVertex(roads, v);

		// エリア外なら、onBoundaryフラグをセット
		if (!targetArea.contains(roads.graph[tgtDesc]->pt)) {
			roads.graph[tgtDesc]->onBoundary = true;
		}

		// シードに追加
		// NOTE: エリア外でもとりあえずシードに追加する。
		// 理由: シード頂点へのスナップさせたい時があるので。
		seeds.push_back(tgtDesc);
	}

	// エッジを追加
	GraphUtil::addEdge(roads, v_desc, tgtDesc, e);

	return true;
}

/**
 * PMに従って、カーネルを合成する
 * edgesに、エッジのリストを格納して返却する。
 * 各エッジのpolylineは、(0, 0)からスタートする。つまり、始点となる頂点の座標を(0, 0)とするということだ。
 *
 * @param roadType			avenue / local street
 * @param v_desc			現在の頂点ID
 * @param length			エッジの長さの基準値（統計データから取得したもの）
 * @param step				各セグメントの長さ
 * @param edges [OUT]		返却するエッジリスト
 */
void PatchRoadGenerator::synthesizeItem(int roadType, RoadVertexDesc v_desc, float length, float step, std::vector<RoadEdgePtr> &edges) {
	// 当該頂点から出るエッジの方向を取得する
	float direction = RoadGeneratorHelper::getFirstEdgeAngle(roads, v_desc);

	for (int i = 0; i < 4; ++i) {
		RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
		e->polyline.push_back(QVector2D(0, 0));
		float len = Util::genRand(length * 0.7, length * 1.5);
		e->polyline.push_back(QVector2D(len * cosf(direction), len * sinf(direction)));
		
		edges.push_back(e);

		direction += 3.141592653 * 0.5f;
	}
}

int PatchRoadGenerator::defineExId(const QVector2D& pt) {
	std::vector<float> sigma;
	sigma.push_back(G::getDouble("interpolationSigma1"));
	sigma.push_back(G::getDouble("interpolationSigma2"));
	for (int i = 2; i < features.size(); ++i) {
		sigma.push_back(G::getDouble("interpolationSigma2"));
	}

	int numSeedsPerEx = hintLine.size() / features.size();

	float* pdf = new float[hintLine.size()];
	for (int i = 0; i < features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;

			//float dist = (hintLine[index] - pt).length();
			
			// とりあえずblending用に、X軸方向での距離だけを使用してみよう
			float dist = fabs((hintLine[index] - pt).x());

			if (dist == 0.0f) dist = 0.01f;
			float cdf = sigma[i] / dist;

			if (index == 0) {
				pdf[index] = cdf;
			} else {
				pdf[index] = pdf[index - 1] + cdf;
			}
		}
	}

	float rand = Util::genRand(0.0f, pdf[hintLine.size() - 1]);
	for (int i = 0; i < features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;

			if (rand < pdf[index]) {
				return i;
			}
		}
	}

	return features.size() - 1;
}

void PatchRoadGenerator::check() {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		if (roads.graph[*vi]->generationType == "") {
			printf("No generation type is set for vertex %d.\n", *vi);
			assert(false);
			return;
		}
	}


}
