#include "ShapeDetector.h"
#include "GraphUtil.h"
#include <QHash>
#include "CircleHoughTransform.h"

std::vector<RoadEdgeDescs> ShapeDetector::detect(RoadGraph &roads, float scale, float threshold) {
	float threshold2 = SQR(threshold);

	RoadVertexDesc center;

	QMap<RoadVertexDesc, bool> usedVertices;
	QMap<RoadEdgeDesc, bool> usedEdges;

	// detect circles
	std::vector<RoadEdgeDescs> shapes = CircleHoughTransform::detect(roads, scale);
	time_t start = clock();
	for (int i = 0; i < shapes.size(); ++i) {
		for (int j = 0; j < shapes[i].size(); ++j) {
			RoadVertexDesc src = boost::source(shapes[i][j], roads.graph);
			RoadVertexDesc tgt = boost::target(shapes[i][j], roads.graph);

			usedVertices[src] = true;
			usedVertices[tgt] = true;
			usedEdges[shapes[i][j]] = true;
		}
	}
	time_t end = clock();
	std::cout << "Circle detection: " << (double)(end - start) / CLOCKS_PER_SEC << " [sec]" << std::endl;

	// detect close vertices
	start = clock();
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			if (usedVertices.contains(*vi)) continue;

			// don't include deadend
			if (GraphUtil::getDegree(roads, *vi) == 1) continue;

			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;
				if (usedEdges.contains(*ei)) continue;

				RoadVertexDesc tgt = boost::target(*ei, roads.graph);
				if (usedVertices.contains(tgt)) continue;

				// don't include deadend
				if (GraphUtil::getDegree(roads, tgt) == 1) continue;

				if ((roads.graph[*vi]->pt - roads.graph[tgt]->pt).lengthSquared() <= threshold2) {
					std::vector<RoadEdgeDesc> shape;
					addVerticesToGroup(roads, *vi, threshold, shape, usedVertices, usedEdges);
					shapes.push_back(shape);
					break;
				}
			}
		}
	}
	end = clock();
	std::cout << "Close vertices detection: " << (double)(end - start) / CLOCKS_PER_SEC << " [sec]" << std::endl;

	

	return shapes;
}


void ShapeDetector::addVerticesToGroup(RoadGraph &roads, RoadVertexDesc srcDesc, float threshold, RoadEdgeDescs &shape, QMap<RoadVertexDesc, bool> &usedVertices, QMap<RoadEdgeDesc, bool> &usedEdges) {
	std::cout << "shape is detected..." << srcDesc << std::endl;

	float threshold2 = SQR(threshold);

	std::list<RoadVertexDesc> queue;
	queue.push_back(srcDesc);

	QMap<RoadVertexDesc, bool> vertex_descs;
	vertex_descs[srcDesc] = true;

	QMap<RoadEdgeDesc, bool> edge_descs;

	while (!queue.empty()) {
		RoadVertexDesc desc = queue.front();
		queue.pop_front();

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			//if (edge_descs.contains(*ei)) continue;
			if (usedEdges.contains(*ei)) continue;

			RoadVertexDesc tgt = boost::target(*ei, roads.graph);
			if (usedVertices.contains(tgt)) continue;

			if ((roads.graph[desc]->pt - roads.graph[tgt]->pt).lengthSquared() < threshold2) {
				edge_descs[*ei] = true;

				if (!vertex_descs.contains(tgt)) {				
					queue.push_back(tgt);
					vertex_descs[tgt] = true;
				}
			}
		}
	}

	for (QMap<RoadEdgeDesc, bool>::iterator it = edge_descs.begin(); it != edge_descs.end(); ++it) {
		shape.push_back(it.key());
	}
}
