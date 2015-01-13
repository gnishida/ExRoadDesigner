#include "PatchTensorRoadGenerator.h"
#include <QTest>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include "global.h"
#include "Util.h"
#include "ConvexHull.h"
#include "TopNSearch.h"
#include "GraphUtil.h"
#include "RoadGeneratorHelper.h"
#include "SmallBlockRemover.h"
#include "ShapeDetector.h"

void PatchTensorRoadGenerator::generateRoadNetwork() {
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
		// detect interesting shapes
		std::vector<std::vector<RoadEdgeDescs> > shapes;
		{
			shapes.resize(features.size());
			for (int i = 0; i < features.size(); ++i) {
				//shapes[i] = ShapeDetector::detect(features[i].reducedRoads(RoadEdge::TYPE_AVENUE), 0.2, G::getFloat("avenuePatchDistance"));
				//shapes[i] = features[i].avenueShapes;
				shapes[i] = features[i].shapes(RoadEdge::TYPE_AVENUE, G::getFloat("houghScale"), G::getFloat("avenuePatchDistance"));

				for (int j = 0; j < shapes[i].size(); ++j) {
					for (int k = 0 ; k < shapes[i][j].size(); ++k) {
						features[i].reducedRoads(RoadEdge::TYPE_AVENUE).graph[shapes[i][j][k]]->properties["shape_id"] = j;
					}
				}
			}
		}

		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numAvenueIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y(), true);
			if (z < G::getFloat("seaLevelForAvenue")) {
				std::cout << "attemptExpansion (avenue): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (avenue): " << i << " (Seed: " << desc << ")" << std::endl;
			int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			if (roads.graph[desc]->properties["generation_type"] == "example") {
				attemptExpansion(RoadEdge::TYPE_AVENUE, desc, features[ex_id], shapes[ex_id], seeds);
			} else {
				attemptExpansion2(RoadEdge::TYPE_AVENUE, desc, features[ex_id], shapes[ex_id], seeds);
			}
		}
	}

	seeds.clear();

	// Avenueをクリーンナップ
	if (G::getBool("cleanAvenues")) {
		RoadGeneratorHelper::extendDanglingEdges(roads, 300.0f);
		RoadGeneratorHelper::removeDeadend(roads);
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
		
		// detect interesting shapes
		std::vector<std::vector<RoadEdgeDescs> > shapes;
		{
			shapes.resize(features.size());
			for (int i = 0; i < features.size(); ++i) {
				//shapes[i] = ShapeDetector::detect(features[i].reducedRoads(RoadEdge::TYPE_STREET), 0.2, G::getFloat("streetPatchDistance"));
				//shapes[i] = features[i].streetShapes;
				shapes[i] = features[i].shapes(RoadEdge::TYPE_STREET, G::getFloat("houghScale"), G::getFloat("streetPatchDistance"));

				for (int j = 0; j < shapes[i].size(); ++j) {
					for (int k = 0 ; k < shapes[i][j].size(); ++k) {
						features[i].reducedRoads(RoadEdge::TYPE_STREET).graph[shapes[i][j][k]]->properties["shape_id"] = j;
					}
				}
			}
		}

		int i;
		for (i = 0; !seeds.empty() && i < G::getInt("numStreetIterations"); ++i) {
			RoadVertexDesc desc = seeds.front();
			seeds.pop_front();

			float z = vboRenderManager->getTerrainHeight(roads.graph[desc]->pt.x(), roads.graph[desc]->pt.y(), true);
			if (z < G::getFloat("seaLevelForStreet")) {
				std::cout << "attemptExpansion (street): " << i << " (skipped because it is under the sea or on the mountains)" << std::endl;
				continue;
			}

			std::cout << "attemptExpansion (street): " << i << " (Seed: " << desc << ")" << std::endl;
			int ex_id = roads.graph[desc]->properties["ex_id"].toInt();
			if (roads.graph[desc]->properties["generation_type"] == "example") {
				attemptExpansion(RoadEdge::TYPE_STREET, desc, features[ex_id], shapes[ex_id], seeds);
			} else {
				attemptExpansion2(RoadEdge::TYPE_STREET, desc, features[ex_id], shapes[ex_id], seeds);
			}
		}
	}

	// 指定されたエリアでCropping
	if (G::getBool("cropping")) {
		GraphUtil::extractRoads2(roads, targetArea);
	}

	// Local Streetsをクリーンナップ
	if (G::getBool("cleanStreets")) {
		RoadGeneratorHelper::removeDeadend(roads);
	}

	GraphUtil::cleanEdges(roads);
}

/**
 * シード頂点を生成する。
 */
void PatchTensorRoadGenerator::generateAvenueSeeds(std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	// B-splineの位置を計算
	Polyline2D bsplineHintLine;
	bsplineHintLine.push_back(hintLine[0]);
	for (int i = 1; i < hintLine.size() - 1; ++i) {
		QVector2D pt1 = (hintLine[i - 1] + hintLine[i]) * 0.5f;
		QVector2D pt2 = (hintLine[i] + hintLine[i + 1]) * 0.5f;
		bsplineHintLine.push_back((pt1 + pt2) * 0.5f);
	}
	bsplineHintLine.push_back(hintLine.last());
	
	int numSeedsPerEx = hintLine.size() / features.size();

	for (int i = 0; i <features.size(); ++i) {
		for (int j = 0; j < numSeedsPerEx; ++j) {
			int index = i * numSeedsPerEx + j;


			float angle;

			if (index > 0 && index < hintLine.size() - 1) {
				QVector2D ex_vec = features[i].hintLine[j] - features[i].hintLine[j - 1];
				QVector2D vec = hintLine[index] - hintLine[index - 1];

				float angle1 = Util::diffAngle(vec, ex_vec, false);
				QVector2D diffVec1(cosf(angle1), sinf(angle1));

				ex_vec = features[i].hintLine[j + 1] - features[i].hintLine[j];
				vec = hintLine[index + 1] - hintLine[index];

				float angle2 = Util::diffAngle(vec, ex_vec, false);
				QVector2D diffVec2(cosf(angle2), sinf(angle2));
				
				QVector2D averagedVec = (diffVec1 + diffVec2) * 0.5f;

				angle = atan2f(averagedVec.y(), averagedVec.x());
			} else if (index > 0) {
				QVector2D ex_vec = features[i].hintLine[j] - features[i].hintLine[j - 1];
				QVector2D vec = hintLine[index] - hintLine[index - 1];

				angle = Util::diffAngle(vec, ex_vec, false);
			} else {
				QVector2D ex_vec = features[i].hintLine[j + 1] - features[i].hintLine[j];
				QVector2D vec = hintLine[index + 1] - hintLine[index];

				angle = Util::diffAngle(vec, ex_vec, false);
			}


			addAvenueSeed(features[i], bsplineHintLine[index], features[i].hintLine[j], index, i, angle, seeds);
		}
	}

	std::cout << "computing tensor field..." << std::endl;
	initTensorField();
	std::cout << "done." << std::endl;

	//hintLine = bsplineHintLine;
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
bool PatchTensorRoadGenerator::addAvenueSeed(ExFeature &f, const QVector2D &pt, const QVector2D &ex_pt, int group_id, int ex_id, float angle, std::list<RoadVertexDesc>& seeds) {
	if (!targetArea.contains(pt)) return false;

	// Avenueカーネルの中で、offsetの位置に最も近いものを探す
	RoadVertexDesc seedDesc = GraphUtil::getVertex(f.reducedRoads(RoadEdge::TYPE_AVENUE), ex_pt);

	// 頂点を追加し、シードとする
	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	//roads.graph[desc]->properties["used"] = true;
	roads.graph[desc]->properties["group_id"] = group_id;
	roads.graph[desc]->properties["generation_type"] = "example";
	roads.graph[desc]->properties["ex_id"] = ex_id;
	roads.graph[desc]->properties["example_desc"] = seedDesc;
	roads.graph[desc]->properties["rotation_angle"] = angle;
	seeds.push_back(desc);

	return true;
}

/**
 * Local Street用のシードを生成する。
 */
void PatchTensorRoadGenerator::generateStreetSeeds(std::list<RoadVertexDesc> &seeds) {
	seeds.clear();

	// 頂点自体を、Local streetsのシードにする
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			if (!roads.graph[*vi]->properties.contains("example_desc")) continue;

			if (!targetArea.contains(roads.graph[*vi]->pt)) continue;
			float z = vboRenderManager->getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), true);
			if (z < G::getFloat("seaLevelForStreet")) continue;

			// ターゲットエリア座標空間から、Example座標空間へのオフセットを計算
			int ex_id = roads.graph[*vi]->properties["ex_id"].toInt();
			RoadVertexDesc ex_v_desc = roads.graph[*vi]->properties["example_desc"].toUInt();

			RoadVertexDesc seedDesc;
			if (GraphUtil::getVertex(features[ex_id].reducedRoads(RoadEdge::TYPE_STREET), features[ex_id].reducedRoads(RoadEdge::TYPE_AVENUE).graph[ex_v_desc]->pt, 1.0f, seedDesc)) {
				seeds.push_back(*vi);
				roads.graph[*vi]->properties["example_street_desc"] = seedDesc;
			}

		}
	}
}

/**
 * このシードを使って、道路生成する。
 * Exampleベースで生成する。
 */
void PatchTensorRoadGenerator::attemptExpansion(int roadType, RoadVertexDesc srcDesc, ExFeature &f, std::vector<RoadEdgeDescs> &shapes, std::list<RoadVertexDesc> &seeds) {
	RoadVertexDesc ex_v_desc;
	if (roadType == RoadEdge::TYPE_AVENUE) {
		ex_v_desc = roads.graph[srcDesc]->properties["example_desc"].toUInt();
	} else {
		ex_v_desc = roads.graph[srcDesc]->properties["example_street_desc"].toUInt();
	}
	float angle = roads.graph[srcDesc]->properties["rotation_angle"].toFloat();
	RoadVertexPtr ex_vertex = f.reducedRoads(roadType).graph[ex_v_desc];

	float roadSnapFactor = G::getFloat("roadSnapFactor");
	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	int shape_index;
	if (!roads.graph[srcDesc]->properties.contains("shape") && RoadGeneratorHelper::isShape(f.reducedRoads(roadType), ex_v_desc, shapes, shape_index)) {
		RoadGraph replacementGraph;
		buildReplacementGraphByExample(roadType, replacementGraph, srcDesc, f.reducedRoads(roadType), ex_v_desc, 0.0f/*angle*/, shapes[shape_index]);

		warp(roadType, srcDesc, replacementGraph, tensorField);

		if (isValidRule(srcDesc, replacementGraph)) {
			rewrite(roadType, srcDesc, replacementGraph, seeds);
		}
	} else {
		BBox bbox = targetArea.envelope();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(ex_v_desc, f.reducedRoads(roadType).graph); ei != eend; ++ei) {
			RoadVertexDesc tgt = boost::target(*ei, f.reducedRoads(roadType).graph);

			Polyline2D polyline = GraphUtil::orderPolyLine(f.reducedRoads(roadType), *ei, ex_v_desc);

			QVector2D offset = polyline[0];
			polyline.translate(offset * -1.0f);

			//polyline.rotate(-Util::rad2deg(angle));

			polyline = GraphUtil::finerEdge(polyline, 10.0f);
			Polyline2D rotatedPolyline;
			rotatedPolyline.push_back(polyline[0]);
			for (int i = 1; i < polyline.size(); ++i) {
				QVector2D pt = roads.graph[srcDesc]->pt + rotatedPolyline[i - 1];
				float angle = tensorField.get(pt);
				rotatedPolyline.push_back(rotatedPolyline[i - 1] + Util::rotate(polyline[i] - polyline[i - 1], angle));
			}

			if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, rotatedPolyline, 0.3f)) continue;

			growRoadSegment(roadType, srcDesc, f, rotatedPolyline, f.reducedRoads(roadType).graph[*ei]->lanes, tgt, true, roadSnapFactor, roadAngleTolerance, seeds);
		}
	}
}

void PatchTensorRoadGenerator::buildReplacementGraphByExample(int roadType, RoadGraph &replacementGraph, RoadVertexDesc srcDesc, RoadGraph &exRoads, RoadVertexDesc ex_srcDesc, float angle, RoadEdgeDescs &shape) {
	replacementGraph.clear();

	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// add vertices of the interesting shape
	{
		QHash<RoadVertexDesc, bool> vertices;

		for (int i = 0; i < shape.size(); ++i) {
			RoadVertexDesc src = boost::source(shape[i], exRoads.graph);
			RoadVertexDesc tgt = boost::target(shape[i], exRoads.graph);

			vertices[src] = true;
			vertices[tgt] = true;
		}

		// create the vertices list such that "ex_srcDesc" will be the first element.
		std::vector<RoadVertexDesc> vertices2;
		vertices2.push_back(ex_srcDesc);
		vertices.remove(ex_srcDesc);
		for (QHash<RoadVertexDesc, bool>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
			vertices2.push_back(it.key());
		}

		//for (QHash<RoadVertexDesc, bool>::iterator it = vertices.begin(); it != vertices.end(); ++it) {
		for (int i = 0; i < vertices2.size(); ++i) {
			// Transformした後の座標を計算
			QVector2D pt = Util::transform(exRoads.graph[vertices2[i]]->pt, exRoads.graph[ex_srcDesc]->pt, angle, roads.graph[srcDesc]->pt);

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
			v->onBoundary = exRoads.graph[vertices2[i]]->onBoundary;
			RoadVertexDesc v_desc = GraphUtil::addVertex(replacementGraph, v);
			replacementGraph.graph[v_desc]->properties["generation_type"] = roads.graph[srcDesc]->properties["generation_type"];
			if (roadType == RoadEdge::TYPE_AVENUE) {
				replacementGraph.graph[v_desc]->properties["example_desc"] = vertices2[i];
			} else {
				replacementGraph.graph[v_desc]->properties["example_street_desc"] = vertices2[i];
			}
			replacementGraph.graph[v_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			replacementGraph.graph[v_desc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];
			replacementGraph.graph[v_desc]->properties["shape"] = true;
			replacementGraph.graph[v_desc]->properties["rotation_angle"] = roads.graph[srcDesc]->properties["rotation_angle"];
			conv[vertices2[i]] = v_desc;
		}
	}

	// 円のエッジを追加
	{
		for (int i = 0; i < shape.size(); ++i) {
			RoadVertexDesc src = boost::source(shape[i], exRoads.graph);
			RoadVertexDesc tgt = boost::target(shape[i], exRoads.graph);

			RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(*exRoads.graph[shape[i]]));

			// エッジを座標変換する
			edge->polyline.translate(-exRoads.graph[ex_srcDesc]->pt);
			edge->polyline.rotate(Util::rad2deg(-angle));
			edge->polyline.translate(roads.graph[srcDesc]->pt);

			RoadEdgeDesc e_desc = GraphUtil::addEdge(replacementGraph, conv[src], conv[tgt], edge);
			replacementGraph.graph[e_desc]->properties["generation_type"] = roads.graph[srcDesc]->properties["generation_type"];
			replacementGraph.graph[e_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
			replacementGraph.graph[e_desc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];
			replacementGraph.graph[e_desc]->properties["shape"] = true;
		}
	}
}

/**
 * Warp the replacement graph according to the tensor field.
 * The replacement graph is already mapped in the target domain space, so it is straightforward to just apply the tensor field to warp the graph.
 */
void PatchTensorRoadGenerator::warp(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, TensorField &tensorField) {
	QMap<RoadVertexDesc, QVector2D> newPositions;
	newPositions[0] = replacementGraph.graph[0]->pt;

	QList<RoadVertexDesc> queue;
	queue.push_back(0);

	QMap<RoadVertexDesc, bool> usedVertices;
	usedVertices[0] = true;

	//float angle = tensorField.get(roads.graph[srcDesc]->pt);

	QMap<RoadEdgeDesc, bool> usedEdges;

	while (!queue.empty()) {
		RoadVertexDesc v_desc = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, replacementGraph.graph); ei != eend; ++ei) {
			if (usedEdges.contains(*ei)) continue;

			usedEdges[*ei] = true;

			RoadVertexDesc tgt = boost::target(*ei, replacementGraph.graph);

			Polyline2D polyline = GraphUtil::orderPolyLine(replacementGraph, *ei, v_desc);
			//QVector2D offset = polyline[0];
			//polyline.translate(-offset);
			polyline = GraphUtil::finerEdge(polyline, 10.0f);
			Polyline2D rotatedPolyline;

			//rotatedPolyline.push_back(replacementGraph.graph[v_desc]->pt);
			rotatedPolyline.push_back(newPositions[v_desc]);
			
			for (int i = 1; i < polyline.size(); ++i) {
				float angle = tensorField.get(rotatedPolyline[i - 1]);
				rotatedPolyline.push_back(rotatedPolyline[i - 1] +  Util::rotate(polyline[i] - polyline[i - 1], angle));
			}
						
			if (!usedVertices.contains(tgt)) {
				replacementGraph.graph[*ei]->polyline = rotatedPolyline;
				//replacementGraph.graph[tgt]->pt = rotatedPolyline.back();
				newPositions[tgt] = rotatedPolyline.back();
				queue.push_back(tgt);
				usedVertices[tgt] = true;
			} else {
				GraphUtil::movePolyline(replacementGraph, rotatedPolyline, rotatedPolyline[0], newPositions[tgt]);
				replacementGraph.graph[*ei]->polyline = rotatedPolyline;			
			}
		}
	}

	// update the vertex positions to the new ones
	for (QMap<RoadVertexDesc, QVector2D>::iterator it = newPositions.begin(); it != newPositions.end(); ++it) {
		replacementGraph.graph[it.key()]->pt = it.value();
	}
}

void PatchTensorRoadGenerator::rewrite(int roadType, RoadVertexDesc srcDesc, RoadGraph &replacementGraph, std::list<RoadVertexDesc>& seeds) {
	QMap<RoadVertexDesc, RoadVertexDesc> conv;

	// add vertices
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(replacementGraph.graph); vi != vend; ++vi) {
			if (!replacementGraph.graph[*vi]->valid) continue;

			RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*replacementGraph.graph[*vi]));
			RoadVertexDesc v_desc;
			if (!GraphUtil::getVertex(roads, replacementGraph.graph[*vi]->pt, 1.0f, v_desc)) {
				v_desc = GraphUtil::addVertex(roads, v);

				// if this vertex is on the boundary, then the example-based cannot be applied for the further generation from this.
				if (replacementGraph.graph[*vi]->onBoundary) {
					roads.graph[v_desc]->properties["generation_type"] = "mixed";
					roads.graph[v_desc]->properties.remove("example_desc");
					roads.graph[v_desc]->properties.remove("rotation_angle");
				}

				// if the vertex is a dead end, then, it will be added to the seeds.
				//if (GraphUtil::getDegree(replacementGraph, *vi) == 1) {
					seeds.push_back(v_desc);
				//}
			} else {
				roads.graph[v_desc]->properties = replacementGraph.graph[*vi]->properties;
				
				// initial seed自体がrewriteされる場合、seedに入れないと、そこからエッジが延びていかない
				if (GraphUtil::getDegree(roads, v_desc) == 0) {
					seeds.push_back(v_desc);
				}
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

bool PatchTensorRoadGenerator::isValidRule(RoadVertexDesc srcDesc, RoadGraph &replacementGraph) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		//if (src == srcDesc || tgt == srcDesc) continue;

		QVector2D intPoint;
		if (GraphUtil::isIntersect(replacementGraph, roads.graph[*ei]->polyline, intPoint)) return false;
	}

	return true;
}

/**
 * このシードを使って、PM方式で道路生成する。
 */
void PatchTensorRoadGenerator::attemptExpansion2(int roadType, RoadVertexDesc srcDesc, ExFeature &f, std::vector<RoadEdgeDescs> &shapes, std::list<RoadVertexDesc> &seeds) {
	float snapThreshold;

	if (roadType == RoadEdge::TYPE_AVENUE) {
		snapThreshold = f.avgAvenueLength * 1.2f;
	} else {
		snapThreshold = f.avgStreetLength * 0.2f;
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
			roads.graph[tgtDesc]->properties["generation_type"] = "snapped";
			roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closeEdge]->properties["group_id"];
			roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closeEdge]->properties["ex_id"];
			roads.graph[tgtDesc]->properties.remove("example_desc");
			roads.graph[tgtDesc]->properties["parent"] = srcDesc;

			GraphUtil::snapVertex(roads, srcDesc, tgtDesc);
			return;
		}
	}

	// 道路生成用のカーネルを合成する
	std::vector<RoadEdgePtr> edges;
	if ((roadType == RoadEdge::TYPE_AVENUE && roads.graph[srcDesc]->properties.contains("example_desc")) || (roadType == RoadEdge::TYPE_STREET && roads.graph[srcDesc]->properties.contains("example_street_desc"))) {
		RoadVertexDesc ex_v_desc;
		if (roadType == RoadEdge::TYPE_AVENUE) {
			ex_v_desc = roads.graph[srcDesc]->properties["example_desc"].toUInt();
		} else {
			ex_v_desc = roads.graph[srcDesc]->properties["example_street_desc"].toUInt();
		}
		float angle = roads.graph[srcDesc]->properties["rotation_angle"].toFloat();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(ex_v_desc, f.reducedRoads(roadType).graph); ei != eend; ++ei) {
			RoadVertexDesc tgt = boost::target(*ei, f.reducedRoads(roadType).graph);

			Polyline2D polyline = GraphUtil::orderPolyLine(f.reducedRoads(roadType), *ei, ex_v_desc);

			QVector2D offset = polyline[0];
			polyline.translate(offset * -1.0f);
			polyline.rotate(-Util::rad2deg(angle));

			RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(*f.reducedRoads(roadType).graph[*ei]));
			edge->polyline = polyline;
			edges.push_back(edge);
		}
	} else {
		float rotation_angle;
		std::pair<int, RoadVertexDesc> pair = synthesizeItem(roadType, srcDesc, shapes, edges, rotation_angle);
		roads.graph[srcDesc]->properties["ex_id"] = pair.first;
		roads.graph[srcDesc]->properties["example_desc"] = pair.second;
		roads.graph[srcDesc]->properties["rotation_angle"] = rotation_angle;
	}
	
	float roadSnapFactor = G::getFloat("roadSnapFactor");
	float roadAngleTolerance = G::getFloat("roadAngleTolerance");

	for (int i = 0; i < edges.size(); ++i) {
		if (RoadGeneratorHelper::isRedundantEdge(roads, srcDesc, edges[i]->polyline, roadAngleTolerance)) continue;
		growRoadSegment(roadType, srcDesc, f, edges[i]->polyline, edges[i]->lanes, 0, false, roadSnapFactor, roadAngleTolerance, seeds);
	}
}

/**
 * 指定されたpolylineに従って、srcDesc頂点からエッジを伸ばす。
 * エッジの端点が、srcDescとは違うセルに入る場合は、falseを返却する。
 */
bool PatchTensorRoadGenerator::growRoadSegment(int roadType, RoadVertexDesc srcDesc, ExFeature &f, const Polyline2D &polyline, int lanes, RoadVertexDesc next_ex_v_desc, bool byExample, float snapFactor, float angleTolerance, std::list<RoadVertexDesc> &seeds) {
	float sigma2 = SQR(G::getFloat("interpolationSigma1"));
	float threshold1 = G::getFloat("interpolationThreshold1");

	bool intercepted = false;

	float z0 = vboRenderManager->getTerrainHeight(roads.graph[srcDesc]->pt.x(), roads.graph[srcDesc]->pt.y(), true);

	// 新しいエッジを生成
	RoadEdgePtr new_edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
	for (int i = 0; i < polyline.size(); ++i) {
		QVector2D pt = roads.graph[srcDesc]->pt + polyline[i];

		new_edge->polyline.push_back(pt);

		// 水没、または、山の上なら、道路生成をストップ
		float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y(), true);
		//if (z < G::getFloat("seaLevel")) {
		if ((roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForAvenue")) || (roadType == RoadEdge::TYPE_STREET && z < G::getFloat("seaLevelForStreet"))) {
			// 最初っから水没している場合は、そもそもエッジ生成をキャンセル
			if (new_edge->polyline.size() <= 1) return false;

			RoadGeneratorHelper::bendEdgeBySteepElevationChange(roadType, new_edge->polyline, vboRenderManager);
			pt = new_edge->polyline.back();

			intercepted = true;
		}

		// if the slope is too steep, bend the direction to avoid climbing up
		if (z - z0 > 14.0f) {
			RoadGeneratorHelper::bendEdgeBySteepElevationChange(new_edge->polyline, z0, vboRenderManager);
			pt = new_edge->polyline.back();
			z = vboRenderManager->getTerrainHeight(pt.x(), pt.y(), true);
			z0 = z;
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
		}

		if (intercepted) break;
	}

	if (new_edge->polyline.size() == 1) return false;

	RoadVertexDesc tgtDesc;

	if (byExample) {
		//snapFactor = 0.01f;
		snapFactor *= 0.4f;
	}

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

	if (!snapDone && GraphUtil::getVertex(roads, new_edge->polyline.last(), new_edge->polyline.length() * snapFactor, srcDesc, tgtDesc)) {
		snapTried = true;

		if (byExample && roads.graph[tgtDesc]->properties["generation_type"] == "example" && roads.graph[tgtDesc]->properties["group_id"] == roads.graph[srcDesc]->properties["group_id"]) {
			angleTolerance = 0.3f;
		}

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

	if (!snapDone && GraphUtil::getEdge(roads, new_edge->polyline.last(), new_edge->polyline.length() * snapFactor, srcDesc, closestEdge, intPoint)) {
		snapTried = true;

		if (byExample && roads.graph[closestEdge]->properties["generation_type"] == "example" && roads.graph[tgtDesc]->properties["group_id"] == roads.graph[srcDesc]->properties["group_id"]) {
			angleTolerance = 0.3f;
		}

		// 他のエッジにスナップ
		tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
		roads.graph[tgtDesc]->properties["generation_type"] = "snapped";
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
		roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
		roads.graph[tgtDesc]->properties.remove("example_desc");
		roads.graph[tgtDesc]->properties["parent"] = srcDesc;

		GraphUtil::movePolyline(roads, new_edge->polyline, roads.graph[srcDesc]->pt, roads.graph[tgtDesc]->pt);
		if (!GraphUtil::hasRedundantEdge(roads, tgtDesc, new_edge->polyline, angleTolerance)) {
			snapDone = true;
		}
	}

	if (snapTried && !snapDone) return false;

	if (!snapDone) {
		// 頂点を追加
		RoadVertexPtr v = RoadVertexPtr(new RoadVertex(new_edge->polyline.last()));
		tgtDesc = GraphUtil::addVertex(roads, v);
		roads.graph[tgtDesc]->properties["parent"] = srcDesc;

		// 新しい頂点にgroup_idを引き継ぐ
		roads.graph[tgtDesc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
		roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];

		// 新しい頂点のgeneration_typeを設定
		roads.graph[tgtDesc]->properties["generation_type"] = byExample ? "example" : "mixed";

		if (targetArea.contains(new_edge->polyline.last())) {
			roads.graph[tgtDesc]->properties["generation_type"] = "mixed";

			// シードに追加する
			if (byExample && !intercepted) {	// Exampleベースで、且つ、途中で生成ストップしていない場合（つまり、Exampleそのまま）
				//if (roadType == RoadEdge::TYPE_AVENUE || GraphUtil::getDegree(feature.reducedRoads(roadType), next_ex_v_desc) > 1) {
				if (f.reducedRoads(roadType).graph[next_ex_v_desc]->onBoundary || GraphUtil::getDegree(f.reducedRoads(roadType), next_ex_v_desc) > 1) {
					seeds.push_back(tgtDesc);
				}

				// 対応するExampleが存在する場合は、それを設定する
				//if (!f.reducedRoads(roadType).graph[next_ex_v_desc]->properties.contains("used") && GraphUtil::getDegree(feature.reducedRoads(roadType), next_ex_v_desc) > 1) {
				if (GraphUtil::getDegree(f.reducedRoads(roadType), next_ex_v_desc) > 1) {
					//f.reducedRoads(roadType).graph[next_ex_v_desc]->properties["used"] = true;
					roads.graph[tgtDesc]->properties["generation_type"] = "example";
					roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];
					if (roadType == RoadEdge::TYPE_AVENUE) {
						roads.graph[tgtDesc]->properties["example_desc"] = next_ex_v_desc;
					} else {
						roads.graph[tgtDesc]->properties["example_street_desc"] = next_ex_v_desc;
					}
					roads.graph[tgtDesc]->properties["angle"] = tensorField.get(roads.graph[tgtDesc]->pt);
				}
			} else {
				seeds.push_back(tgtDesc);
			}
		} else {
			// ターゲットエリアの外に出たら
			if (byExample) {
				roads.graph[tgtDesc]->properties["generation_type"] = "example";
				roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];
				if (roadType == RoadEdge::TYPE_AVENUE) {
					roads.graph[tgtDesc]->properties["example_desc"] = next_ex_v_desc;
				} else {
					roads.graph[tgtDesc]->properties["example_street_desc"] = next_ex_v_desc;
				}
			} else {
				roads.graph[tgtDesc]->properties["generation_type"] = "mixed";
			}

			roads.graph[tgtDesc]->onBoundary = true;
		}

		// Example道路のDeadendは、Deadendとして正式に登録する
		if (byExample && GraphUtil::getDegree(f.reducedRoads(roadType), next_ex_v_desc) == 1) {
			roads.graph[tgtDesc]->properties["deadend"] = true;
		}
	}

	RoadEdgeDesc e_desc = GraphUtil::addEdge(roads, srcDesc, tgtDesc, new_edge);

	// 新しいエッジにgroup_idを引き継ぐ
	roads.graph[e_desc]->properties["group_id"] = roads.graph[srcDesc]->properties["group_id"];
	roads.graph[e_desc]->properties["ex_id"] = roads.graph[srcDesc]->properties["ex_id"];

	// 新しいエッジのgeneration_typeを設定
	roads.graph[e_desc]->properties["generation_type"] = byExample ? "example" : "mixed";
	if (roads.graph[srcDesc]->properties["generation_type"] == "mixed") {
		roads.graph[e_desc]->properties["generation_type"] = "mixed";
	}

	return true;
}

/**
 * PMに従って、カーネルを合成する
 */
std::pair<int, RoadVertexDesc> PatchTensorRoadGenerator::synthesizeItem(int roadType, RoadVertexDesc v_desc, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges, float &rotation_angle) {
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

	// build the cumulative densitify function
	std::vector<float> cdf;
	{
		float total = 0.0f;

		std::vector<float> pdf;

		// build the pdf
		int numSeedsPerEx = hintLine.size() / features.size();
		for (int i = 0; i < features.size(); ++i) {
			for (int j = 0; j < numSeedsPerEx; ++j) {
				int index = i * numSeedsPerEx + j;

				// デバッグ用
				//if (i == 0 && roads.graph[v_desc]->pt.x() > hintLine[1].x()) continue;
				//if (i == 1 && roads.graph[v_desc]->pt.x() < hintLine[0].x()) continue;

				float dist2 = (roads.graph[v_desc]->pt - hintLine[index]).lengthSquared() / D2;
				float p = expf(-dist2 / 2.0f / sigma2[i]);
				pdf.push_back(p);
				total += p;

			}
		}

		// normailze
		for (int i = 0; i < pdf.size(); ++i) {
			pdf[i] /= total;
		}

		// build the cdf
		for (int i = 0; i < pdf.size(); ++i) {
			if (i == 0) {
				cdf.push_back(pdf[i]);
			} else {
				cdf.push_back(cdf[i - 1] + pdf[i]);
			}
		}
	}

	int index = Util::sampleFromCdf(cdf);

	int numSeedsPerEx = hintLine.size() / features.size();
	int ex_id = index / numSeedsPerEx;
		
	RoadVertexDesc ex_desc = RoadGeneratorHelper::createEdgesByExample(features[ex_id].reducedRoads(roadType), direction, shapes, edges, rotation_angle);
	return std::make_pair<int, RoadVertexDesc>(ex_id, ex_desc);
}

void PatchTensorRoadGenerator::initTensorField() {
	tensorField.init(targetArea, hintLine, features, 0.1f);

	tensorField.save("tensor.jpg");
}

