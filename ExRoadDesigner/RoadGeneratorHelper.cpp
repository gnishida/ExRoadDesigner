#include "RoadGeneratorHelper.h"
#include <limits>
#include "global.h"
#include "Util.h"
#include "ConvexHull.h"
#include "GraphUtil.h"
#include "TopNSearch.h"
#include "BSpline.h"
/**
* Checks if new edge will intersect an existing edge
**/
bool RoadGeneratorHelper::intersects(RoadGraph &roads, const QVector2D& p0, const QVector2D& p1, RoadEdgeDesc &eiClosest, QVector2D &closestIntPt) {
	float min_dist = std::numeric_limits<float>::max();
	bool intersected = false;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		// Highwayとはintersectしない
		if (roads.graph[*ei]->type == RoadEdge::TYPE_HIGHWAY) continue;

		for (int i = 0; i < roads.graph[*ei]->polyline.size() - 1; ++i) {
			//if new segment intersects other segment
			QVector2D intPt;
			float tab, tcd;
			if (Util::segmentSegmentIntersectXY(p0, p1, roads.graph[*ei]->polyline[i], roads.graph[*ei]->polyline[i + 1], &tab, &tcd, true, intPt)) {
				float dist = (p0 - intPt).lengthSquared();

				//make sure we get only closest segment
				if (dist < min_dist) {
					min_dist = dist;
					eiClosest = *ei;
					closestIntPt = intPt;
					intersected = true;
				}
			}

		}
	}	

	return intersected;
}

/**
 * polylineは、srcDescから始まるオーダになっている前提。
 */
bool RoadGeneratorHelper::intersects(RoadGraph &roads, RoadVertexDesc srcDesc, const Polyline2D &polyline, RoadEdgeDesc &eiClosest, QVector2D &closestIntPt) {
	for (int i = 0; i < polyline.size() - 1; ++i) {
		if (intersects(roads, polyline[i], polyline[i + 1], eiClosest, closestIntPt)) {
			return true;
		}
	}

	return false;
}

/**
 * 近くの頂点にsnapすべきか、チェックする。
 * （この関数は、最新！！　これ以外は、使ってないはず）
 * v_descに近い頂点を探す。
 * ただし、v_descとスナップ先とのなす角度が９０度以上であること。
 * また、スナップによる移動距離が閾値以下であるものの中で、最短のものを選ぶ。
 * 
 * @param pos				エッジ先端
 * @param threshold			距離の閾値
 * @param srcDesc			この頂点からエッジを延ばしている
 * @param edge				このエッジ
 * @param snapDesc			最も近い頂点
 * @return					もしsnapすべき頂点があれば、trueを返却する
 */
bool RoadGeneratorHelper::canConnectToVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc) {
	float min_dist = std::numeric_limits<float>::max();

	// 当該頂点から出るポリラインを取得
	bool flag = false;
	Polyline2D polyline;
	RoadVertexDesc prev_desc;
	RoadEdgeDesc e_desc;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (flag) {
				// 当該頂点から複数のエッジが出ているので、スナップさせない
				return false;
			}

			prev_desc = boost::target(*ei, roads.graph);
			e_desc = *ei;

			// 当該頂点から並ぶように、polylineを並べ替える
			if ((roads.graph[*ei]->polyline[0] - roads.graph[v_desc]->pt).lengthSquared() > (roads.graph[*ei]->polyline.last() - roads.graph[v_desc]->pt).lengthSquared()) {
				std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
			}
			polyline = roads.graph[*ei]->polyline;

			flag = true;
		}
	}

	if (!flag) {
		// エッジがないので、スナップさせない
		return false;
	}

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		// 自分自身にはスナップしない
		if (*vi == v_desc) continue;

		// 隣接頂点にもスナップしない
		if (*vi == prev_desc) continue;

		// prev_desc - v_desc - *vi のなす角が90度未満なら、スナップしない
		if (Util::diffAngle(roads.graph[*vi]->pt - roads.graph[v_desc]->pt, polyline.last() - polyline[0]) < M_PI * 0.5f) continue;

		float dist2 = (roads.graph[*vi]->pt - roads.graph[v_desc]->pt).lengthSquared();
		if (dist2 < min_dist) {
			// コネクトすることで、他エッジと交差するかチェック
			GraphUtil::movePolyline(roads, polyline, roads.graph[*vi]->pt, roads.graph[prev_desc]->pt);
			if (!GraphUtil::isIntersect(roads, polyline, e_desc)) {
				min_dist = dist2;
				snapDesc = *vi;
			}
		}
	}

	if (min_dist <= threshold * threshold) return true;
	else return false;
}

bool RoadGeneratorHelper::canConnectToFixVertex(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadVertexDesc& snapDesc) {
	float min_dist = std::numeric_limits<float>::max();

	// 当該頂点から出るポリラインを取得
	bool flag = false;
	Polyline2D polyline;
	RoadVertexDesc prev_desc;
	RoadEdgeDesc e_desc;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (flag) {
				// 当該頂点から複数のエッジが出ているので、スナップさせない
				return false;
			}

			prev_desc = boost::target(*ei, roads.graph);
			e_desc = *ei;

			// 当該頂点から並ぶように、polylineを並べ替える
			if ((roads.graph[*ei]->polyline[0] - roads.graph[v_desc]->pt).lengthSquared() > (roads.graph[*ei]->polyline.last() - roads.graph[v_desc]->pt).lengthSquared()) {
				std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
			}
			polyline = roads.graph[*ei]->polyline;

			flag = true;
		}
	}

	if (!flag) {
		// エッジがないので、スナップさせない
		return false;
	}

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		// fixed頂点のみ、対象とする
		if (!roads.graph[*vi]->fixed) continue;

		// 自分自身にはスナップしない
		if (*vi == v_desc) continue;

		// 隣接頂点にもスナップしない
		if (*vi == prev_desc) continue;

		// prev_desc - v_desc - *vi のなす角が90度未満なら、スナップしない
		if (Util::diffAngle(roads.graph[*vi]->pt - roads.graph[v_desc]->pt, polyline.last() - polyline[0]) < M_PI * 0.5f) continue;

		float dist2 = (roads.graph[*vi]->pt - roads.graph[v_desc]->pt).lengthSquared();
		if (dist2 < min_dist) {
			// コネクトすることで、他エッジと交差するかチェック
			GraphUtil::movePolyline(roads, polyline, roads.graph[*vi]->pt, roads.graph[prev_desc]->pt);
			if (!GraphUtil::isIntersect(roads, polyline, e_desc)) {
				min_dist = dist2;
				snapDesc = *vi;
			}
		}
	}

	if (min_dist <= threshold * threshold) return true;
	else return false;
}

/**
 * 近くのエッジにsnapすべきか、チェックする。
 * ただし、スナップによる変位角は90度未満で、接続された２つのエッジのなす角は45度より大きいこと。
 * また、スナップによる移動距離が閾値以下であるものの中で、最短のものを選ぶ。
 * 
 * @param pos				エッジ先端
 * @param threshold			距離の閾値
 * @param srcDesc			この頂点からエッジを延ばしている
 * @param snapDesc			最も近い頂点
 * @return					もしsnapすべき頂点があれば、trueを返却する
 */
bool RoadGeneratorHelper::canSnapToEdge(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadEdgeDesc& snapEdge, QVector2D &closestPt) {
	float min_dist = std::numeric_limits<float>::max();

	// 当該頂点から出るポリラインを取得
	bool flag = false;
	Polyline2D polyline;
	RoadVertexDesc prev_desc;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (flag) {
				// 当該頂点から複数のエッジが出ているので、スナップさせない
				return false;
			}

			prev_desc = boost::target(*ei, roads.graph);

			// 当該頂点から並ぶように、polylineを並べ替える
			if ((roads.graph[*ei]->polyline[0] - roads.graph[v_desc]->pt).lengthSquared() > (roads.graph[*ei]->polyline.last() - roads.graph[v_desc]->pt).lengthSquared()) {
				std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
			}
			polyline = roads.graph[*ei]->polyline;

			flag = true;
		}
	}

	if (!flag) {
		// エッジがないので、スナップさせない
		return false;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		if (src == v_desc || tgt == v_desc) continue;
		if (src == prev_desc || tgt == prev_desc) continue;

		QVector2D closePt;
		float dist = GraphUtil::distance(roads, roads.graph[v_desc]->pt, *ei, closePt);

		// 変位角が90度以上なら、スキップ
		float phi = Util::diffAngle(closePt - roads.graph[prev_desc]->pt, roads.graph[v_desc]->pt - roads.graph[prev_desc]->pt);
		if (phi >= M_PI * 0.5f) continue;

		// ２つのエッジのなす角が45度以下なら、スキップ
		float theta1 = Util::diffAngle(closePt - roads.graph[src]->pt, closePt - roads.graph[prev_desc]->pt);
		if (theta1 <= M_PI * 0.25f) continue;
		float theta2 = Util::diffAngle(closePt - roads.graph[tgt]->pt, closePt - roads.graph[prev_desc]->pt);
		if (theta2 <= M_PI * 0.25f) continue;

		if (dist < min_dist) {
			min_dist = dist;
			snapEdge = *ei;
			closestPt = closePt;
		}
	}		

	if (min_dist < threshold) return true;
	else return false;
}

bool RoadGeneratorHelper::canSnapToFixEdge(RoadGraph& roads, RoadVertexDesc v_desc, float threshold, RoadEdgeDesc& snapEdge, QVector2D &closestPt) {
	float min_dist = std::numeric_limits<float>::max();

	// 当該頂点から出るポリラインを取得
	bool flag = false;
	Polyline2D polyline;
	RoadVertexDesc prev_desc;
	{
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			if (flag) {
				// 当該頂点から複数のエッジが出ているので、スナップさせない
				return false;
			}

			prev_desc = boost::target(*ei, roads.graph);

			// 当該頂点から並ぶように、polylineを並べ替える
			if ((roads.graph[*ei]->polyline[0] - roads.graph[v_desc]->pt).lengthSquared() > (roads.graph[*ei]->polyline.last() - roads.graph[v_desc]->pt).lengthSquared()) {
				std::reverse(roads.graph[*ei]->polyline.begin(), roads.graph[*ei]->polyline.end());
			}
			polyline = roads.graph[*ei]->polyline;

			flag = true;
		}
	}

	if (!flag) {
		// エッジがないので、スナップさせない
		return false;
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		// fixed頂点のみを対象
		if (!roads.graph[src]->fixed && !roads.graph[tgt]->fixed) continue;

		if (src == v_desc || tgt == v_desc) continue;
		if (src == prev_desc || tgt == prev_desc) continue;

		QVector2D closePt;
		float dist = GraphUtil::distance(roads, roads.graph[v_desc]->pt, *ei, closePt);

		// 変位角が90度以上なら、スキップ
		float phi = Util::diffAngle(closePt - roads.graph[prev_desc]->pt, roads.graph[v_desc]->pt - roads.graph[prev_desc]->pt);
		if (phi >= M_PI * 0.5f) continue;

		// ２つのエッジのなす角が45度以下なら、スキップ
		float theta1 = Util::diffAngle(closePt - roads.graph[src]->pt, closePt - roads.graph[prev_desc]->pt);
		if (theta1 <= M_PI * 0.25f) continue;
		float theta2 = Util::diffAngle(closePt - roads.graph[tgt]->pt, closePt - roads.graph[prev_desc]->pt);
		if (theta2 <= M_PI * 0.25f) continue;

		if (dist < min_dist) {
			min_dist = dist;
			snapEdge = *ei;
			closestPt = closePt;
		}
	}		

	if (min_dist < threshold) return true;
	else return false;
}

/**
 * snap先の頂点を探す。ただし、deadendの頂点は対象外。また、水面下の頂点も対象外。
 * また、方向ベクトルがangle方向からしきい値を超えてる場合、その頂点はスキップする。
 * さらに、距離がdistance_threshold未満であること。
 *
 * @param vboRenderManager		標高を取得するため
 * @param roads					道路グラフ
 * @param srcDesc				snap元の頂点ID
 * @param distance_threshold	snap先との距離のしきい値（これより遠い頂点は、対象外）
 * @param z_threshold			snap先までの間の標高のしきい値（これより低い所を通り場合は、対象外）
 * @param angle					snap元頂点からの方向基準
 * @param angle_threshold		方向のしきい値（方向基準からこのしきい値を超える場合は、対象外）
 * @param nearest_desc [OUT]	snap先の頂点ID
 * @return						snap先が見つかった場合はtrueを返却する。
 */
bool RoadGeneratorHelper::getVertexForSnapping(VBORenderManager& vboRenderManager, RoadGraph& roads, RoadVertexDesc srcDesc, float distance_threshold, float z_threshold, float angle, float angle_threshold, RoadVertexDesc& nearest_desc) {
	float distance_threshold2 = distance_threshold * distance_threshold;
	float min_cost = std::numeric_limits<float>::max();
	bool found = false;;

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;
		if (*vi == srcDesc) continue;

		if (roads.graph[*vi]->deadend) continue;

		QVector2D vec = roads.graph[*vi]->pt - roads.graph[srcDesc]->pt;
		float angle2 = atan2f(vec.y(), vec.x());
		if (Util::diffAngle(angle, angle2) > angle_threshold) continue;

		float dist = vec.lengthSquared();
		if (dist > distance_threshold2) continue;

		// エッジが水面下かチェック
		float z = vboRenderManager.getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y());
		if (z < z_threshold) continue;

		// 適当なコスト関数で、最適な頂点を探す。
		// 基本的には、距離が近く、角度の差が小さいやつ。でも、係数はむずかしい。。。
		float cost = dist + Util::diffAngle(angle, angle2) * 1000.0;

		if (cost < min_cost) {
			min_cost = cost;
			nearest_desc = *vi;
			found = true;
		}
	}

	return found;
}

/**
 * snap先のエッジを探す。ただし、水面下のエッジは対象外。
 * また、方向ベクトルがangle方向からしきい値を超えてる場合、その頂点はスキップする。
 * さらに、距離がdistance_threshold未満であること。
 *
 * @param vboRenderManager		標高を取得するため
 * @param roads					道路グラフ
 * @param srcDesc				snap元の頂点ID
 * @param distance_threshold	snap先との距離のしきい値（これより遠い頂点は、対象外）
 * @param z_threshold			snap先までの間の標高のしきい値（これより低い所を通り場合は、対象外）
 * @param angle					snap元頂点からの方向基準
 * @param angle_threshold		方向のしきい値（方向基準からこのしきい値を超える場合は、対象外）
 * @param nearest_desc [OUT]	snap先の頂点ID
 * @return						snap先が見つかった場合はtrueを返却する。
 */
bool RoadGeneratorHelper::getEdgeForSnapping(VBORenderManager& vboRenderManager, RoadGraph& roads, RoadVertexDesc srcDesc, float distance_threshold, float z_threshold, float angle, float angle_threshold, RoadEdgeDesc& nearest_desc, QVector2D& nearestPt) {
	if (angle < 0) {
		angle += 3.14159265 * 2.0f;
	}

	float min_dist = distance_threshold;
	bool found = false;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		if (src == srcDesc || tgt == srcDesc) continue;

		// エッジが水面下かチェック
		float z1 = vboRenderManager.getTerrainHeight(roads.graph[src]->pt.x(), roads.graph[src]->pt.y());
		if (z1 < z_threshold) continue;
		float z2 = vboRenderManager.getTerrainHeight(roads.graph[tgt]->pt.x(), roads.graph[tgt]->pt.y());
		if (z2 < z_threshold) continue;

		QVector2D vec1 = roads.graph[src]->pt - roads.graph[srcDesc]->pt;
		QVector2D vec2 = roads.graph[tgt]->pt - roads.graph[srcDesc]->pt;
		float angle1 = atan2f(vec1.y(), vec1.x());
		float angle2 = atan2f(vec2.y(), vec2.x());

		if (Util::withinAngle(angle, angle1, angle2) || Util::diffAngle(angle, angle1) < angle_threshold || Util::diffAngle(angle, angle2) < angle_threshold) {
			QVector2D pt;
			float dist = GraphUtil::distance(roads, roads.graph[srcDesc]->pt, *ei, pt);
			if (dist < min_dist) {
				min_dist = dist;
				found = true;
				nearest_desc = *ei;
				nearestPt = pt;
			}
		}
	}

	return found;
}

/**
 * 指定された頂点について、指定されたエッジに似たエッジが既に登録済みかどうかチェックする。
 * polylineには、各点の、頂点からのオフセット座標が入る。
 * 登録済みのエッジに対しては、エッジの端点への方向ベクトルとpolylineの端点の方向ベクトルのなす角度が30度未満なら、trueを返却する。
 */
bool RoadGeneratorHelper::isRedundantEdge(RoadGraph& roads, RoadVertexDesc v_desc, const Polyline2D &polyline, float angleTolerance) {
	if (polyline.size() == 0) true;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = out_edges(v_desc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		if (roads.graph[*ei]->polyline.size() == 0) continue;

		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		if ((roads.graph[*ei]->polyline[0] - roads.graph[v_desc]->pt).lengthSquared() <= (roads.graph[*ei]->polyline.last() - roads.graph[v_desc]->pt).lengthSquared()) {
			if (Util::diffAngle(roads.graph[*ei]->polyline[1] - roads.graph[*ei]->polyline[0], polyline[1]) < angleTolerance) return true;
		} else {
			if (Util::diffAngle(roads.graph[*ei]->polyline.nextLast() - roads.graph[*ei]->polyline.last(), polyline[1]) < angleTolerance) return true;
		}

		/*
		if (Util::diffAngle(roads.graph[tgt]->pt - roads.graph[v_desc]->pt, polyline.last()) < angleTolerance) {
			return true;
		}
		*/
	}

	return false;
}

/**
 * Example領域を使って、指定された位置のモジュラ位置（Example座標系）を返却する。
 * モジュラ位置は、Example領域のBBoxの中に入る座標となる。
 * ※ 現状の実装では、Axis Aligned BBoxを使用し、Oriented BBoxなどをサポートしていない。
 *
 * @param targetArea	ターゲット領域
 * @param exampleArea	Example領域（Exampleの中心を原点とする座標系）
 * @param pt			指定された位置
 * @param bbox			指定された位置が入る、BBox（ターゲット座標系）
 * @return				指定された位置をmoduloした結果（Example座標系）
 */
QVector2D RoadGeneratorHelper::modulo(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt, BBox &bbox) {
	QVector2D ret;

	QVector2D center = targetArea.envelope().midPt();
	QVector2D offsetPt = pt - center;

	BBox bboxExample = exampleArea.envelope();

	if (offsetPt.x() < bboxExample.minPt.x()) {
		ret.setX(bboxExample.maxPt.x() - (int)(bboxExample.minPt.x() - offsetPt.x()) % (int)bboxExample.dx());

		int u = (bboxExample.minPt.x() - offsetPt.x()) / bboxExample.dx() + 1;
		bbox.minPt.setX(bboxExample.minPt.x() - bboxExample.dx() * u);
		bbox.maxPt.setX(bboxExample.maxPt.x() - bboxExample.dx() * u);
	} else if (offsetPt.x() > bboxExample.maxPt.x()) {
		ret.setX(bboxExample.minPt.x() + (int)(offsetPt.x() - bboxExample.maxPt.x()) % (int)bboxExample.dx());

		int u = (offsetPt.x() - bboxExample.maxPt.x()) / bboxExample.dx() + 1;
		bbox.minPt.setX(bboxExample.minPt.x() + bboxExample.dx() * u);
		bbox.maxPt.setX(bboxExample.maxPt.x() + bboxExample.dx() * u);
	} else {
		ret.setX(offsetPt.x());
		bbox.minPt.setX(bboxExample.minPt.x());
		bbox.maxPt.setX(bboxExample.maxPt.x());
	}

	if (offsetPt.y() < bboxExample.minPt.y()) {
		ret.setY(bboxExample.maxPt.y() - (int)(bboxExample.minPt.y() - offsetPt.y()) % (int)bboxExample.dy());

		int v = (bboxExample.minPt.y() - offsetPt.y()) / bboxExample.dy() + 1;
		bbox.minPt.setY(bboxExample.minPt.y() - bboxExample.dy() * v);
		bbox.maxPt.setY(bboxExample.maxPt.y() - bboxExample.dy() * v);
	} else if (offsetPt.y() > bboxExample.maxPt.y()) {
		ret.setY(bboxExample.minPt.y() + (int)(offsetPt.y() - bboxExample.maxPt.y()) % (int)bboxExample.dy());

		int v = (offsetPt.y() - bboxExample.maxPt.y()) / bboxExample.dy() + 1;
		bbox.minPt.setY(bboxExample.minPt.y() + bboxExample.dy() * v);
		bbox.maxPt.setY(bboxExample.maxPt.y() + bboxExample.dy() * v);
	} else {
		ret.setY(offsetPt.y());
		bbox.minPt.setY(bboxExample.minPt.y());
		bbox.maxPt.setY(bboxExample.maxPt.y());
	}

	bbox.minPt += center;
	bbox.maxPt += center;

	return ret;
}

/**
 * 指定された点を含むBBoxに、initial seedがあるかチェックする。
 */
bool RoadGeneratorHelper::containsInitialSeed(const Polygon2D &targetArea, const Polygon2D &exampleArea, const QVector2D &pt) {
	BBox targetBBox = targetArea.envelope();
	BBox exampleBBox = exampleArea.envelope();

	QVector2D center = targetBBox.midPt();

	if (pt.x() > center.x()) {
		int u = targetBBox.dx() / 2 - exampleBBox.dx() / 2;
		if (u > 0) {
			u = (int)(u / exampleBBox.dx()) + 1;
		} else {
			u = 0;
		}
		if (pt.x() - center.x() - exampleBBox.dx() / 2 > u * exampleBBox.dx()) return false;
	} else {
		int u = targetBBox.dx() / 2 - exampleBBox.dx() / 2;
		if (u > 0) {
			u = (int)(u / exampleBBox.dx()) + 1;
		} else {
			u = 0;
		}
		if (center.x() - pt.x() - exampleBBox.dx() / 2 > u * exampleBBox.dx()) return false;
	}

	if (pt.y() > center.y()) {
		int v = targetBBox.dy() / 2 - exampleBBox.dy() / 2;
		if (v > 0) {
			v = (int)(v / exampleBBox.dy()) + 1;
		} else {
			v = 0;
		}
		if (pt.y() - center.y() - exampleBBox.dy() / 2 > v * exampleBBox.dy()) return false;
	} else {
		int v = targetBBox.dy() / 2 - exampleBBox.dy() / 2;
		if (v > 0) {
			v = (int)(v / exampleBBox.dy()) + 1;
		} else {
			v = 0;
		}
		if (center.y() - pt.y() - exampleBBox.dy() / 2 > v * exampleBBox.dy()) return false;
	}

	return true;
}

void RoadGeneratorHelper::createFourDirection(float direction, std::vector<float> &directions) {
	float deltaDir;
	float tmpDir;
	
	deltaDir = 0.5f * M_PI;
	for (int i = 0; i < 4; ++i) {
		if (i==0) {
			tmpDir = direction;
		} else {
			tmpDir = tmpDir + deltaDir;// + ucore::Util::genRand(-departingAnglesNoise, departingAnglesNoise);
		}

		if (tmpDir > 2.0 * M_PI) {
			tmpDir = tmpDir - 2.0 * M_PI; 
		}

		directions.push_back(tmpDir);
	}
}

/**
 * Deadendの道路セグメントを削除する。
 * ただし、onBoundaryフラグがtrueの場合は、対象外。
 * また、deadendフラグがtrueの場合も、対象外。
 * 処理を繰り返し、deadend道路がなくなるまで、繰り返す。
 */
void RoadGeneratorHelper::removeDeadend(RoadGraph& roads) {
	bool removed = false;

	do {
		removed = false;

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadVertexDesc src = boost::source(*ei, roads.graph);
			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (roads.graph[src]->onBoundary || roads.graph[tgt]->onBoundary) continue;
			if (roads.graph[src]->properties.contains("deadend") && roads.graph[src]->properties["deadend"] == true) continue;
			if (roads.graph[tgt]->properties.contains("deadend") && roads.graph[tgt]->properties["deadend"] == true) continue;
			if (roads.graph[src]->fixed || roads.graph[tgt]->fixed) continue;

			if (GraphUtil::getDegree(roads, src) == 1) {
				removeEdge(roads, src, *ei);
				removed = true;
			} else if (GraphUtil::getDegree(roads, tgt) == 1) {
				removeEdge(roads, tgt, *ei);
				removed = true;
			}
		}
	} while (removed);
}

/**
 * 指定された頂点から伸びるエッジを削除する。
 * degree=2の頂点については、引き続き、その先のエッジも削除していく。
 *
 * @param roads			道路グラフ
 * @param srcDesc		この頂点から削除を開始する
 * @param start_e_desc	このエッジ方向に、削除を開始する
 */
void RoadGeneratorHelper::removeEdge(RoadGraph& roads, RoadVertexDesc srcDesc, RoadEdgeDesc start_e_desc) {
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

/**
 * Dangling Edgeを少し伸ばして、他の頂点にsnapさせる。
 * ただし、onBoundaryフラグがtrueの場合は、対象外。
 * また、他のエッジと交差したり、角度がredundantなら、snapしないで削除する。
 */
void RoadGeneratorHelper::extendDanglingEdges(RoadGraph &roads, float maxDistance) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		// もとから存在する頂点は、そのまま
		if (roads.graph[*vi]->fixed) continue;

		// 境界の外に伸びていくエッジは、そのまま
		//if (roads.graph[*vi]->onBoundary) continue;

		// Deadendじゃないエッジは、対象外
		if (GraphUtil::getDegree(roads, *vi) != 1) continue;

		// この頂点から出る唯一のエッジedge_descを取得
		RoadEdgeDesc edge_desc;
		{
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				edge_desc = *ei;
				break;
			}
		}

		// このエッジを100m伸ばして、他のエッジにぶつかる場合は、スナップさせる
		{
			RoadEdgeDesc closestEdge;
			QVector2D intPoint;
			RoadVertexDesc snapDesc;
			if (canConnectToFixVertex(roads, *vi, maxDistance, snapDesc)) {
				// 他の頂点にスナップ
				GraphUtil::addEdge(roads, *vi, snapDesc, roads.graph[edge_desc]->type, roads.graph[edge_desc]->lanes, roads.graph[edge_desc]->oneWay);

				continue;
			} else if (canSnapToFixEdge(roads, *vi, maxDistance, closestEdge, intPoint)) {
				// 他のエッジにスナップ
				RoadVertexDesc tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
				if (!roads.graph[tgtDesc]->properties.contains("generation_type")) {
					roads.graph[tgtDesc]->properties["generation_type"] = roads.graph[*vi]->properties["generation_type"];
					roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
					roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
					roads.graph[tgtDesc]->properties["parent"] = *vi;
				}

				GraphUtil::addEdge(roads, *vi, tgtDesc, roads.graph[edge_desc]->type, roads.graph[edge_desc]->lanes, roads.graph[edge_desc]->oneWay);

				continue;
			} else if (canConnectToVertex(roads, *vi, maxDistance, snapDesc)) {
				// 他の頂点にスナップ
				GraphUtil::addEdge(roads, *vi, snapDesc, roads.graph[edge_desc]->type, roads.graph[edge_desc]->lanes, roads.graph[edge_desc]->oneWay);

				continue;
			} else if (canSnapToEdge(roads, *vi, maxDistance, closestEdge, intPoint)) {
			//if (GraphUtil::getEdge(roads, roads.graph[*vi]->pt, 100.0f, *vi, closestEdge, intPoint)) {
				// 他のエッジにスナップ
				RoadVertexDesc tgtDesc = GraphUtil::splitEdge(roads, closestEdge, intPoint);
				if (!roads.graph[tgtDesc]->properties.contains("generation_type")) {
					roads.graph[tgtDesc]->properties["generation_type"] = roads.graph[*vi]->properties["generation_type"];
					roads.graph[tgtDesc]->properties["group_id"] = roads.graph[closestEdge]->properties["group_id"];
					roads.graph[tgtDesc]->properties["ex_id"] = roads.graph[closestEdge]->properties["ex_id"];
					roads.graph[tgtDesc]->properties["parent"] = *vi;
				}

				GraphUtil::addEdge(roads, *vi, tgtDesc, roads.graph[edge_desc]->type, roads.graph[edge_desc]->lanes, roads.graph[edge_desc]->oneWay);

				continue;
			}
		}

		// snap先を探す
		RoadVertexDesc snapDesc = GraphUtil::getVertex(roads, roads.graph[*vi]->pt, *vi);

		Polyline2D polyline;
		polyline.push_back(roads.graph[snapDesc]->pt);
		polyline.push_back(roads.graph[*vi]->pt);

		// スナップ先の頂点にとって、redundantなら、スナップせずにエッジを削除する
		if (GraphUtil::hasRedundantEdge(roads, snapDesc, polyline, 1.0f)) {
			roads.graph[edge_desc]->valid = false;
		} else {
			std::reverse(polyline.begin(), polyline.end());

			// スナップによって、他のエッジと交差するなら、スナップせずにエッジを削除する
			if (GraphUtil::isIntersect(roads, polyline)) {
				roads.graph[edge_desc]->valid = false;
			} else {
				// スナップさせる
				RoadEdgeDesc e = GraphUtil::addEdge(roads, *vi, snapDesc, roads.graph[edge_desc]->type, roads.graph[edge_desc]->lanes);

				// エッジを10m単位に分割する（Local Streetsのシード用に）
				roads.graph[e]->polyline = GraphUtil::finerEdge(roads, e, 10.0f);

				roads.graph[e]->properties["generation_type"] = "snapped";
				roads.graph[e]->properties["group_id"] = roads.graph[*vi]->properties["group_id"];
				roads.graph[e]->properties["ex_id"] = roads.graph[*vi]->properties["ex_id"];
				roads.graph[e]->properties.remove("example_desc");
			}
		}
	}
}

/**
 * Deadendの道路セグメントを、可能な限りつなぐ。
 * エリア間の道路網をつなぐ場合に使用する。
 * ただし、"fixed"フラグがtrueの頂点は、動かさない。
 */
void RoadGeneratorHelper::connectRoads(RoadGraph& roads, float distance_threshold, float angle_threshold) {
	// ConvexHullを計算する
	ConvexHull convexHull;
	Polygon2D hull;

	// 境界上の頂点、エッジの組をリストアップする
	QList<RoadVertexDesc> boundaryNodes;
	QMap<RoadVertexDesc, RoadEdgeDesc> boundaryEdges;
	QMap<RoadVertexDesc, RoadVertexDesc> boundaryNodesPair;
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);
		
		convexHull.addPoint(roads.graph[src]->pt);
		convexHull.addPoint(roads.graph[tgt]->pt);

		if (!roads.graph[src]->fixed && roads.graph[src]->onBoundary && GraphUtil::getDegree(roads, src) == 1) {
			if (!boundaryNodes.contains(src)) boundaryNodes.push_back(src);
			if (!boundaryEdges.contains(src)) boundaryEdges[src] = *ei;
			if (!boundaryNodesPair.contains(src)) boundaryNodesPair[src] = tgt;
		} else if (!roads.graph[tgt]->fixed && roads.graph[tgt]->onBoundary && GraphUtil::getDegree(roads, tgt) == 1) {
			if (!boundaryNodes.contains(tgt)) boundaryNodes.push_back(tgt);
			if (!boundaryEdges.contains(tgt)) boundaryEdges[tgt] = *ei;
			if (!boundaryNodesPair.contains(tgt)) boundaryNodesPair[tgt] = src;
		}
	}

	convexHull.convexHull(hull);

	// リストアップしたエッジを、それぞれ少しずつ伸ばしていき、他のエッジにぶつかったらストップする
	int numIterations = 50000;
	while (!boundaryNodes.empty() && numIterations >= 0) {
		RoadVertexDesc v_desc = boundaryNodes.front();
		boundaryNodes.pop_front();

		if (!roads.graph[v_desc]->valid) continue;

		RoadVertexDesc v2_desc = boundaryNodesPair[v_desc];
		RoadEdgeDesc e_desc = boundaryEdges[v_desc];

		QVector2D step;
		if ((roads.graph[v_desc]->pt - roads.graph[e_desc]->polyline[0]).lengthSquared() <= (roads.graph[v2_desc]->pt - roads.graph[e_desc]->polyline[0]).lengthSquared()) {
			step = roads.graph[e_desc]->polyline[0] - roads.graph[e_desc]->polyline[1];
		} else {
			step = roads.graph[e_desc]->polyline.last() - roads.graph[e_desc]->polyline.nextLast();
		}
		step = step.normalized() * 20.0f;

		if (growRoadOneStep(roads, v_desc, step)) {
			if (hull.contains(roads.graph[v_desc]->pt)) {
				boundaryNodes.push_back(v_desc);
			}
		}

		numIterations--;
	}
}

void RoadGeneratorHelper::connectRoads2(RoadGraph &roads, float distance_threshold) {
	float distance_threshold2 = SQR(distance_threshold);

	// classify the vertices into avenues and local streets
	GraphUtil::setVertexType(roads);

	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;
			if (!roads.graph[*vi]->onBoundary) continue;

			// エッジを取得
			bool edge_found = false;
			RoadEdgeDesc edge_desc;
			Polyline2D polyline;
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				edge_desc = *ei;
				polyline = GraphUtil::orderPolyLine(roads, *ei, *vi);
				edge_found = true;
				break;
			}

			if (!edge_found) continue;
		
			float min_dist = std::numeric_limits<float>::max();
			RoadVertexDesc snap_desc;

			// find the closest vertex to snap
			RoadVertexIter vi2, vend2;
			for (boost::tie(vi2, vend2) = boost::vertices(roads.graph); vi2 != vend2; ++vi2) {
				if (*vi2 == *vi) continue;
				if (!roads.graph[*vi2]->valid) continue;
				if (!roads.graph[*vi2]->fixed) continue;
				if (roads.graph[*vi2]->type < roads.graph[*vi]->type) continue;

				float dist = (roads.graph[*vi]->pt - roads.graph[*vi2]->pt).lengthSquared();
				if (dist < min_dist) {
					min_dist = dist;
					snap_desc = *vi2;
				}
			}

			if (min_dist < distance_threshold2) {
				GraphUtil::snapVertex(roads, *vi, snap_desc);
			}
		}
	}

	// remove the self intersection
	bool removed = true;
	while (removed) {
		removed = false;
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadEdgeIter ei2, eend2;
			for (boost::tie(ei2, eend2) = boost::edges(roads.graph); ei2 != eend2; ++ei2) {
				if (!roads.graph[*ei2]->valid) continue;

				if (GraphUtil::isIntersect(roads, roads.graph[*ei]->polyline, roads.graph[*ei2]->polyline)) {
					if (roads.graph[*ei]->type >= roads.graph[*ei2]->type) {
						roads.graph[*ei2]->valid = false;
					} else {
						roads.graph[*ei]->valid = false;
					}

					removed = true;
					break;
				}
			}

			if (removed) break;
		}
	}
}

/**
 * Try to connect the "onBoundary" vertices to the other "onBoundary" vertices.
 * If no "onBoundary" vertices is found close, try to connect the other vertices.
 * When the vertex snaps to the other, the change in the angle should be less than 90 degree.
 */
void RoadGeneratorHelper::connectRoads3(RoadGraph &roads, VBORenderManager* vboRenderManager, float avenueThreshold, float streetThreshold) {
	float avenueThreshold2 = SQR(avenueThreshold);
	float streetThreshold2 = SQR(streetThreshold);

	float maxRotation = G::getFloat("rotationForSteepSlope");

	GraphUtil::setVertexType(roads);

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;
		if (!roads.graph[*vi]->onBoundary) continue;

		if (GraphUtil::getDegree(roads, *vi) != 1) continue;

		// エッジを取得
		RoadEdgeDesc edge_desc;
		Polyline2D polyline;
		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			edge_desc = *ei;
			polyline = GraphUtil::orderPolyLine(roads, *ei, *vi);
			break;
		}

		float min_dist = std::numeric_limits<float>::max();
		RoadVertexDesc snap_desc;

		// find the closest vertex to snap
		RoadVertexIter vi2, vend2;
		for (boost::tie(vi2, vend2) = boost::vertices(roads.graph); vi2 != vend2; ++vi2) {
			if (*vi2 == *vi) continue;
			if (!roads.graph[*vi2]->valid) continue;
			if (roads.graph[*vi2]->type < roads.graph[*vi]->type) continue;

			float z = vboRenderManager->getTerrainHeight(roads.graph[*vi2]->pt.x(), roads.graph[*vi2]->pt.y());
			if (z < G::getFloat("seaLevelForStreet")) continue;

			// if the change in the angle is not less than 60 degree, skip this vertex.
			if (Util::diffAngle(polyline[0] - polyline[1], roads.graph[*vi2]->pt - roads.graph[*vi]->pt) >= M_PI * 0.3333f) continue;

			float dist = (roads.graph[*vi]->pt - roads.graph[*vi2]->pt).lengthSquared();
			if (dist < min_dist) {
				min_dist = dist;
				snap_desc = *vi2;
			}
		}

		if ((roads.graph[*ei]->type == RoadEdge::TYPE_AVENUE && min_dist < avenueThreshold2) || (roads.graph[*ei]->type == RoadEdge::TYPE_STREET && min_dist < streetThreshold2)) {
			//GraphUtil::snapVertex(roads, *vi, snap_desc);

			// Extend the polyline from *vi to snap_desc
			/*
			{
				QVector2D dir = polyline[0] - polyline[1];
				float angle = atan2f(dir.y(), dir.x());
				QVector2D pt = roads.graph[*vi]->pt;
				std::reverse(polyline.begin(), polyline.end());
				float step = 100.0f;
				float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y(), true);

				int count = 0;
				while ((roads.graph[snap_desc]->pt - pt).lengthSquared() > SQR(step)*4 && count < 10) {
					QVector2D dir1 = roads.graph[snap_desc]->pt - pt;
					float angle1 = atan2f(dir1.y(), dir1.x());

					float new_angle = (angle + angle1) * 0.5f;
					
					QVector2D new_pt = pt + QVector2D(cosf(new_angle), sinf(new_angle)) * step;
					float new_z = vboRenderManager->getTerrainHeight(new_pt.x(), new_pt.y(), true);

					if (fabs(new_z - z) > 7.0f) {
						QVector2D best_new_pt;
						float best_a;
						float min_diff = std::numeric_limits<float>::max();
						float best_z;
						for (float th = new_angle; th <= new_angle + maxRotation; th += 0.1f) {
							QVector2D new_pt2 = pt + QVector2D(cosf(th), sinf(th)) * step;
							float new_z2 = vboRenderManager->getTerrainHeight(new_pt.x(), new_pt.y(), true);
							float diff = fabs(new_z2 - z);
							if (diff < min_diff) {
								min_diff = diff;
								best_new_pt = new_pt2;
								best_a = th;
								best_z = new_z2;
							}
						}
						for (float th = new_angle; th >= new_angle - maxRotation; th -= 0.1f) {
							QVector2D new_pt2 = pt + QVector2D(cosf(th), sinf(th)) * step;
							float new_z2 = vboRenderManager->getTerrainHeight(new_pt.x(), new_pt.y(), true);
							float diff = fabs(new_z2 - z);
							if (diff < min_diff) {
								min_diff = diff;
								best_new_pt = new_pt2;
								best_a = th;
								best_z = new_z2;
							}
						}

						new_pt = best_new_pt;
						new_angle = best_a;
						new_z = best_z;
					}

					pt = new_pt;
					polyline.push_back(pt);
					z = new_z;

					angle = new_angle;

					count++;
				}

				if (count >= 10) continue;

				polyline.push_back(roads.graph[snap_desc]->pt);

				roads.graph[edge_desc]->polyline = polyline;
				roads.graph[*vi]->pt = roads.graph[snap_desc]->pt;
			}
			*/

			// Move the vetex *vi to snap_desc
			{
				QVector2D dir0 = polyline[0] - polyline[1];
				float angle0 = atan2f(dir0.y(), dir0.x());
				QVector2D dir1 = roads.graph[snap_desc]->pt - roads.graph[*vi]->pt;
				float angle1 = atan2f(dir1.y(), dir1.x());

				float length = (roads.graph[snap_desc]->pt - roads.graph[*vi]->pt).length();
				dir1 /= length;
					
				float angle = (angle0 + angle1) * 0.5f;
				std::reverse(polyline.begin(), polyline.end());
					
				QVector2D pt1 = polyline.back() + QVector2D(cosf(angle), sinf(angle)) * length * 0.3333333f;
				QVector2D pt2 = pt1 + QVector2D(cosf(angle1), sinf(angle1)) * length * 0.3333333f;
					
				polyline.push_back(pt1);
				polyline.push_back(pt2);
				polyline.push_back(roads.graph[snap_desc]->pt);

				roads.graph[edge_desc]->polyline = polyline;

				// Move the vertex
				roads.graph[*vi]->pt = roads.graph[snap_desc]->pt;
			}
		}
	}

	roads.setModified();
}

bool RoadGeneratorHelper::growRoadOneStep(RoadGraph& roads, RoadVertexDesc srcDesc, const QVector2D& step) {
	bool snapped = false;
	bool intersected = false;

	QVector2D pt = roads.graph[srcDesc]->pt + step;
	RoadEdgeDesc closestEdge;

	// INTERSECTS -- If edge intersects other edge
	QVector2D intPoint;
	intersected = RoadGeneratorHelper::intersects(roads, roads.graph[srcDesc]->pt, pt, closestEdge, intPoint);
	if (intersected) {
		pt = intPoint;
	}

	if (intersected) {
		RoadVertexDesc splitVertex = GraphUtil::splitEdge(roads, closestEdge, pt);
		GraphUtil::snapVertex(roads, srcDesc, splitVertex);

		// 交差相手のエッジが、成長中のエッジなら、その成長をストップする
		RoadVertexDesc src = boost::source(closestEdge, roads.graph);
		RoadVertexDesc tgt = boost::target(closestEdge, roads.graph);
		if (roads.graph[src]->onBoundary) {
			RoadEdgeDesc e = GraphUtil::getEdge(roads, src, splitVertex);
			roads.graph[e]->valid = false;
			roads.graph[src]->valid = false;
		} else if (roads.graph[tgt]->onBoundary) {
			RoadEdgeDesc e = GraphUtil::getEdge(roads, tgt, splitVertex);
			roads.graph[e]->valid = false;
			roads.graph[tgt]->valid = false;
		}

		return false;
	} else {
		GraphUtil::moveVertex(roads, srcDesc, pt);
		return true;
	}	
}

/**
 * 海岸の手間で、エッジをカットする
 * polylineのlast()側が、カットされる側にあることを前提とする
 */
void RoadGeneratorHelper::cutEdgeByWater(Polyline2D &polyline, VBORenderManager& vboRenderManager, float z_threshold) {
	// 点が2つ未満の場合は、処理不能
	if (polyline.size() < 2) return;

	// polylineの中で、海岸をまたぐセグメントを探す
	for (int i = 0; i < polyline.size(); ++i) {
		float z = vboRenderManager.getTerrainHeight(polyline[i].x(), polyline[i].y());
		if (z < z_threshold) {
			// またいだ後のセグメントを全て削除
			for (int j = i + 1; j < polyline.size(); ) {
				polyline.erase(polyline.begin() + j);
			}
		}
	}

	// 海岸ぎりぎりの場所を探す
	QVector2D vec = polyline.last() - polyline.nextLast();
	vec.normalize();
	QVector2D pt = polyline.last();
	while (true) {
		pt -= vec;
		float z = vboRenderManager.getTerrainHeight(pt.x(), pt.y());
		if (z < z_threshold) {
			continue;
		} else {
			break;
		}
	}

	polyline[polyline.size() - 1] = pt;
}

/**
 * 急激な標高の変化がある場合に、エッジをその境界に沿うように曲げる
 * polylineのlast()が、境界の外にあることを前提とする
 * This function is only for the partially submerged road segment.
 */
void RoadGeneratorHelper::bendEdgeBySteepElevationChange(int roadType, Polyline2D &polyline, VBORenderManager *vboRenderManager) {
	// 点が2つ未満の場合は、処理不能
	if (polyline.size() < 2) return;

	// bend the last segment parallel to the coast line
	{
		QVector2D vec = polyline.last() - polyline.nextLast();
		float length = vec.length();
		float th0 = atan2f(vec.y(), vec.x());
		for (int i = 0; i < 32; ++i) {
			// try the left turn
			{
				float th = th0 - (float)i * 0.1f;
				QVector2D pt = polyline.nextLast() + QVector2D(cosf(th), sinf(th)) * length;
				float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
				//if (z >= G::getFloat("seaLevel")) {
				if ((roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForAvenue")) || (roadType == RoadEdge::TYPE_STREET && z < G::getFloat("seaLevelForStreet"))) {
				} else {
					polyline[polyline.size() - 1] = pt;
					return;
				}
			}

			// try the right turn
			{
				float th = th0 + (float)i * 0.1f;
				QVector2D pt = polyline.nextLast() + QVector2D(cosf(th), sinf(th)) * length;
				float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
				//if (z >= G::getFloat("seaLevel")) {
				if ((roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForAvenue")) || (roadType == RoadEdge::TYPE_STREET && z < G::getFloat("seaLevelForStreet"))) {
				} else {
					polyline[polyline.size() - 1] = pt;
					return;
				}
			}
		}
	}

	return;
}

/**
 * Find the least slope direction, and bend the road segment to that direction.
 */
void RoadGeneratorHelper::bendEdgeBySteepElevationChange(Polyline2D &polyline, float z0, VBORenderManager *vboRenderManager) {
	if (polyline.size() < 2) return;

	float min_dz = std::numeric_limits<float>::max();
	QVector2D min_pt;

	float max_rotation = G::getFloat("rotationForSteepSlope");

	// bend the last segment until the slope is not too steep
	{
		QVector2D vec = polyline.last() - polyline.nextLast();
		float length = vec.length();
		float th0 = atan2f(vec.y(), vec.x());

		// try the left turn
		for (float th = th0; th <= th0 + max_rotation; th += 0.1f) {
			QVector2D pt = polyline.nextLast() + QVector2D(cosf(th), sinf(th)) * length;
			float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
			if (fabs(z - z0) < min_dz) {
				min_dz = fabs(z - z0);
				min_pt = pt;
			}
		}

		// try the right turn
		for (float th = th0; th >= th0 - max_rotation; th -= 0.1f) {
			QVector2D pt = polyline.nextLast() + QVector2D(cosf(th), sinf(th)) * length;
			float z = vboRenderManager->getTerrainHeight(pt.x(), pt.y());
			if (fabs(z - z0) < min_dz) {
				min_dz = fabs(z - z0);
				min_pt = pt;
			}
		}
	}

	polyline.back() = min_pt;
	
	return;
}

/**
 * Return true if at least one polyline point or vertex is under the sea level.
 */
bool RoadGeneratorHelper::submerged(int roadType, RoadGraph &roads, VBORenderManager *vboRenderManager) {
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		if (submerged(roadType, roads.graph[*ei]->polyline, vboRenderManager)) return true;
	}

	return false;
}

bool RoadGeneratorHelper::submerged(int roadType, const Polyline2D &polyline, VBORenderManager *vboRenderManager) {
	for (int i = 0; i < polyline.size(); ++i) {
		float z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y());
		if ((roadType == RoadEdge::TYPE_AVENUE && z < G::getFloat("seaLevelForAvenue")) || (roadType == RoadEdge::TYPE_STREET && z < G::getFloat("seaLevelForStreet"))) return true;		
	}

	return false;
}

/*
bool RoadGeneratorHelper::steepSlope(RoadGraph &roads, VBORenderManager *vboRenderManager) {
	float min_z = std::numeric_limits<float>::max();
	float max_z = -std::numeric_limits<float>::max();

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		float z = vboRenderManager->getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), true);
		if (z < min_z) min_z = z;
		if (z > max_z) max_z = z;
	}

	if (max_z - min_z > 7.0f) return true;
	else return false;
}
*/

float RoadGeneratorHelper::maxZ(RoadGraph &roads, VBORenderManager *vboRenderManager) {
	float max_z = -std::numeric_limits<float>::max();

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 5.0f);

		for (int i = 0; i < polyline.size(); ++i) {
			float z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y());
			max_z = std::max(max_z, z);
		}
	}

	return max_z;
}

/**
 * 道路網がZ座標の最小値を返却する。
 * checkConnectors=trueなら、connector=trueのエッジも対象。falseなら、対象外とする。
 */
float RoadGeneratorHelper::minZ(RoadGraph &roads, VBORenderManager *vboRenderManager, bool checkConnectors) {
	float min_z = std::numeric_limits<float>::max();

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;
		if (!checkConnectors && roads.graph[*ei]->connector) continue;

		Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 5.0f);

		for (int i = 0; i < polyline.size(); ++i) {
			float z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y());
			min_z = std::min(min_z, z);
		}
	}

	return min_z;
}

float RoadGeneratorHelper::diffZ(RoadGraph &roads, VBORenderManager *vboRenderManager) {
	float min_z = std::numeric_limits<float>::max();
	float max_z = -std::numeric_limits<float>::max();

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 5.0f);

		for (int i = 0; i < polyline.size(); ++i) {
			float z = vboRenderManager->getTerrainHeight(polyline[i].x(), polyline[i].y());
			min_z = std::min(min_z, z);
			max_z = std::max(max_z, z);
		}
	}

	return max_z - min_z;
}

float RoadGeneratorHelper::diffSlope(RoadGraph &roads, VBORenderManager *vboRenderManager) {
	float min_slope = std::numeric_limits<float>::max();
	float max_slope = -std::numeric_limits<float>::max();

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		for (int i = 0; i < roads.graph[*ei]->polyline.size() - 1; ++i) {
			float z1 = vboRenderManager->getTerrainHeight(roads.graph[*ei]->polyline[i].x(), roads.graph[*ei]->polyline[i].y());
			float z2 = vboRenderManager->getTerrainHeight(roads.graph[*ei]->polyline[i + 1].x(), roads.graph[*ei]->polyline[i + 1].y());
			float len = (roads.graph[*ei]->polyline[i + 1] - roads.graph[*ei]->polyline[i]).length();
			float slope = atan2f(z2 - z1, len);
			if (slope < min_slope) min_slope = slope;
			if (slope > max_slope) max_slope = slope;
		}
	}
	
	return max_slope - min_slope;
}

/**
 * 指定した頂点から出るエッジについて、間の角度の最大値 [rad]を返却する。
 */
float RoadGeneratorHelper::largestAngleBetweenEdges(RoadGraph& roads, RoadVertexDesc srcDesc, int roadType) {
	std::vector<float> angles;

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;
		if (roads.graph[*ei]->type != roadType) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		angles.push_back(atan2f((polyline[1] - polyline[0]).y(), (polyline[1] - polyline[0]).x()));
	}

	if (angles.size() <= 1) return M_PI * 2.0f;

	std::sort(angles.begin(), angles.end());

	float max_diff = 0;
	for (int i = 0; i < angles.size() - 1; ++i) {
		float diff = angles[i + 1] - angles[i];
		if (diff > max_diff) max_diff = diff;
	}
	float diff = angles[0] + M_PI * 2 - angles.back();
	if (diff > max_diff) max_diff = diff;

	return max_diff;
}

float RoadGeneratorHelper::getFirstEdgeAngle(RoadGraph& roads, RoadVertexDesc srcDesc) {
	QVector2D dir(1, 0);

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		dir = polyline[1] - polyline[0];
		break;
	}

	return atan2f(dir.y(), dir.x());
}

QVector2D RoadGeneratorHelper::getFirstEdgeDir(RoadGraph& roads, RoadVertexDesc srcDesc) {
	QVector2D dir(1, 0);

	RoadOutEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::out_edges(srcDesc, roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, srcDesc);
		dir = polyline[1] - polyline[0];
		break;
	}

	return dir;
}

void RoadGeneratorHelper::removeIntersectionsOnRiver(RoadGraph &roads, VBORenderManager *vboRenderManager, float seaLevel) {
	GraphUtil::reduce(roads);

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		float z = vboRenderManager->getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y());
		if (z > seaLevel) continue;

		int degree = GraphUtil::getDegree(roads, *vi);
		if (degree == 2) continue;
		
		if (degree == 1) {
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				roads.graph[*ei]->valid = false;
				break;
			}
		}

		if (degree == 3 || degree == 4) {
			QMap<float, std::pair<RoadEdgeDesc, RoadEdgeDesc> > degrees;
			RoadEdgeDesc e1, e2;
			//bool found = false;

			// remain the straight line
			RoadOutEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, *vi);

				RoadOutEdgeIter ei2, eend2;
				for (boost::tie(ei2, eend2) = boost::out_edges(*vi, roads.graph); ei2 != ei; ++ei2) {					
				}

				++ei2;

				for (; ei2 != eend2; ++ei2) {
					if (!roads.graph[*ei2]->valid) continue;

					Polyline2D polyline2 = GraphUtil::orderPolyLine(roads, *ei2, *vi);

					float angle = Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]);
					degrees[angle] = std::make_pair(*ei, *ei2);
					/*
					if (Util::diffAngle(polyline[1] - polyline[0], polyline2[1] - polyline2[0]) > M_PI * 0.7f) {
						e1 = *ei;
						e2 = *ei2;
						found = true;
						break;
					}*/
				}
			}

			for (QMap<float, std::pair<RoadEdgeDesc, RoadEdgeDesc> >::iterator it = degrees.begin(); it != degrees.end(); ++it) {//(degrees);//::iterator it = degrees.();
				e1 = it.value().first;
				e2 = it.value().second;
			}

			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;

				if (*ei != e1 && *ei != e2) {
					roads.graph[*ei]->valid = false;
				}
			}

			/*
			if (found) {
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend && !found; ++ei) {
					if (!roads.graph[*ei]->valid) continue;

					if (*ei != e1 && *ei != e2) {
						roads.graph[*ei]->valid = false;
					}
				}
			} else {
				for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend && !found; ++ei) {
					if (!roads.graph[*ei]->valid) continue;
					roads.graph[*ei]->valid = false;
					break;
				}
			}
			*/
		}

		if (degree > 3) {
			RoadOutEdgeIter ei, eend;
			int cnt = 0;
			for (boost::tie(ei, eend) = boost::out_edges(*vi, roads.graph); ei != eend; ++ei, ++cnt) {
				if (!roads.graph[*ei]->valid) continue;

				RoadVertexDesc tgt =boost::target(*ei, roads.graph);

				if (cnt >= 2) {
					roads.graph[*ei]->valid = false;
				}
			}
		}
	}
}

/**
 * 小さすぎるブロックを削除する。
 */
void RoadGeneratorHelper::removeSmallBlocks(RoadGraph &roads, float minArea) {

}

/**
 * 指定された頂点に近くで、同じgroup_idで、example_descをプロパティに設定している頂点を探す。
 */
RoadVertexDesc RoadGeneratorHelper::getClosestVertexByExample(RoadGraph &roads, RoadVertexDesc v_desc) {
	float min_dist = std::numeric_limits<float>::max();
	RoadVertexDesc nearest_v_desc;

	int group_id = roads.graph[v_desc]->properties["group_id"].toInt();
	
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		if (roads.graph[*vi]->properties["group_id"].toInt() == group_id && roads.graph[*vi]->properties.contains("example_desc")) {
			float dist = (roads.graph[*vi]->pt - roads.graph[v_desc]->pt).lengthSquared();
			if (dist < min_dist) {
				min_dist = dist;
				nearest_v_desc = *vi;
			}
		}
	}

	return nearest_v_desc;
}

/**
 * Create one edge according to the speicified parameters.
 *
 * @param	roadType		the type of the road segment (RoadEdge::TYPE_AVENUE or RoadEdge::TYPE_STREET)
 * @param	length			the length of the road segment
 * @param	curvature		the curvature of the road segment
 * @param	angle			the initial angle of the road segment
 * @param	lanes			the number of lanes
 * @param	step_size		step size
 * @param	edge			output edge
 */
Polyline2D RoadGeneratorHelper::createOneEdge(int roadType, float length, float curvature, float angle, float step_size) {
	Polyline2D polyline;

	QVector2D cur(0, 0);
	int sign = 1;

	// 50%の確率で、どっちに曲がるか決定
	/*if (Util::genRand() >= 0.5f) {
		curvature = -curvature;
	}*/

	polyline.push_back(cur);

	for (int j = 0; j < 10000 && polyline.length() < length; ++j) {
		// Advance the current point to the next position
		cur.setX(cur.x() + cos(angle) * step_size);
		cur.setY(cur.y() + sin(angle) * step_size);

		polyline.push_back(cur);

		// 10%の確率で、曲がる方向を変える
		//if (Util::genRand() >= 0.9f) sign = -sign;

		float theta = atan2f(curvature * step_size, 1.0f);
		angle = angle + theta * (float)sign * 0.5f;
	}

	return polyline;
}

void RoadGeneratorHelper::createFourEdges(ExFeature &f, int roadType, const QVector2D &ex_pt, int lanes, float direction, float step, std::vector<RoadEdgePtr> &edges) {
	edges.clear();

	// 指定された example空間座標に基づき、エッジ長と曲率の平均、分散を計算する
	BBox bbox = f.area.envelope();
	float dist = (bbox.dx() + bbox.dy()) * 0.25f;
	//float avgLength, varLength, avgCurvature, varCurvature;
	//GraphUtil::computeStatistics(f.roads(roadType), ex_pt, dist, avgLength, varLength, avgCurvature, varCurvature);

	std::vector<float> directions;
	createFourDirection(direction, directions);

	int sign = 1;
	
	for (int i = 0; i < directions.size(); ++i) {
		float length, curvature;
		chooseEdgeLengthAndCurvature(f.roads(roadType), ex_pt, dist, directions[i], length, curvature);

		// 50%の確率で、どっちに曲がるか決定
		if (Util::genRand() >= 0.5f) {
			curvature = -curvature;
		}

		//float deltaDir = 0.0f;

		RoadEdgePtr edge = RoadEdgePtr(new RoadEdge(roadType, lanes));
		edge->polyline = createOneEdge(roadType, length, curvature, directions[i], step);
		/*
		QVector2D cur(0, 0);
		edge->polyline.push_back(cur);

		for (int j = 0; j < 10000 && edge->polyline.length() < length; ++j) {
			// Advance the current point to the next position
			cur.setX(cur.x() + cos(directions[i]) * step);
			cur.setY(cur.y() + sin(directions[i]) * step);

			edge->polyline.push_back(cur);

			// 10%の確率で、曲がる方向を変える
			if (Util::genRand() >= 0.9f) sign = -sign;

			float theta = atan2f(curvature * step, 1.0f);
			directions[i] = directions[i] + theta * (float)sign * 0.5f;
		}
		*/

		edges.push_back(edge);
	}
}

/**
 * Example道路から、指定された方向に似たエッジの中から、ランダムにエッジを選択し、その長さ、曲率を取得する。
 * ただし、interesting shapeの一部に含まれるエッジは、対象外とする。これにより、短すぎるエッジなどが含まれないようになる。
 */
void RoadGeneratorHelper::chooseEdgeLengthAndCurvature(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, float &length, float &curvature, int &lanes) {
	TopNSearch<RoadEdgeDesc> tns;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		// don't use the isolated edge
		if (GraphUtil::getDegree(roads, src) == 1 && GraphUtil::getDegree(roads, tgt) == 1) continue;

		// don't use the edge which is going outside the area
		if (GraphUtil::getDegree(roads, src) == 1 || GraphUtil::getDegree(roads, tgt) == 1) continue;

		// don't use the interesting shapes
		int shape_index;
		if (isShape(roads, src, shapes, shape_index) && isShape(roads, tgt, shapes, shape_index)) continue;

		QVector2D vec = roads.graph[src]->pt - roads.graph[tgt]->pt;
		float dir = atan2f(vec.y(), vec.x());

		float diff_angle = Util::diffAngle(angle, dir);

		tns.add(diff_angle, *ei);
	}

	QList<RoadEdgeDesc> e_descs = tns.topN(5, TopNSearch<RoadEdgeDesc>::ORDER_ASC);
	int index = Util::genRand(0, e_descs.size());

	length = roads.graph[e_descs[index]]->getLength();
	curvature = Util::curvature(roads.graph[e_descs[index]]->polyline);
	lanes = roads.graph[e_descs[index]]->lanes;
}

/**
 * Example道路から、指定された座標周辺で、指定された方向に似たエッジの中から、ランダムにエッジを選択し、その長さ、曲率を取得する。
 */
void RoadGeneratorHelper::chooseEdgeLengthAndCurvature(RoadGraph &roads, const QVector2D &ex_pt, float distance, float direction, float &length, float &curvature) {
	float dist2 = SQR(distance);

	float totalLength = 0.0f;
	float totalLength2 = 0.0f;
	float totalCurvature = 0.0f;
	float totalCurvature2 = 0.0f;
	int num = 0;

	std::vector<RoadEdgeDesc> edge_descs;
	std::vector<RoadEdgeDesc> edge_descs2;
	std::vector<RoadEdgeDesc> edge_descs3;
	std::vector<RoadEdgeDesc> edge_descs4;

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		RoadVertexDesc src = boost::source(*ei, roads.graph);
		RoadVertexDesc tgt = boost::target(*ei, roads.graph);

		edge_descs4.push_back(*ei);

		if ((roads.graph[src]->pt - ex_pt).lengthSquared() > dist2 && (roads.graph[tgt]->pt - ex_pt).lengthSquared() > dist2) continue;

		edge_descs3.push_back(*ei);

		QVector2D vec = roads.graph[src]->pt - roads.graph[tgt]->pt;
		float dir = atan2f(vec.y(), vec.x());

		float angle = Util::diffAngle(direction, dir);

		// 方向のずれが３０度以内なら、リストに追加
		if (angle <= M_PI * 30.0f / 180.0f && angle <= M_PI * 150.0f / 180.0f) edge_descs.push_back(*ei);

		// 方向のずれが６０度以内なら、リスト２に追加
		if (angle <= M_PI * 60.0f / 180.0f && angle <= M_PI * 120.0f / 180.0f) edge_descs2.push_back(*ei);
	}

	if (edge_descs.size() > 0) {
		int index = Util::genRand(0, edge_descs.size());
		if (index >= edge_descs.size()) index = edge_descs.size() - 1;

		length = roads.graph[edge_descs[index]]->polyline.length();
		curvature = Util::curvature(roads.graph[edge_descs[index]]->polyline);
	} else if (edge_descs2.size() > 0) {
		int index = Util::genRand(0, edge_descs2.size());
		if (index >= edge_descs2.size()) index = edge_descs2.size() - 1;

		length = roads.graph[edge_descs2[index]]->polyline.length();
		curvature = Util::curvature(roads.graph[edge_descs2[index]]->polyline);
	} else if (edge_descs3.size() > 0) {
		int index = Util::genRand(0, edge_descs3.size());
		if (index >= edge_descs3.size()) index = edge_descs3.size() - 1;

		length = roads.graph[edge_descs3[index]]->polyline.length();
		curvature = Util::curvature(roads.graph[edge_descs3[index]]->polyline);
	} else {
		int index = Util::genRand(0, edge_descs4.size());
		if (index >= edge_descs4.size()) index = edge_descs4.size() - 1;

		length = roads.graph[edge_descs4[index]]->polyline.length();
		curvature = Util::curvature(roads.graph[edge_descs4[index]]->polyline);
	}
}

/**
 * 位置ptが、bboxの中の上下左右、どの方向かを返却する。
 *
 * @return				0: 右 / 1: 上 / 2: 左 / 3: 下
 */
int RoadGeneratorHelper::getRelativeDirectionInArea(const BBox &bbox, const QVector2D &pt) {
	float dx = (pt.x() - bbox.midPt().x()) / bbox.dx();
	float dy = (pt.y() - bbox.midPt().y()) / bbox.dy();

	if (fabs(dx) > fabs(dy)) {
		if (dx >= 0) return 0;
		else return 2;
	} else {
		if (dy >= 0) return 1;
		else return 3;
	}
}

/**
 * エリアをfactor倍した領域に入っているかチェックする
 */
bool RoadGeneratorHelper::isWithinScaledArea(const Polygon2D &area, float factor, const QVector2D &pt) {
	QVector2D center = area.envelope().midPt();

	Polygon2D scaledArea;
	for (int i = 0; i < area.size(); ++i) {
		scaledArea.push_back((area[i] - center) * factor + center);
	}

	return scaledArea.contains(pt);
}

/**
 * 当該頂点が、interesting shapeの一部かどうか？
 */
bool RoadGeneratorHelper::isShape(RoadGraph &roads, RoadVertexDesc desc, std::vector<RoadEdgeDescs> &shapes, int &shape_index) {
	for (int i = 0; i < shapes.size(); ++i) {
		for (int j = 0; j < shapes[i].size(); ++j) {
			RoadVertexDesc src = boost::source(shapes[i][j], roads.graph);
			RoadVertexDesc tgt = boost::target(shapes[i][j], roads.graph);
			if (desc == src || desc == tgt) {
				shape_index = i;
				return true;
			}
		}
	}

	return false;
}

// パッチオブジェクトに変換する
std::vector<Patch> RoadGeneratorHelper::convertToPatch(int roadType, RoadGraph& roads, RoadGraph& avenues, std::vector<RoadEdgeDescs> &shapes) {
	std::vector<Patch> patches;
	for (int i = 0; i < shapes.size(); ++i) {
		Patch patch;

		QMap<RoadVertexDesc, RoadVertexDesc> conv;
		for (int j = 0; j < shapes[i].size(); ++j) {
			RoadVertexDesc src = boost::source(shapes[i][j], roads.graph);
			RoadVertexDesc tgt = boost::target(shapes[i][j], roads.graph);

			if (!conv.contains(src)) {
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*roads.graph[src]));
				RoadVertexDesc v_desc = GraphUtil::addVertex(patch.roads, v);
				if (roadType == RoadEdge::TYPE_AVENUE) {
					patch.roads.graph[v_desc]->properties["example_desc"] = src;
				} else {
					//patch.roads.graph[v_desc]->properties["example_street_desc"] = src;

					// どうやら、local streetの場合も、example_descで良いと思う。
					patch.roads.graph[v_desc]->properties["example_desc"] = src;
				}
				if (GraphUtil::getDegree(roads, src) == 1 && !roads.graph[src]->onBoundary) {
					if (roadType == RoadEdge::TYPE_AVENUE) {
						patch.roads.graph[v_desc]->deadend = true;
					} else {
						// local streetの場合、avenueとの交点はdeadendと見なさない
						RoadVertexDesc nearest_desc;
						if (!GraphUtil::getVertex(avenues, roads.graph[src]->pt, 0.1f, nearest_desc)) {
							patch.roads.graph[v_desc]->deadend = true;
						}
					}
				}
				patch.roads.graph[v_desc]->onBoundary = roads.graph[tgt]->onBoundary;
				patch.roads.graph[v_desc]->patchId = i;
				patch.roads.graph[v_desc]->type = roadType;
				patch.roads.graph[v_desc]->properties["orig_degree"] = GraphUtil::getDegree(roads, src);
				conv[src] = v_desc;
			}
			if (!conv.contains(tgt)) {
				RoadVertexPtr v = RoadVertexPtr(new RoadVertex(*roads.graph[tgt]));
				RoadVertexDesc v_desc = GraphUtil::addVertex(patch.roads, v);
				if (roadType == RoadEdge::TYPE_AVENUE) {
					patch.roads.graph[v_desc]->properties["example_desc"] = tgt;
				} else {
					//patch.roads.graph[v_desc]->properties["example_street_desc"] = tgt;

					// どうやら、local streetの場合も、example_descで良いと思う。
					patch.roads.graph[v_desc]->properties["example_desc"] = tgt;
				}
				if (GraphUtil::getDegree(roads, tgt) == 1 && !roads.graph[tgt]->onBoundary) {
					if (roadType == RoadEdge::TYPE_AVENUE) {
						patch.roads.graph[v_desc]->deadend = true;
					} else {
						// local streetの場合、avenueとの交点はdeadendと見なさない
						RoadVertexDesc nearest_desc;
						if (!GraphUtil::getVertex(avenues, roads.graph[tgt]->pt, 0.1f, nearest_desc)) {
							patch.roads.graph[v_desc]->deadend = true;
						}
					}
				}
				patch.roads.graph[v_desc]->onBoundary = roads.graph[tgt]->onBoundary;
				patch.roads.graph[v_desc]->patchId = i;
				patch.roads.graph[v_desc]->type = roadType;
				patch.roads.graph[v_desc]->properties["orig_degree"] = GraphUtil::getDegree(roads, tgt);
				conv[tgt] = v_desc;
			}

			RoadEdgePtr e = RoadEdgePtr(new RoadEdge(*roads.graph[shapes[i][j]]));
			GraphUtil::addEdge(patch.roads, conv[src], conv[tgt], e);
		}

		patches.push_back(patch);
	}

	// 各パッチのコネクタを設定
	for (int i = 0; i < patches.size(); ++i) {
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(patches[i].roads.graph); vi != vend; ++vi) {
			if (GraphUtil::getDegree(patches[i].roads, *vi) < patches[i].roads.graph[*vi]->properties["orig_degree"].toInt()
				|| (GraphUtil::getDegree(patches[i].roads, *vi) == 1 && !patches[i].roads.graph[*vi]->deadend)) {
			//if (GraphUtil::getDegree(patches[i].roads, *vi) == 1) {
				patches[i].connectors.push_back(*vi);
				patches[i].roads.graph[*vi]->connector = true;

				// この頂点からエッジを辿りながら、各エッジにconnectorフラグをマークする
				markConnectorToEdge(patches[i].roads, *vi);
			}
		}
	}

	return patches;
}

void RoadGeneratorHelper::markConnectorToEdge(RoadGraph &roads, RoadVertexDesc srcDesc) {
	QMap<RoadVertexDesc, bool> visited;
	std::list<RoadVertexDesc> queue;

	queue.push_back(srcDesc);

	while (!queue.empty()) {
		RoadVertexDesc v = queue.front();
		queue.pop_front();

		if (visited[v]) continue;
		visited[v] = true;

		RoadOutEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::out_edges(v, roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			roads.graph[*ei]->connector = true;

			RoadVertexDesc tgt = boost::target(*ei, roads.graph);

			if (GraphUtil::getDegree(roads, tgt) == 2) {
				queue.push_back(tgt);
			}
		}
	}
}

RoadVertexDesc RoadGeneratorHelper::createEdgesByExample(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges, float &rotation_angle) {
	QMap<int, RoadVertexDesc> conv;

	// 有効な頂点をリストアップ
	int count = 0;
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			int shape_index;
			if (isShape(roads, *vi, shapes, shape_index)) continue;

			if (GraphUtil::getDegree(roads, *vi) == 1) continue;

			conv[count] = *vi;
			count++;
		}
	}

	// exampleとして使用する頂点 (v_desc) を決定する
	int index = Util::genRand(0, count);

	RoadVertexDesc v_desc = conv[index];

	int degree = GraphUtil::getDegree(roads, v_desc);

	int edge_index = Util::genRand(0, degree);
	std::vector<RoadEdgeDesc> edge_descs;
	//float rotation_angle;
	{
		RoadOutEdgeIter ei, eend;
		int i = 0;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei, ++i) {
			if (!roads.graph[*ei]->valid) continue;

			if (i == edge_index) {
				Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, v_desc);
				QVector2D vec = polyline[1] - polyline[0];
				float a = atan2f(vec.y(), vec.x());
				rotation_angle = Util::diffAngle(angle, a, false);
			} else {
				edge_descs.push_back(*ei);
			}
		}
	}

	// 各エッジを回転させ、オフセット移動する
	for (int i = 0; i < edge_descs.size(); ++i) {
		RoadVertexDesc src = boost::source(edge_descs[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edge_descs[i], roads.graph);

		QString ex_edge_name = QString("%1-%2").arg(src).arg(tgt);

		RoadEdgePtr e = RoadEdgePtr(new RoadEdge(*roads.graph[edge_descs[i]]));
		e->polyline.translate(-roads.graph[v_desc]->pt);
		if (e->polyline[0].lengthSquared() > e->polyline.back().lengthSquared()) {
			std::reverse(e->polyline.begin(), e->polyline.end());
		}
		e->polyline.rotate(Util::rad2deg(-rotation_angle));
		e->properties["mixed_desc"] = ex_edge_name;
		edges.push_back(e);
	}

	return v_desc;
}

RoadVertexDesc RoadGeneratorHelper::createEdgesByExample2(RoadGraph &roads, float angle, std::vector<RoadEdgeDescs> &shapes, std::vector<RoadEdgePtr> &edges) {
	QMap<int, RoadVertexDesc> conv;

	// 有効な頂点をリストアップ
	int count = 0;
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			int shape_index;
			if (isShape(roads, *vi, shapes, shape_index)) continue;

			if (GraphUtil::getDegree(roads, *vi) == 0) continue;

			if (roads.graph[*vi]->properties.contains("avenue_intersected")) {
				conv[count] = *vi;
				count++;
			}
		}
	}

	// exampleとして使用する頂点 (v_desc) を決定する
	int index = Util::genRand(0, count);
	RoadVertexDesc v_desc = conv[index];

	std::vector<RoadEdgeDesc> edge_descs;
	float rotation_angle;
	{
		RoadOutEdgeIter ei, eend;
		int i = 0;
		for (boost::tie(ei, eend) = boost::out_edges(v_desc, roads.graph); ei != eend; ++ei, ++i) {
			if (!roads.graph[*ei]->valid) continue;

			if (i == 0) {
				Polyline2D polyline = GraphUtil::orderPolyLine(roads, *ei, v_desc);
				QVector2D vec = polyline[1] - polyline[0];
				float a = atan2f(vec.y(), vec.x());
				rotation_angle = Util::diffAngle(angle, a, false);
			}

			edge_descs.push_back(*ei);
		}
	}

	// 各エッジを回転させ、オフセット移動する
	for (int i = 0; i < edge_descs.size(); ++i) {
		RoadVertexDesc src = boost::source(edge_descs[i], roads.graph);
		RoadVertexDesc tgt = boost::target(edge_descs[i], roads.graph);

		QString ex_edge_name = QString("%1-%2").arg(src).arg(tgt);

		RoadEdgePtr e = RoadEdgePtr(new RoadEdge(*roads.graph[edge_descs[i]]));
		e->polyline.translate(-roads.graph[v_desc]->pt);
		if (e->polyline[0].lengthSquared() > e->polyline.back().lengthSquared()) {
			std::reverse(e->polyline.begin(), e->polyline.end());
		}
		e->polyline.rotate(Util::rad2deg(-rotation_angle));
		e->properties["mixed_desc"] = ex_edge_name;
		edges.push_back(e);
	}

	return v_desc;
}

/**
 * 道路網を画像として保存する。
 */
void RoadGeneratorHelper::saveRoadImage(RoadGraph& roads, std::list<RoadVertexDesc>& seeds, const char* filename) {
	BBox bbox = GraphUtil::getAABoundingBox(roads);
	cv::Mat img(bbox.dy() + 1, bbox.dx() + 1, CV_8UC3, cv::Scalar(0, 0, 0));

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		cv::Scalar color(128, 128, 128);
		if (roads.graph[*ei]->type == RoadEdge::TYPE_STREET) {
			color = cv::Scalar(192, 192, 192);
		}

		for (int pl = 0; pl < roads.graph[*ei]->polyline.size() - 1; ++pl) {
			int x1 = roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x();
			int y1 = img.rows - (roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y());
			int x2 = roads.graph[*ei]->polyline[pl + 1].x() - bbox.minPt.x();
			int y2 = img.rows - (roads.graph[*ei]->polyline[pl + 1].y() - bbox.minPt.y());
			cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), color, 3);
		}
	}

	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
		if (!roads.graph[*vi]->valid) continue;

		int x = roads.graph[*vi]->pt.x() - bbox.minPt.x();
		int y = img.rows - (roads.graph[*vi]->pt.y() - bbox.minPt.y());

		// seedを描画
		if (std::find(seeds.begin(), seeds.end(), *vi) != seeds.end()) {
			cv::circle(img, cv::Point(x, y), 6, cv::Scalar(255, 0, 0), 3);
		}

		// onBoundaryを描画
		if (roads.graph[*vi]->onBoundary) {
			cv::circle(img, cv::Point(x, y), 10, cv::Scalar(0, 255, 255), 3);
		}

		// deadendを描画
		if (roads.graph[*vi]->deadend) {
			cv::circle(img, cv::Point(x, y), 10, cv::Scalar(0, 0, 255), 3);
		}

		// 頂点IDを描画
		QString str = QString::number(*vi);
		cv::putText(img, str.toUtf8().data(), cv::Point(x, y), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
	}

	cv::imwrite(filename, img);
}