#include "common.h"
#include "Polygon2D.h"
#include "Polyline2D.h"
#include "Util.h"

void Polygon2D::correct() {
	boost::geometry::correct(*this);
}

float Polygon2D::area() const {
	return fabs(boost::geometry::area(*this));
}

QVector2D Polygon2D::centroid() const {
	QVector2D ret;
	boost::geometry::centroid(*this, ret);

	return ret;
}

bool Polygon2D::contains(const QVector2D &pt) const {
	return boost::geometry::within(pt, *this);
}

bool Polygon2D::contains(const QVector2D &pt) {
	return boost::geometry::within(pt, *this);
}

bool Polygon2D::contains(const Polygon2D &polygon) const {
	for (int i = 0; i < polygon.size(); ++i) {
		if (!contains(polygon[i])) return false;
	}

	return true;
}

Polygon2D Polygon2D::convexHull() const {
	Polygon2D hull;
	Polygon2D poly;
	
	boost::geometry::convex_hull(*this, hull);

	return hull;
}

BBox Polygon2D::envelope() const {
	BBox bbox;
	boost::geometry::envelope(*this, bbox);
	return bbox;
}

/**
 * このポリゴンと、与えられた線分との交点を求める。交点があれば、trueを返却する。
 *
 * @param a		線分を定義する頂点の１つ
 * @param b		線分を定義する頂点の１つ
 * @param intPt	求められた交点
 * @return		交点があればtrueを返却する
 */ 
bool Polygon2D::intersects(const QVector2D& a, const QVector2D& b, QVector2D& intPt) const {
	Polyline2D polyline;
	polyline.push_back(a);
	polyline.push_back(b);

	std::vector<QVector2D> points;
	boost::geometry::intersection(*this, polyline, points);
	if (points.size() > 0) {
		intPt = points[0];
		return true;
	} else {
		return false;
	}
}

/**
 * 当該ポリゴンを移動する。
 */
void Polygon2D::translate(float x, float y) {
	Polygon2D temp = *this;
	this->clear();

	boost::geometry::strategy::transform::translate_transformer<QVector2D, QVector2D> translate(x, y);
    boost::geometry::transform(temp, *this, translate);
}

/**
 * 移動したポリゴンを返却する。
 *
 * @param x			X軸方向の移動距離
 * @param y			Y軸方向の移動距離
 * @ret				移動したポリゴン
 */
void Polygon2D::translate(float x, float y, Polygon2D &ret) const {
	boost::geometry::strategy::transform::translate_transformer<QVector2D, QVector2D> translate(x, y);
    boost::geometry::transform(*this, ret, translate);
}

/**
 * 原点を中心に、指定した角度だけ時計回りに回転する。
 *
 * @param angle		時計回りの回転角度[degree]
 */
void Polygon2D::rotate(float angle) {
	Polygon2D temp = *this;
	this->clear();

	boost::geometry::strategy::transform::rotate_transformer<QVector2D, QVector2D, boost::geometry::degree> rotate(angle);
    boost::geometry::transform(temp, *this, rotate);
}

/**
 * 原点を中心に、指定した角度だけ時計回りに回転したポリゴンを返却する。
 *
 * @param angle		時計回りの回転角度[degree]
 * @ret				回転したポリゴン
 */
void Polygon2D::rotate(float angle, Polygon2D &ret) const {
	boost::geometry::strategy::transform::rotate_transformer<QVector2D, QVector2D, boost::geometry::degree> rotate(angle);
    boost::geometry::transform(*this, ret, rotate);
}

/**
 * 指定された点を中心に、指定された角度だけ時計回りに回転したポリゴンを返却する。
 *
 * @param angle		時計回りの回転角度[degree]
 * @param orig		回転中心
 */
void Polygon2D::rotate(float angle, const QVector2D &orig) {
	translate(-orig.x(), -orig.y());
	rotate(angle);
	translate(orig.x(), orig.y());
}

/**
 * このポリゴンを三角形または凸四角形の集合に分割する。
 * 各図形の頂点は、openでCCWオーダである。
 */
std::vector<Polygon2D> Polygon2D::tessellate() {
	std::vector<Polygon2D> trapezoids;

	if (size() < 3) return trapezoids;

	// create 2D polygon data
	std::vector<boost::polygon::point_data<float>  > polygon;
	polygon.resize(size());
	for (int i = 0; i < size(); i++) {
		polygon[i] = boost::polygon::construct<boost::polygon::point_data<float> >(at(i).x(), at(i).y());
	}

	// create 2D polygon with holes data
	boost::polygon::polygon_with_holes_data<float> temp;
	boost::polygon::set_points(temp, polygon.begin(), polygon.end());

	// create 2D polygon set
	boost::polygon::polygon_set_data<float> polygonSet;
	polygonSet.insert(temp);

	// tessellation
	std::vector<boost::polygon::polygon_with_holes_data<float> > results;
	polygonSet.get_trapezoids(results);

	for (int i = 0; i < results.size(); i++) {
		boost::polygon::polygon_with_holes_data<float>::iterator_type it = results[i].begin();
		Polygon2D trapezoid;
		while (it != results[i].end()) {
			float z = 0.0f;
			bool done = false;

			trapezoid.push_back(QVector2D((*it).x(), (*it).y()));
			it++;
		}

		if (trapezoid.size() < 3) continue;

		//Polygon2D::reorientFace(trapezoid);

		// The resulting polygon is usually closed, so remove the last point.
		if ((trapezoid[trapezoid.size() - 1] - trapezoid[0]).lengthSquared() == 0) {
			//trapezoid.pop_back();
		}

		if (trapezoid.size() >= 3) trapezoids.push_back(trapezoid);
	}

	return trapezoids;
}

/**
* Get polygon oriented bounding box
**/
QVector2D Polygon2D::getOBB(Polygon2D &obb) const {
	QVector2D minSize;
	float minArea = std::numeric_limits<float>::max();

	int cSz = this->size();
	for (int i=0; i<cSz; ++i) {
		QVector2D dir = (at((i+1) % cSz) - at(i)).normalized();
		Polygon2D rotBBox;
		QVector2D size = getOBB(dir, rotBBox);

		float area = rotBBox.area();
		if (area < minArea) {
			minArea = area;
			minSize = size;
			obb = rotBBox;
		}
	}

	return minSize;
}

/**
 * 指定されたX軸方向を使って、AABBを計算する。
 *
 * @dir			指定されたX軸方向
 * @obb			求められたOBB
 * @bbox		求められたOBBの、ローカル座標におけるbbox
 **/
QVector2D Polygon2D::getOBB(const QVector2D& dir, Polygon2D& obb) const {
	// もともとのポリゴンの中心を保存する
	QVector2D center = this->centroid();

	// 指定された方向がX軸になるよう、ポリゴンを回転する
	float alpha = atan2f(dir.y(), dir.x());
	Polygon2D rotPolygon;
	rotate(Util::rad2deg(alpha), rotPolygon);

	// 回転したポリゴンの中心が原点になるよう、移動する
	Polygon2D rotPolygon2;
	QVector2D center2 = rotPolygon.centroid();
	rotPolygon.translate(-center2.x(), -center2.y());

	// 回転したポリゴンに対して、Bounding Boxを求める
	BBox bbox = rotPolygon.envelope();

	// Bounding Boxのサイズを計算する
	QVector2D size(bbox.dx(), bbox.dy());

	// Bounding Boxをポリゴンオブジェクトにコンバートする
	boost::geometry::convert(bbox, obb);

	// Bounding Boxを、再度回転して戻す
	obb.rotate(Util::rad2deg(-alpha));

	// 再度、もともとのポリゴンの中心に戻す
	obb.translate(center.x(), center.y());

	return size;
}

boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > Polygon2D::convertToBoostPolygon() const {
	boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > ret;

	for (int i = 0; i < size(); ++i) {
		ret.outer().push_back(boost::geometry::model::d2::point_xy<double>(at(i).x(), at(i).y()));
	}

	return ret;
}