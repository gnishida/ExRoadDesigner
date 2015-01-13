#include "../../common/Util.h"
#include "../GraphUtil.h"
#include "GenericRoadGenerator.h"

GenericRoadGenerator::GenericRoadGenerator() {
}

GenericRoadGenerator::~GenericRoadGenerator() {
}

void GenericRoadGenerator::generateRoadNetwork(RoadGraph &roads, const Polygon2D &area, const GenericFeature& gf) {
	std::list<RoadVertexDesc> seeds;
	std::list<RoadVertexDesc> newSeeds;

	generateInitialSeeds(roads, area, gf, seeds);

	int iteCount = 0;

	//======= grow arterials
	while (!seeds.empty() && iteCount < 1000) {
		RoadVertexDesc desc = seeds.front();
		seeds.pop_front();

		attemptExpansion(roads, area, desc, 2, gf, newSeeds);

		//append seeds in newSeeds to seeds
		seeds.splice(seeds.end(), newSeeds);

		iteCount++;
	}

	//Remove dead ends
	removeDeadEnds(roads);

	//======= grow streets
	if (!generateInitialStreetSeeds(roads, gf, seeds)) {
		return;
	}

	iteCount = 0;
	while (!seeds.empty() && iteCount < 1000) {
		RoadVertexDesc desc = seeds.front();
		seeds.pop_front();

		attemptExpansion(roads, area, desc, 1, gf, newSeeds);

		//append seeds in newSeeds to seeds
		seeds.splice(seeds.end(), newSeeds);

		iteCount++;
	}

	//Remove dead ends
	removeDeadEnds(roads);
}

/**
 * Make a center as an initial seed, and add it as a vertex of the road graph.
 *
 * @param urbanGeometry
 * @param seeds
 * @param inRoadGraph
 */
void GenericRoadGenerator::generateInitialSeeds(RoadGraph &roads, const Polygon2D &area, const GenericFeature& gf, std::list<RoadVertexDesc>& seeds) {
	seeds.clear();

	QVector2D center = area.centroid();

	RoadVertexPtr v = RoadVertexPtr(new RoadVertex(center));
	RoadVertexDesc desc = GraphUtil::addVertex(roads, v);
	roads.graph[desc]->angles = gf.getAngles(4);
	roads.graph[desc]->lengths = gf.getLengths(2, 4);

	seeds.push_back(desc);
}

/**
 * ローカルストリートのシードを生成する。
 */
/*
bool GenericRoadGenerator::generateInitialStreetSeeds(RoadGraph &roads, const GenericFeature& gf, std::list<RoadVertexDesc>& seeds) {
	std::vector<RoadEdgeDesc> edges;

	// Avenueのエッジをリストアップ
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		// Avenueエッジのみを対象
		if (roads.graph[*ei]->type == 2) {
			edges.push_back(*ei);
		}
	}

	// 各Avenueエッジ上に、ストリート生成用のシードを置いていく
	for (int ei = 0; ei < edges.size(); ++ei) {
		float step = gf.generateLength(1, Util::uniform_rand());
		float remained_step = step;

		for (int i = 0; i < roads.graph[edges[ei]]->polyLine.size() - 1; ++i) {
			QVector2D dir = roads.graph[edges[ei]]->polyLine[i + 1] - roads.graph[edges[ei]]->polyLine[i];
			float length = dir.length();
			float remained_length = length;
			while (remained_length > remained_step) {
				remained_length -= remained_step;
				remained_step = step;

				QVector2D pt = roads.graph[edges[ei]]->polyLine[i + 1] - dir * remained_length / length;
				RoadVertexDesc desc = GraphUtil::splitEdge(roads, edges[ei], pt);

				// 頂点の情報をセット
				roads.graph[desc]->angles = gf.getPerpendicularAngles(dir);
				roads.graph[desc]->lengths = gf.getLengths(1, 2);

				// シードに追加
				seeds.push_back(desc);
			}

			remained_step -= remained_length;
		}
	}

	return true;
}
*/
bool GenericRoadGenerator::generateInitialStreetSeeds(RoadGraph &roads, const GenericFeature& gf, std::list<RoadVertexDesc>& seeds) {
	std::vector<RoadEdgeDesc> edges;

	// degreeが2の頂点を、シードとする
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		if (GraphUtil::getNumEdges(roads, *vi) != 2) continue;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			// 頂点の情報をセット
			roads.graph[*vi]->angles = gf.getPerpendicularAngles(roads.graph[tgt]->pt - roads.graph[*vi]->pt);
			roads.graph[*vi]->lengths = gf.getLengths(1, 2);

			seeds.push_back(*vi);

			break;
		}
	}

	return true;
}

/**
* Attempts expansion of a segment in all possible directions and adds new edges to roadGraph.
* If new seeds are found, they are added to newSeeds.
* This function DOES modify the graph
**/
void GenericRoadGenerator::attemptExpansion(RoadGraph &roads, const Polygon2D &area, RoadVertexDesc &srcDesc, int roadType, const GenericFeature& gf, std::list<RoadVertexDesc> &newSeeds) {
	newSeeds.clear();

	for (int i = 0; i < roads.graph[srcDesc]->angles.size(); ++i) {
		float theta = roads.graph[srcDesc]->angles[i];
		QVector2D dir(cosf(theta), sinf(theta));

		bool snapped = false;
		bool intersected = false;
		bool outside = false;

		float dist = roads.graph[srcDesc]->lengths[i];
		dist = dist + Util::uniform_rand(dist * -0.3f, dist * 0.3f);

		RoadVertexDesc tgtDesc;
		QVector2D pt = roads.graph[srcDesc]->pt + dir * dist;

		// INTERSECTS -- If edge intersects other edge
		QVector2D intPoint;
		RoadEdgeDesc closestEdge;
		intersected = intersects(roads, roads.graph[srcDesc]->pt, pt, closestEdge, intPoint);
		if (intersected) {
			pt = intPoint;

			intersected = true;
		}

		float threshold;
		if (roadType == RoadEdge::TYPE_STREET) {
			threshold = (std::max)(0.25f * dist, 10.0f);
		} else {
			threshold = (std::max)(0.25f * dist, 40.0f);
		}

		// 近くに頂点があるか？
		RoadVertexDesc desc;
		RoadEdgeDesc e_desc;
		if (GraphUtil::getVertex(roads, pt, threshold, srcDesc, desc)) {
			tgtDesc = desc;
			snapped = true;
			intersected = false;
		} else if (GraphUtil::getEdge(roads, pt, threshold, e_desc)) {
			tgtDesc = GraphUtil::splitEdge(roads, e_desc, pt);
			snapped = true;
			intersected = false;
		} else {
			if (!area.contains(pt)) {
				// エリア外周との交点を求める
				area.intersects(roads.graph[srcDesc]->pt, pt, pt);
				outside = true;
			}

			if (intersected) {
				// 交差相手のエッジを分割
				tgtDesc = GraphUtil::splitEdge(roads, closestEdge, pt);
			} else {
				// 頂点を追加
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(pt));
				tgtDesc = GraphUtil::addVertex(roads, v);
			
				if (outside) {
					roads.graph[tgtDesc]->onBoundary = true;
				}

				int numDirections = gf.getDirections(roadType);
				roads.graph[tgtDesc]->angles = gf.getAngles(numDirections - 1, roads.graph[srcDesc]->pt - roads.graph[tgtDesc]->pt);
				roads.graph[tgtDesc]->lengths = gf.getLengths(roadType, numDirections - 1);
			}
		}

		// エッジを追加
		if (!GraphUtil::hasEdge(roads, srcDesc, tgtDesc)) {
			GraphUtil::addEdge(roads, srcDesc, tgtDesc, roadType, 1);
		}

		// シードに追加
		if (!snapped && !intersected && !outside) {
			newSeeds.push_back(tgtDesc);
		}
	}
}

/**
* Checks if new edge will intersect an existing edge
* This function DOES NOT modify the graph
**/
bool GenericRoadGenerator::intersects(RoadGraph &roads, const QVector2D& p0, const QVector2D& p1, RoadEdgeDesc &eiClosest, QVector2D &closestIntPt) {
	float min_dist = std::numeric_limits<float>::max();
	bool intersect = false;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		//if new segment intersects other segment
		QVector2D intPt;
		float tab, tcd;
		if (Util::segmentSegmentIntersectXY(p0, p1, roads.graph[src]->pt, roads.graph[tgt]->pt, &tab, &tcd, true, intPt)) {
			float dist = (p0 - intPt).lengthSquared();

			//make sure we get only closest segment
			if (dist < min_dist) {
				min_dist = dist;
				eiClosest = *ei;
				closestIntPt = intPt;
				intersect = true;
			}
		}
	}	

	return intersect;
}

/**
 * Dead-endのエッジを削除する。
 * ただし、境界にあるエッジは対象外とする。
 */
void GenericRoadGenerator::removeDeadEnds(RoadGraph &roads) {
	bool removedOne = true;
	while (removedOne) {
		removedOne = false;
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			if (roads.graph[*vi]->onBoundary) continue;

			if (GraphUtil::getDegree(roads, *vi) == 1) {
				// invalidate all the outing edges.
				RoadOutEdgeIter ei, eend;
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
					roads.graph[*ei]->valid = false;
				}

				// invalidate the vertex as well.
				roads.graph[*vi]->valid = false;

				removedOne = true;
			}
		}
	}
}