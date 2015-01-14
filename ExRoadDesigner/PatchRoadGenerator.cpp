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
				printf("ERROR: this check should have been done before.\n");
				continue;
			}

			// 水中なら、スキップする
			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y(), true);
			if (z < G::getFloat("seaLevelForAvenue")) {
				std::cout << "attemptExpansion (avenue): " << iter << " (skipped because it is under the sea)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << iter << " (Seed: " << desc << ")" << std::endl;
			//int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			int ex_id = defineExId(roads.graph[desc]->pt);

			if (!attemptConnect(RoadEdge::TYPE_AVENUE, desc, ex_id, features, seeds)) {
				if (!attemptExpansion(RoadEdge::TYPE_AVENUE, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
					attemptExpansion2(RoadEdge::TYPE_AVENUE, desc, features[ex_id], seeds);
				}
			}

			char filename[255];
			sprintf(filename, "road_images/avenues_%d.jpg", iter);
			RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);

			iter++;
		}
	}
	std::cout << "Avenue generation completed." << std::endl;

	seeds.clear();

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		removeDeadend(roads);
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

			// 既に3つ以上エッジがある場合は、スキップする
			if (GraphUtil::getDegree(roads, desc) >= 3) {
				continue;
			}

			// エリアの外なら、スキップする
			if (!targetArea.contains(roads.graph[desc]->pt)) {
				printf("ERROR: this check should have been done before.\n");
				continue;
			}

			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y(), true);
			if (z < G::getFloat("seaLevelForStreet")) {
				std::cout << "attemptExpansion (street): " << iter << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (street): " << iter << " (Seed: " << desc << ")" << std::endl;

			int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			//if (!attemptConnect(RoadEdge::TYPE_STREET, desc, features[ex_id], seeds)) {
				if (!attemptExpansion(RoadEdge::TYPE_STREET, desc, ex_id, features[ex_id], patches[ex_id], seeds)) {
					attemptExpansion2(RoadEdge::TYPE_STREET, desc, features[ex_id], seeds);
				}
			//}

			char filename[255];
			sprintf(filename, "road_images/streets_%d.jpg", iter);
			RoadGeneratorHelper::saveRoadImage(roads, seeds, filename);

			iter++;
		}
		std::cout << "Local street generation completed." << std::endl;
	}

	// 指定されたエリアでCropping
	if (G::getBool("cropping")) {
		GraphUtil::extractRoads2(roads, targetArea);
	}

	if (G::getBool("cleanStreets")) {
		RoadGeneratorHelper::removeDeadend(roads);
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
	roads.graph[desc]->properties["generation_type"] = "example";
	roads.graph[desc]->properties["ex_id"] = ex_id;
	roads.graph[desc]->properties["example_desc"] = seedDesc;
	roads.graph[desc]->rotationAngle = G::getFloat("rotationAngle");//0.0f;
	seeds.push_back(desc);

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

			if (roads.graph[*vi]->properties.contains("example_desc")) {
				float z = vboRenderManager->getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), true);
				if (z < G::getFloat("seaLevelForStreet")) continue;

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
 * コネクトできなかった場合は、falseを返却する。
 */
bool PatchRoadGenerator::attemptConnect(int roadType, RoadVertexDesc srcDesc, int ex_id, std::vector<ExFeature>& features, std::list<RoadVertexDesc> &seeds) {
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

	// 当該頂点から出るエッジをリストアップし、基底の方向を決定する
	float direction = 0.0f;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline  = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
			QVector2D vec = polyline[1] - polyline[0];
			direction = atan2f(vec.y(), vec.x());
			break;
		}
	}

	// 既にあるエッジと正反対の方向を計算
	direction += 3.141592653;

	// ものすごい近くに、他の頂点がないかあれば、そことコネクトして終わり。
	// それ以外の余計なエッジは生成しない。さもないと、ものすごい密度の濃い道路網になっちゃう。
	{
		RoadVertexDesc nearestDesc;
		if (GraphUtil::getVertexExceptDeadend(roads, srcDesc, length, direction, 0.3f, nearestDesc)) {
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

				if (has_ex_v1 && has_ex_v2) {
					QVector2D ex_vec = features[pre_ex_id].roads(roadType).graph[ex_v2_desc]->pt - features[pre_ex_id].roads(roadType).graph[ex_v1_desc]->pt;
					QVector2D vec = roads.graph[nearestDesc]->pt - roads.graph[srcDesc]->pt;
					if ((vec - ex_vec).lengthSquared() <= 0.1f) return false;
				}
			}

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);

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

			if (!GraphUtil::isIntersect(roads, e->polyline)) {
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);
				return true;
			}
		}
	}

	// 近くにエッジがあれば、コネクト
	{
		RoadVertexDesc nearestDesc;
		RoadEdgeDesc nearestEdgeDesc;
		QVector2D intPoint;
		if (GraphUtil::getCloseEdge(roads, srcDesc, length, direction, 0.3f, nearestEdgeDesc, intPoint)) {
			// エッジにスナップ
			nearestDesc = GraphUtil::splitEdge(roads, nearestEdgeDesc, intPoint);
			roads.graph[nearestDesc]->properties["generation_type"] = "snapped";
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

	// ものすごい近くに、他の頂点がないかあれば、そことコネクトして終わり。
	// それ以外の余計なエッジは生成しない。さもないと、ものすごい密度の濃い道路網になっちゃう。
	{
		RoadVertexDesc nearestDesc;
		if (GraphUtil::getVertexExceptDeadend(roads, srcDesc, length, direction, 1.5f, nearestDesc)) {
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

				if (has_ex_v1 && has_ex_v2) {
					QVector2D ex_vec = features[pre_ex_id].roads(roadType).graph[ex_v2_desc]->pt - features[pre_ex_id].roads(roadType).graph[ex_v1_desc]->pt;
					QVector2D vec = roads.graph[nearestDesc]->pt - roads.graph[srcDesc]->pt;
					if ((vec - ex_vec).lengthSquared() <= 0.1f) return false;
				}
			}

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);

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

			if (!GraphUtil::isIntersect(roads, e->polyline)) {
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);
				return true;
			}
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
			printf("ERROR!!!!!!!!!!!!!!");
		}
	
		buildReplacementGraphByExample(roadType, replacementGraph, srcDesc, ex_id, f.roads(roadType), ex_v_desc, angle, patches[patch_id], patch_id);
	}

	// replacementGraphが、既に生成済みのグラフと重なるかどうかチェック
	if (GraphUtil::isIntersect(replacementGraph, roads)) {
		int xxx = 0;

		return false;
	}
	

	rewrite(roadType, srcDesc, replacementGraph, seeds);

	return true;
}

/**
 * replacement グラフを構築する。ただし、現在頂点srcDescから伸びる既存エッジと重複するエッジは削除しておく。
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
	RoadVertexDesc root_desc;

	// add vertices of the patch
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(patch.roads.graph); vi != vend; ++vi) {
			if (patch.roads.graph[*vi]->onBoundary) {
				int xxx = 0;
			}

			// Transformした後の座標を計算
			QVector2D pt = Util::transform(patch.roads.graph[*vi]->pt, exRoads.graph[ex_srcDesc]->pt, angle, roads.graph[srcDesc]->pt);

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*patch.roads.graph[*vi]));
			v->pt = pt;
			v->rotationAngle = angle;
			v->properties["ex_id"] = ex_id;//roads.graph[srcDesc]->properties["ex_id"];
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			if (v->patchId != patchId) {
				int xxxx = 0;
			}
			//v->patchId = patchId;

			// 元のExampleで境界上の頂点だったかどうかは、ターゲットエリアでは関係ないので、一旦リセットする
			// GEN 1/14  ↓コメントアウト
			//v->onBoundary = false;

			RoadVertexDesc v_desc = GraphUtil::addVertex(replacementGraph, v);
			conv[*vi] = v_desc;

			if (patch.roads.graph[*vi]->properties["example_desc"].toUInt() == ex_srcDesc) {
				root_desc = v_desc;
			}
		}
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
				if (replacementGraph.graph[*ei2]->connector) continue;

				Polyline2D polyline2 = GraphUtil::orderPolyLine(replacementGraph, *ei2, root_desc);

				if (Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]) < 0.3f) {
					// この方向に伸びるエッジを削除する
					removeEdge(replacementGraph, root_desc, *ei2);
				}
			}
		}
	}

	GraphUtil::clean(replacementGraph);
}

/**
 * replacement graphを生成する。
 * ただし、既存グラフと接続するコネクタ部分を削除する。
 */
void PatchRoadGenerator::buildReplacementGraphByExample2(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, int ex_id, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, Patch &patch, int patchId, RoadVertexDesc v_connect, RoadVertexDesc v_root) {
	replacementGraph.clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;
	RoadVertexDesc connector_desc;
	RoadVertexDesc root_desc;

	// add vertices of the patch
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(patch.roads.graph); vi != vend; ++vi) {
			if (patch.roads.graph[*vi]->onBoundary) {
				int xxx = 0;
			}

			// Transformした後の座標を計算
			QVector2D pt = Util::transform(patch.roads.graph[*vi]->pt, exRoads.graph[ex_srcDesc]->pt, angle, roads.graph[srcDesc]->pt);

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*patch.roads.graph[*vi]));
			v->pt = pt;
			v->rotationAngle = angle;
			v->properties["ex_id"] = ex_id;//roads.graph[srcDesc]->properties["ex_id"];
			v->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			if (v->patchId != patchId) {
				int xxxx = 0;
			}
			v->patchId = patchId;

			// 元のExampleで境界上の頂点だったかどうかは、ターゲットエリアでは関係ないので、一旦リセットする
			v->onBoundary = false;

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

			removeEdge(replacementGraph, connector_desc, *ei);
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
					removeEdge(replacementGraph, root_desc, *ei2);
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
			RoadVertexDesc v_desc;
			if (!GraphUtil::getVertex(roads, replacementGraph.graph[*vi]->pt, 1.0f, v_desc)) {
				// 頂点を新規追加
				v_desc = GraphUtil::addVertex(roads, v);

				// エリア外なら、onBoundaryフラグをセット
				if (!targetArea.contains(roads.graph[v_desc]->pt)) {
					roads.graph[v_desc]->onBoundary = true;
				}

				// 新規追加された頂点について、もとのreplacementGraphでdegree==1で、deadendでないなら、シードに追加
				/*if (GraphUtil::getDegree(replacementGraph, *vi) == 1 
					&& replacementGraph.graph[*vi]->deadend == false
					&& !roads.graph[v_desc]->onBoundary) {*/
				// 新規追加された頂点について、もとのreplacementGraphでconnectorなら、シードに追加
				if (replacementGraph.graph[*vi]->connector) {
					seeds.push_back(v_desc);
				}
			} else {
				roads.graph[v_desc]->properties = replacementGraph.graph[*vi]->properties;

				// initial seed自体がrewriteされる場合、seedに入れないと、そこからエッジが延びていかない
				//if (GraphUtil::getDegree(roads, v_desc) == 0) {
				//	seeds.push_back(v_desc);
				//}
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

	// 当該頂点から出るエッジをリストアップし、基底の方向を決定する
	float direction = 0.0f;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline  = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
			QVector2D vec = polyline[1] - polyline[0];
			direction = atan2f(vec.y(), vec.x());
			break;
		}
	}

	// 既にあるエッジと正反対の方向を計算
	direction += 3.141592653;

	// ものすごい近くに、他の頂点がないかあれば、そことコネクトして終わり。
	// それ以外の余計なエッジは生成しない。さもないと、ものすごい密度の濃い道路網になっちゃう。
	{
		RoadVertexDesc nearestDesc;
		if (GraphUtil::getVertexExceptDeadend(roads, srcDesc, length, direction, 0.3f, nearestDesc)) {
			// もし、既にエッジがあるなら、キャンセル
			if (GraphUtil::hasEdge(roads, srcDesc, nearestDesc)) return;

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);

			if (!GraphUtil::isIntersect(roads, e->polyline)) {
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);
				return;
			}
		}
	}

	// 近くにエッジがあれば、コネクト
	{
		RoadVertexDesc nearestDesc;
		RoadEdgeDesc nearestEdgeDesc;
		QVector2D intPoint;
		if (GraphUtil::getCloseEdge(roads, srcDesc, length, direction, 0.3f, nearestEdgeDesc, intPoint)) {
			// エッジにスナップ
			nearestDesc = GraphUtil::splitEdge(roads, nearestEdgeDesc, intPoint);
			roads.graph[nearestDesc]->properties["generation_type"] = "snapped";
			roads.graph[nearestDesc]->properties["group_id"] = roads.graph[nearestEdgeDesc]->properties["group_id"];
			roads.graph[nearestDesc]->properties["ex_id"] = roads.graph[nearestEdgeDesc]->properties["ex_id"];
			roads.graph[nearestDesc]->properties.remove("example_desc");

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);
			RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);

			return;
		}
	}

	// ものすごい近くに、他の頂点がないかあれば、そことコネクトして終わり。
	// それ以外の余計なエッジは生成しない。さもないと、ものすごい密度の濃い道路網になっちゃう。
	{
		RoadVertexDesc nearestDesc;
		if (GraphUtil::getVertexExceptDeadend(roads, srcDesc, length, direction, 1.5f, nearestDesc)) {
			// もし、既にエッジがあるなら、キャンセル
			if (GraphUtil::hasEdge(roads, srcDesc, nearestDesc)) return;

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
			e->polyline.push_back(roads.graph[srcDesc]->pt);
			e->polyline.push_back(roads.graph[nearestDesc]->pt);

			if (!GraphUtil::isIntersect(roads, e->polyline)) {
				RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, nearestDesc, e);
				return;
			}
		}
	}

	// 道路生成用のカーネルを合成する
	synthesizeItem(roadType, srcDesc, length, edges);

	float z = vboRenderManager->getTerrainHeight(roads.graph[srcDesc]->pt.x(), roads.graph[srcDesc]->pt.y(), true);

	int cnt = 0;
	for (int i = 0; i < edges.size(); ++i) {
		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, edges[i]->polyline, roadAngleTolerance)) continue;
		if (growRoadSegment(roadType, srcDesc, f, edges[i]->polyline, edges[i]->lanes, roadAngleTolerance, seeds)) {
			cnt++;
		}

		// hack: to avoid creating an intersection on the river.
		if (roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForStreet")) {
			if (cnt >= 1) break;
		}

	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 */
bool PatchRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature& f, const Polyline2D &polyline, int lanes, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	float angle = atan2f(polyline[1].y() - polyline[0].y(), polyline[1].x() - polyline[0].x());

	RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
	new_edge->polyline.push_back(roads.graph[srcDesc]->pt);

	RoadVertexDesc tgtDesc;
	bool found = false;
	{
		// 指定された方向で、直近の頂点があるかチェックする。
		// まずは、指定された方向に非常に近い頂点があるかチェックする。この際、距離は少し遠くまで許容する。
		// 次に、方向のレンジを少し広げて、その代わり、距離を短くして、改めてチェックする。
		/*if (GraphUtil::getVertex(roads, srcDesc, polyline.length() * 2.0f, angle, 0.1f, tgtDesc)) {
			found = true;
		} else if (GraphUtil::getVertex(roads, srcDesc, polyline.length() * 1.5f, angle, 0.2f, tgtDesc)) {
			found = true;
		} else if (GraphUtil::getVertex(roads, srcDesc, polyline.length(), angle, 0.3f, tgtDesc)) {
			found = true;
		}
		*/
		if (GraphUtil::getVertexExceptDeadend(roads, srcDesc, polyline.length() * 2.0f, angle, 0.3f, tgtDesc)) {
			found = true;
		}

		if (found) {
			// もしスナップ先が、シードじゃないなら、エッジ生成をキャンセル
			if (std::find(seeds.begin(), seeds.end(), tgtDesc) == seeds.end()) {
				//（要検討。80%の確率ぐらいにすべきか？)
				if (Util::genRand(0, 1) < 0.8f) return false;
			}

			// もしスナップ先の頂点が、redundantなエッジを持っているなら、エッジ生成をキャンセル
			Polyline2D snapped_polyline;
			snapped_polyline.push_back(QVector2D(0, 0));
			snapped_polyline.push_back(QVector2D(roads.graph[srcDesc]->pt - roads.graph[tgtDesc]->pt));
			if (RoadGeneratorHelper::isRedundantEdge(roads, tgtDesc, snapped_polyline, 0.3f)) {
				//（とりあえず、ものすごい鋭角の場合は、必ずキャンセル)
				return false;
			}

			// もし他のエッジに交差するなら、エッジ生成をキャンセル
			// （キャンセルせずに、交差させるべき？）
			QVector2D intPoint;
			RoadEdgeDesc closestEdge;
			new_edge->polyline.push_back(roads.graph[tgtDesc]->pt);
			if (GraphUtil::isIntersect(roads, new_edge->polyline, srcDesc, closestEdge, intPoint)) {
				if (Util::genRand(0, 1) < 0.5f) return false;

				new_edge->polyline[1] = intPoint;

				// 他のエッジにスナップ
				tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
				roads.graph[tgtDesc]->properties["generation_type"] = "snapped";
				roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
				roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
				roads.graph[tgtDesc]->properties.remove("example_desc");
			} else {
				new_edge->polyline.push_back(roads.graph[tgtDesc]->pt);
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
			roads.graph[tgtDesc]->properties["generation_type"] = "snapped";
			roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
			roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
			roads.graph[tgtDesc]->properties.remove("example_desc");

			found = true;
		}
	}

	if (!found) {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.last()));
		tgtDesc = GraphUtil::addVertex(roads, v);

		// エリアの外なら、onBoundaryフラグをセット
		if (!targetArea.contains(roads.graph[tgtDesc]->pt)) {
			roads.graph[tgtDesc]->onBoundary = true;
		}

		// エリア内なら、seedsに追加
		if (!roads.graph[tgtDesc]->onBoundary) {
			seeds.push_back(tgtDesc);
		}
	}
	
	// エッジを追加
	// エッジをstepサイズに分割し、分割点に頂点を追加する。この頂点は、後でlocal street生成の初期シードとして使用する。
	{
		float step = new_edge->polyline.length() / 5.0f;
		angle = atan2f(new_edge->polyline[1].y() - new_edge->polyline[0].y(), new_edge->polyline[1].x() - new_edge->polyline[0].x());

		RoadVertexDesc curDesc = srcDesc;
		QVector2D curPt = roads.graph[srcDesc]->pt;
		for (float len = step; len < new_edge->polyline.length() - step; len += step) {
			QVector2D nextPt = curPt + QVector2D(cosf(angle) * step, sinf(angle) * step);

			// 頂点を作成
			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(nextPt));
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
	//RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, tgtDesc, new_edge);

	return true;
}

/**
 * PMに従って、カーネルを合成する
 * edgesに、エッジのリストを格納して返却する。
 * 各エッジのpolylineは、(0, 0)からスタートする。つまり、始点となる頂点の座標を(0, 0)とするということだ。
 */
void PatchRoadGenerator::synthesizeItem(int roadType, RoadVertexDesc v_desc, float length, std::vector<RoadEdgePtr> &edges) {
	// 当該頂点から出るエッジをリストアップし、基底の方向を決定する
	float direction = 0.0f;
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

	for (int i = 0; i < 4; ++i) {
		RoadEdgePtr e = RoadEdgePtr(new RoadEdge(roadType, 1));
		e->polyline.push_back(QVector2D(0, 0));
		//e->polyline.push_back(QVector2D(500.0f * cosf(direction), 500.0f * sinf(direction)));
		float len = Util::genRand(length * 0.7, length * 1.5);
		e->polyline.push_back(QVector2D(len * cosf(direction), len * 1.5f * sinf(direction)));
		
		edges.push_back(e);

		direction += 3.141592653 * 0.5f;
	}
}

/**
 * 指定された頂点から伸びるエッジを削除する。
 */
void PatchRoadGenerator::removeEdge(RoadGraph& roads, RoadVertexDesc srcDesc, RoadEdgeDesc start_e_desc) {
	QMap<RoadVertexDesc, bool> visited;
	std::list<RoadVertexDesc> queue;

	roads.graph[start_e_desc]->valid = false;
	RoadVertexDesc tgt = boost::target(start_e_desc, roads.graph);
	if (GraphUtil::getDegree(roads, tgt) == 1) {
		queue.push_back(tgt);
	}

	if (GraphUtil::getDegree(roads, srcDesc) == 0) roads.graph[srcDesc]->valid = false;

	while (!queue.empty()) {
		RoadVertexDesc v = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			roads.graph[*ei]->valid = false;
			if (GraphUtil::getDegree(roads, v) == 0) roads.graph[v]->valid = false;
			if (GraphUtil::getDegree(roads, tgt) == 0) roads.graph[tgt]->valid = false;

			if (visited[tgt]) continue;

			// 上で既に一本のエッジを無効にしているので、もともとdegree=2の頂点は、残り一本だけエッジが残っているはず。
			// なので、 == 2　ではなく、 == 1　とする。
			if (GraphUtil::getDegree(roads, tgt) == 1) {
				queue.push_back(tgt);
			}
		}
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

void PatchRoadGenerator::removeDeadend(RoadGraph& roads) {
	bool removed = false;

	do {
		removed = false;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (roads.graph[src]->onBoundary || roads.graph[tgt]->onBoundary) continue;
			if (roads.graph[src]->deadend) continue;
			if (roads.graph[tgt]->deadend) continue;
			if (roads.graph[src]->fixed || roads.graph[tgt]->fixed) continue;


			if (GraphUtil::getDegree(roads, src) == 1) {
				removeEdge(roads, src, *ei);
				removed = true;
			}
			
			if (GraphUtil::getDegree(roads, tgt) == 1) {
				removeEdge(roads, tgt, *ei);
				removed = true;
			}
		}
	} while (removed);
}