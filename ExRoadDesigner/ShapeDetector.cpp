#include "ShapeDetector.h"
#include "GraphUtil.h"
#include <QHash>
#include "CircleHoughTransform.h"
#include "RoadGeneratorHelper.h"

/**
 * 与えられた道路網に対して、パッチを検出し、各頂点に、属するpatchIdをセットする。
 *
 * @param roads			道路網
 * @param maxRadius		検知したい円の半径の最大値
 * @param threshold		近接頂点を探すためのしきい値
 * @return				パッチのエッジリスト
 */
std::vector<RoadEdgeDescs> ShapeDetector::detect(RoadGraph &roads, float maxRadius, float threshold) {
	float threshold2 = SQR(threshold);

	RoadVertexDesc center;

	QMap<RoadVertexDesc, bool> usedVertices;
	QMap<RoadEdgeDesc, int> usedEdges;

	std::vector<RoadEdgeDescs> shapes;
	// detect circles
	shapes = CircleHoughTransform::detect(roads, maxRadius, 30.0f);
	time_t start = clock();
	for (int i = 0; i < shapes.size(); ++i) {
		for (int j = 0; j < shapes[i].size(); ++j) {
			RoadVertexDesc src = boost::source(shapes[i][j], roads.graph);
			RoadVertexDesc tgt = boost::target(shapes[i][j], roads.graph);

			usedVertices[src] = true;
			usedVertices[tgt] = true;
			usedEdges[shapes[i][j]] = 2;

			// パッチIDを、属する頂点に設定する
			roads.graph[src]->patchId = i;
			roads.graph[tgt]->patchId = i;
		}
	}
	time_t end = clock();
	std::cout << "Circle detection: " << (double)(end - start) / CLOCKS_PER_SEC << " [sec]" << std::endl;

	// expand circles
	for (int i = 0; i < shapes.size(); ++i) {
		addVerticesToCircle(roads, shapes[i], i, threshold, usedVertices, usedEdges);
	}
	
	// detect close vertices
	{
		time_t start = clock();
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			if (usedVertices.contains(*vi)) continue;

			// don't start from the non-intersection.
			if (GraphUtil::getDegree(roads, *vi) <= 2) continue;

			std::vector<RoadEdgeDesc> shape;
			addVerticesToGroup(roads, *vi, shapes.size(), threshold, shape, usedVertices, usedEdges);
			shapes.push_back(shape);
		}
		time_t end = clock();
		std::cout << "Close vertices detection: " << (double)(end - start) / CLOCKS_PER_SEC << " [sec]" << std::endl;
	}

	// 一旦、shapeのエッジの両端頂点を、usedVerticesに格納する。
	// なぜこれが必要か？
	// 上のdetect close verticesでは、degree==2の頂点は、わざとusedVerticesに格納しなかったから。
	for (int i = 0; i < shapes.size(); ++i) {
		for (int j = 0; j < shapes[i].size(); ++j) {
			RoadVertexDesc src = boost::source(shapes[i][j], roads.graph);
			RoadVertexDesc tgt = boost::source(shapes[i][j], roads.graph);
			usedVertices[src] = true;
			usedVertices[tgt] = true;
		}
	}

	// 基本的に、全てのエッジを、いずれかのパッチに属するようにしたい。
	// しかし、上のclose verticesの方法だと、交差点のないエッジや、孤立したエッジは、パッチになれない。
	// degree>2の制約をdegree=2に変更し、上のチェックを繰り返すことで、交差点のないエッジに対応
	{
		time_t start = clock();
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			if (usedVertices.contains(*vi)) continue;

			// start from the non-intersection.
			if (GraphUtil::getDegree(roads, *vi) == 2) {
				std::vector<RoadEdgeDesc> shape;
				
				bool visited = false;
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
					if (!roads.graph[*ei]->valid) continue;
					if (usedEdges.contains(*ei)) {
						usedEdges[*ei]++;
					} else {
						usedEdges[*ei] = 1;
					}
					shape.push_back(*ei);
				}

				usedVertices[*vi] = true;

				roads.graph[*vi]->patchId = shapes.size();
				shapes.push_back(shape);
			}
		}
		time_t end = clock();
		std::cout << "Close vertices detection 2: " << (double)(end - start) / CLOCKS_PER_SEC << " [sec]" << std::endl;
	}

	// さらに、孤立したエッジのために、
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			if (usedEdges.contains(*ei)) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (usedVertices.contains(src) || usedVertices.contains(tgt)) continue;

			if (GraphUtil::getDegree(roads, src) == 1 && GraphUtil::getDegree(roads, tgt) == 1) {
				std::vector<RoadEdgeDesc> shape;
				shape.push_back(*ei);

				// これらの頂点の属するpatchIdをセットする
				roads.graph[src]->patchId = shapes.size();
				roads.graph[tgt]->patchId = shapes.size();

				shapes.push_back(shape);

			}		
		}
	}

	// さらに、patchIdが未設定の頂点のために、
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (roads.graph[src]->patchId < 0 && roads.graph[tgt]->patchId < 0) {
				printf("ERROR: both src and tgt do not have patchId set.\n");
				continue;
			}

			if (roads.graph[src]->patchId < 0) {
				roads.graph[src]->patchId = roads.graph[tgt]->patchId;
			}
			if (roads.graph[tgt]->patchId < 0) {
				roads.graph[tgt]->patchId = roads.graph[src]->patchId;
			}
		}
	}

	return shapes;
}

void ShapeDetector::addVerticesToCircle(RoadGraph &roads, RoadEdgeDescs& shape, int patchId, float threshold, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, int> &usedEdges) {
	float threshold2 = SQR(threshold);

	std::list<RoadVertexDesc> queue;
	QMap<RoadVertexDesc, bool> visited;

	for (int i = 0; i < shape.size(); ++i) {
		RoadVertexDesc src = boost::source(shape[i], roads.graph);
		RoadVertexDesc tgt = boost::target(shape[i], roads.graph);

		if (!visited.contains(src)) {
			queue.push_back(src);
			visited[src] = true;
		}
		if (!visited.contains(tgt)) {
			queue.push_back(tgt);
			visited[tgt] = true;
		}
	}

	// パッチに含まれるエッジのセット
	QMap<RoadEdgeDesc, bool> edge_descs;
	for (int i = 0; i < shape.size(); ++i) {
		edge_descs[shape[i]] = true;
	}

	while (!queue.empty()) {
		RoadVertexDesc desc = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			//if (usedEdges.contains(*ei)) continue;

			if (!usedEdges.contains(*ei)) {
				usedEdges[*ei] = 1;
			}
			if (std::find(shape.begin(), shape.end(), *ei) == shape.end()) {
				shape.push_back(*ei);
			}
			edge_descs[*ei] = true;
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (usedVertices.contains(tgt)) continue;
			if (visited.contains(tgt)) continue;

			float length = 0.0f;
			if (roads.graph[desc]->properties.contains("length")) {
				length = roads.graph[desc]->properties["length"].toFloat();
			}

			// 頂点tgtのdegreeを計算する。ただし、edge_descsがtrueになっているエッジの数はカウントに入れない。
			int degree = 0;
			RoadOutEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = boost::out_edges(tgt, roads.graph); ei2 != eend2; ++ei2) {
				if (!roads.graph[*ei2]->valid) continue;
				if (edge_descs.contains(*ei2)) continue;
				degree++;
			}

			// 頂点tgtのdegree==1、または、現在の頂点から頂点tgtまでの距離が近いなら、その先に延ばしていく
			if (degree == 1 || length + roads.graph[*ei]->polyline.length() <= threshold) {
				queue.push_back(tgt);
				visited[tgt] = true;

				if (GraphUtil::getDegree(roads, tgt) <= 2) {
					roads.graph[tgt]->properties["length"] = length + roads.graph[*ei]->polyline.length();

					// この頂点は、複数のパッチに属する可能性があるので、パッチIDが未設定の場合だけセットする
					// ただし、境界上の頂点には、パッチIDをセットしない。なぜなら、境界上の頂点は、対応するパッチがないから！！
					if (roads.graph[tgt]->patchId < 0 && !roads.graph[tgt]->onBoundary) {
						roads.graph[tgt]->patchId = patchId;
					}
				} else {
					roads.graph[tgt]->properties["length"] = 0.0f;
					usedVertices[tgt] = true;

					// パッチIDをセットする
					roads.graph[tgt]->patchId = patchId;
				}
			}
		}
	}
}

/**
 * 頂点srcDescからスタートし、近隣のエッジのリストをshapeに格納する。
 *
 * @param roads			道路グラフ
 * @param srcDesc		この頂点からスタート
 * @param patchId		近隣の頂点に、このpatchIdを設定する。ただし、すでに非負の値が設定されている場合は、上書きしないで、そのまま
 * @param threshold		近隣チェックのためのしきい値
 * @param shape [OUT]	近隣のエッジリストを格納する
 * @param usedVertices	訪問済みの頂点。ただし、degree=2の頂点には、このフラグはセットしない！
 * @param usedEdges		属するパッチの数
 */
void ShapeDetector::addVerticesToGroup(RoadGraph &roads, RoadVertexDesc srcDesc, int patchId, float threshold, RoadEdgeDescs &shape, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, int> &usedEdges) {
	// パッチに含まれるエッジのセット
	QMap<RoadEdgeDesc, bool> edge_descs;

	QMap<RoadVertexDesc, bool> visited;

	std::list<RoadVertexDesc> queue;
	queue.push_back(srcDesc);
	usedVertices[srcDesc] = true;
	visited[srcDesc] = true;
	roads.graph[srcDesc]->properties["length"] = 0.0f;

	// パッチIDをセットする
	roads.graph[srcDesc]->patchId = patchId;

	while (!queue.empty()) {
		RoadVertexDesc desc = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			//if (usedEdges.contains(*ei)) continue;

			if (!usedEdges.contains(*ei)) {
				usedEdges[*ei] = 1;
			} else {
				usedEdges[*ei]++;
			}
			if (!edge_descs.contains(*ei)) {
				shape.push_back(*ei);
				edge_descs[*ei] = true;
			}
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);
			if (usedVertices.contains(tgt)) continue;
			if (visited.contains(tgt)) continue;

			if (GraphUtil::getDegree(roads, tgt) <= 2 || roads.graph[desc]->properties["length"].toFloat() + roads.graph[*ei]->polyline.length() <= threshold) {
				queue.push_back(tgt);
				visited[tgt] = true;

				if (GraphUtil::getDegree(roads, tgt) <= 2) {
					roads.graph[tgt]->properties["length"] = roads.graph[desc]->properties["length"].toFloat() + roads.graph[*ei]->polyline.length();

					// この頂点は、複数のパッチに属する可能性があるので、パッチIDが未設定の場合だけセットする
					// // ただし、境界上の頂点には、パッチIDをセットしない。なぜなら、境界上の頂点は、対応するパッチがないから！！
					if (roads.graph[tgt]->patchId < 0 && !roads.graph[tgt]->onBoundary) {
						roads.graph[tgt]->patchId = patchId;
					}
				} else {
					roads.graph[tgt]->properties["length"] = 0.0f;
					usedVertices[tgt] = true;

					// パッチIDをセットする
					roads.graph[tgt]->patchId = patchId;
				}
			}
		}
	}
}
