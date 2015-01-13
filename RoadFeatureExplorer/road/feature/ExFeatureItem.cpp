#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "../../common/Util.h"
#include "ExFeatureItem.h"

void ExFeatureItem::addEdge(const Polyline2D &polyline, int lanes, bool deadend, bool onBoundary) {
	// 既に同じpolylineのエッジが存在する場合は、登録しない
	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i].edge.size() != polyline.size()) continue;

		bool duplicated = true;
		for (int j = 0; j < edges[i].edge.size(); ++j) {
			if ((edges[i].edge[j] - polyline[j]).lengthSquared() > 0.0f) {
				duplicated = false;
				break;
			}
		}

		if (duplicated) return;
	}

	edges.push_back(ExFeatureItemEdge(polyline, lanes, deadend, onBoundary));
}

/**
 * 与えられた方向・長さに最も近い距離を返却する。
 * edges[i]は、頂点自身の座標である(0, 0)を含まず、次の点の座標から始まる。しかも、各点の座標は、頂点自身の座標からの相対座標である。
 * 一方、polylineには、頂点自身の座標から始まる、各点の絶対座標が入っている。
 * したがって、polylineに対しては、polyline[0]を引いて相対座標にしてやる必要がある。
 */
float ExFeatureItem::getMinDistance(const Polyline2D &polyline) const {
	float min_dist2 = std::numeric_limits<float>::max();

	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i].edge.size() == 0) continue;

		// 頂点から、エッジのもう一方の端へのベクトルを計算する
		QVector2D dir1 = edges[i].edge.last();
		QVector2D dir2 = polyline.last() - polyline[0];

		float dist2 = (dir1 - dir2).lengthSquared();
		if (dist2 < min_dist2) {
			min_dist2 = dist2;
		}
	}

	return sqrtf(min_dist2);
}

/**
 * 与えられたエッジとの方向の差の最小値を返却する。
 */
float ExFeatureItem::getMinAngle(const Polyline2D &polyline) const {
	float min_angle = std::numeric_limits<float>::max();

	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i].edge.size() == 0) continue;

		float angle = Util::diffAngle(polyline.last() - polyline[0], edges[i].edge.last());
		if (angle < min_angle) {
			min_angle = angle;
		}
	}

	return min_angle;
}

/**
 * 指定された角度[degree]だけ、交差点カーネルを時計回りに回転する。
 */
void ExFeatureItem::rotate(float deg, const QVector2D &orig) {
	pt = Util::rotate(pt, -Util::deg2rad(deg), orig);

	for (int i = 0; i < edges.size(); ++i) {
		edges[i].edge.rotate(deg, orig);
	}
}

void ExFeatureItem::scale(float scaleX, float scaleY, const QVector2D &orig) {
	QVector2D vec = pt - orig;
	pt.setX(orig.x() + vec.x() * scaleX);
	pt.setY(orig.y() + vec.y() * scaleY);

	for (int i = 0; i < edges.size(); ++i) {
		edges[i].scale(scaleX, scaleY, orig);
	}
}

void ExFeatureItem::load(QDomNode& node) {
	edges.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "edge") {
			Polyline2D polyline;

			QDomNode child2 = child.firstChild();
			while (!child2.isNull()) {
				if (child2.toElement().tagName() == "point") {
					float x = child2.toElement().attribute("x").toFloat();
					float y = child2.toElement().attribute("y").toFloat();
					polyline.push_back(QVector2D(x, y));
				}

				child2 = child2.nextSibling();
			}

			edges.push_back(ExFeatureItemEdge(polyline, child.toElement().attribute("lanes").toFloat(), child.toElement().attribute("deadend") == "true", child.toElement().attribute("onBoundary") == "true"));
		}

		child = child.nextSibling();
	}
}

void ExFeatureItem::save(QDomDocument& doc, QDomNode& node) {
	for (int i = 0; i < edges.size(); ++i) {
		QDomElement node_edge = doc.createElement("edge");

		node_edge.setAttribute("lanes", edges[i].lanes);

		QString str;
		if (edges[i].deadend) {
			node_edge.setAttribute("deadend", "true");
		} else {
			node_edge.setAttribute("deadend", "false");
		}
		
		if (edges[i].onBoundary) {
			node_edge.setAttribute("onBoundary", "true");
		} else {
			node_edge.setAttribute("onBoundary", "false");
		}

		for (int j = 0; j < edges[i].edge.size(); ++j) {
			QDomElement node_point = doc.createElement("point");

			node_point.setAttribute("x", edges[i].edge[j].x());
			node_point.setAttribute("y", edges[i].edge[j].y());

			node_edge.appendChild(node_point);
		}
		
		node.appendChild(node_edge);
	}
}

void ExFeatureItem::imwrite(const QString filename) const {
	cv::Mat mat = cv::Mat_<uchar>::zeros(320, 320);

	for (int i = 0; i < edges.size(); ++i) {
		if (edges[i].edge.size() == 0) continue;

		cv::line(mat, cv::Point(160, 160), cv::Point(edges[i].edge[0].x() * 0.1f + 160, edges[i].edge[0].y() * 0.1f + 160), cv::Scalar(255), 3, CV_AA);
		for (int j = 1; j < edges[i].edge.size(); ++j) {
			cv::line(mat, cv::Point(edges[i].edge[j-1].x() * 0.1f + 160, edges[i].edge[j-1].y() * 0.1f + 160), cv::Point(edges[i].edge[j].x() * 0.1f + 160, edges[i].edge[j].y() * 0.1f + 160), cv::Scalar(255), 3, CV_AA);
		}
	}

	cv::flip(mat, mat, 0);

	cv::imwrite(filename.toUtf8().data(), mat);
}