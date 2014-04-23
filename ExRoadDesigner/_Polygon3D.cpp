#include <QMatrix4x4>
#include "Polygon3D.h"
#include "Polyline3D.h"
#include "Util.h"

const QVector3D & Polygon3D::last() const {
	return at(size() - 1);
}

QVector3D & Polygon3D::last() {
	return at(size() - 1);
}

void Polygon3D::correct() {
	boost::geometry::correct(*this);
}

float Polygon3D::area() const {
	return fabs(boost::geometry::area(*this));
}

QVector2D Polygon3D::centroid() const {
	QVector2D ret;
	boost::geometry::centroid(*this, ret);

	return ret;
}

bool Polygon3D::contains(const QVector3D &pt) const {
	return boost::geometry::within(pt, *this);
}

bool Polygon3D::contains(const QVector3D &pt) {
	return boost::geometry::within(pt, *this);
}

bool Polygon3D::contains(const Polygon3D &polygon) const {
	for (int i = 0; i < polygon.size(); ++i) {
		if (!contains(polygon[i])) return false;
	}

	return true;
}

BBox Polygon3D::envelope() const {
	BBox bbox;
	boost::geometry::envelope(*this, bbox);
	return bbox;
}

/**
 * 当該ポリゴンを移動する。
 */
void Polygon3D::translate(float x, float y) {
	Polygon3D temp = *this;
	this->clear();

	boost::geometry::strategy::transform::translate_transformer<QVector3D, QVector3D> translate(x, y);
    boost::geometry::transform(temp, *this, translate);
}

/**
 * 移動したポリゴンを返却する。
 *
 * @param x			X軸方向の移動距離
 * @param y			Y軸方向の移動距離
 * @ret				移動したポリゴン
 */
void Polygon3D::translate(float x, float y, Polygon3D &ret) const {
	boost::geometry::strategy::transform::translate_transformer<QVector3D, QVector3D> translate(x, y);
    boost::geometry::transform(*this, ret, translate);
}

/**
 * 原点を中心に、指定した角度だけ時計回りに回転する。
 *
 * @param angle		時計回りの回転角度[degree]
 */
void Polygon3D::rotate(float angle) {
	Polygon3D temp = *this;
	this->clear();

	boost::geometry::strategy::transform::rotate_transformer<QVector3D, QVector3D, boost::geometry::degree> rotate(angle);
    boost::geometry::transform(temp, *this, rotate);
}

/**
 * 原点を中心に、指定した角度だけ時計回りに回転したポリゴンを返却する。
 *
 * @param angle		時計回りの回転角度[degree]
 * @ret				回転したポリゴン
 */
void Polygon3D::rotate(float angle, Polygon3D &ret) const {
	boost::geometry::strategy::transform::rotate_transformer<QVector3D, QVector3D, boost::geometry::degree> rotate(angle);
    boost::geometry::transform(*this, ret, rotate);
}

/**
 * 指定された点を中心に、指定された角度だけ時計回りに回転したポリゴンを返却する。
 *
 * @param angle		時計回りの回転角度[degree]
 * @param orig		回転中心
 */
void Polygon3D::rotate(float angle, const QVector2D &orig) {
	translate(-orig.x(), -orig.y());
	rotate(angle);
	translate(orig.x(), orig.y());
}

/**
 * このポリゴンを三角形または凸四角形の集合に分割する。
 * 2Dでのtessellationを行い、得られる図形のZ座標は全て0となる。
 * また、tessellateされた各図形の頂点はclosedで、CCWオーダである。
 */
std::vector<Polygon3D> Polygon3D::tessellate() const {
	std::vector<Polygon3D> trapezoids;

	if (size() < 3) return trapezoids;

	typedef boost::polygon::point_data<double> point;
	typedef boost::polygon::polygon_set_data<double> polygon_set;
	typedef boost::polygon::polygon_with_holes_data<double> bpolygon;

	std::vector<point> pts;
	for (int i = size() - 1; i >= 0; i--) {
		pts.push_back(point(at(i).x(), at(i).y()));
	}

	bpolygon ply;
	boost::polygon::set_points( ply, pts.begin(), pts.end() );

	std::vector<bpolygon> ots;
	polygon_set plys;
	plys.insert(ply);
	plys.get_trapezoids(ots);

	for (int i = 0; i < ots.size(); ++i) {
		Polygon3D trapezoid;

		for (boost::polygon::polygon_with_holes_data<double>::iterator_type it = ots[i].begin(); it != ots[i].end(); ++it) {
			float z;

			// 直近の頂点を探してZ座標を決定する
			/*
			float min_dist = std::numeric_limits<float>::max();
			for (int j = 0; j < size(); ++j) {
				float dist = SQR(at(j).x() - (*it).x()) + SQR(at(j).y() - (*it).y());
				if (dist < min_dist) {
					min_dist = dist;
					z = at(j).z();
				}
			}*/

			bool done = false;

			for (int j = 0; j < size(); j++) {
				if (at(j).x() == (*it).x() && at(j).y() == (*it).y()) {
					z = at(j).z();
					done = true;
					break;
				}
			}
			if (!done) {
				int v1, v2;
				float s;
				findEdge((*it).x(), (*it).y(), v1, v2, s);
				z = at(v1).z() + (at(v2).z() - at(v1).z()) * s;
			}

			trapezoid.push_back(QVector3D((*it).x(), (*it).y(), z));
		}

		if (trapezoid.size() >= 4) {
			// closedのポリゴンなので、最初の頂点を削除する
			trapezoid.erase(trapezoid.begin());

			// CWオーダなので、CCWオーダにする
			std::reverse(trapezoid.begin(), trapezoid.end());

			trapezoids.push_back(trapezoid);
		}
	}

	return trapezoids;
}

void Polygon3D::findEdge(float x, float y, int& v1, int& v2, float& s) const {
	float minDist = (std::numeric_limits<float>::max)();

	for (int i = 0; i < size(); i++) {
		int next = (i + 1) % size();

		float dist = Util::pointSegmentDistanceXY(at(i), at(next), QVector3D(x, y, 0));
		if (dist < minDist) {
			minDist = dist;
			v1 = i;
			v2 = next;
			if (fabs(at(next).x() - at(i).x()) > fabs(at(next).y() - at(i).y())) {
				s = (x - at(i).x()) / (at(next).x() - at(i).x());
			} else {
				if (at(next).y() - at(i).y() != 0.0f) {
					s = (y - at(i).y()) / (at(next).y() - at(i).y());                                
				} else {
					s = 0.0f;
				}
			}
		}
	}
}

/**
* @brief: Given a polygon, this function computes the polygon's inwards offset. The offset distance
* is not assumed to be constant and must be specified in the vector offsetDistances. The size of this
* vector must be equal to the number of vertices of the polygon.
* Note that the i-th polygon segment is defined by vertices i and i+1.
* The polygon vertices are assumed to be oriented clockwise
* @param[in] offsetDistances: Perpendicular distance from offset segment i to polygon segment i.
* @param[out] pgonInset: The vertices of the polygon inset
* @return insetArea: Returns the area of the polygon inset		
**/
void Polygon3D::computeInset(float offsetDistance, Polygon3D &pgonInset) {
	if (size() < 3) return;
	std::vector<float> offsetDistances(size(), offsetDistance);

	computeInset(offsetDistances, pgonInset);
}

void Polygon3D::computeInset(std::vector<float> offsetDistances, Polygon3D &pgonInset) {
	Polygon3D cleanPgon = *this;
	double tol = 0.01f;

	int prev, next;
	int cSz = cleanPgon.size();

	if (cSz < 3) return;

	//if offsets are zero, add a small epsilon just to avoid division by zero
	for (size_t i=0; i<offsetDistances.size(); ++i){
		if(fabs(offsetDistances[i]) < tol){
			offsetDistances[i] = tol;
		}
	}

	//pgonInset.resize(cSz);
	QVector3D intPt;

	for (int cur = 0; cur < cSz; ++cur) {
		//point p1 is the point with index cur
		prev = (cur-1+cSz)%cSz; //point p0
		next = (cur+1)%cSz;	  //point p2

		// Deanend対策
		if (Util::diffAngle(cleanPgon[prev] - cleanPgon[cur], cleanPgon[next] - cleanPgon[cur]) < 0.1f) {
			QVector3D vec = cleanPgon[cur] - cleanPgon[prev];
			QVector3D vec2(-vec.y(), vec.x(), 0);
			/*
			vec2 = vec2.normalized() * offsetDistances[cur];
			intPt = cleanPgon[cur] + vec2;
			pgonInset.push_back(intPt);
			*/

			float angle = atan2f(vec2.y(), vec2.x());
			for (int i = 0; i <= 10; ++i) {
				float a = angle - (float)i * M_PI / 10.0f;
				intPt = QVector3D(cleanPgon[cur].x() + cosf(a) * offsetDistances[cur], cleanPgon[cur].y() + sinf(a) * offsetDistances[cur], cleanPgon[cur].z());
				pgonInset.push_back(intPt);
			}

			/*
			vec = vec.normalized() * offsetDistances[cur];
			intPt = cleanPgon[cur] + vec;
			pgonInset.push_back(intPt);

			intPt = cleanPgon[cur] - vec2;
			pgonInset.push_back(intPt);
			*/
		} else {
			Util::getIrregularBisector(cleanPgon[prev], cleanPgon[cur], cleanPgon[next], offsetDistances[prev], offsetDistances[cur], intPt);
			//pgonInset[cur] = intPt;

			// 鋭角対策
			if (pgonInset.size() >= 2) {
				if (Util::diffAngle(pgonInset[pgonInset.size() - 2] - pgonInset[pgonInset.size() - 1], intPt - pgonInset[pgonInset.size() - 1]) < 0.1f) {
					pgonInset.erase(pgonInset.begin() + pgonInset.size() - 1);
				}
			}

			pgonInset.push_back(intPt);
		}
	}
}

/**
 * Only works for polygons with no holes in them
 */
bool Polygon3D::splitMeWithPolyline(const Polyline3D& pline, Polygon3D &pgon1, Polygon3D &pgon2) {
	bool polylineIntersectsPolygon = false;

	size_t plineSz = pline.size();
	size_t contourSz = this->size();

	if(plineSz < 2 || contourSz < 3){
		//std::cout << "ERROR: Cannot split if polygon has fewer than three vertices of if polyline has fewer than two points\n.";
		return false;
	}

	QVector2D tmpIntPt;
	QVector3D firstIntPt;
	QVector3D secondIntPt;
	float tPline, tPgon;
	int firstIntPlineIdx    = -1;
	int secondIntPlineIdx   = -1;
	int firstIntContourIdx  = -1;
	int secondIntContourIdx = -1;
	int intCount = 0;

	//iterate along polyline
	for (int i = 0; i < plineSz - 1; ++i) {
		int iNext = i+1;

		for (int j = 0; j < contourSz; ++j) {
			int jNext = (j+1)%contourSz;

			if (Util::segmentSegmentIntersectXY(QVector2D(pline[i]), QVector2D(pline[iNext]), QVector2D(at(j)), QVector2D(at(jNext)), &tPline, &tPgon, true, tmpIntPt)) {
				polylineIntersectsPolygon = true;
				float z = at(j).z() + (at(jNext).z() - at(j).z()) * tPgon;

				//first intersection
				if (intCount == 0) {
					firstIntPlineIdx = i;
					firstIntContourIdx = j;
					firstIntPt = QVector3D(tmpIntPt.x(), tmpIntPt.y(), z);
				} else if (intCount == 1) {
					secondIntPlineIdx = i;
					secondIntContourIdx = j;
					secondIntPt = QVector3D(tmpIntPt.x(), tmpIntPt.y(), z);
				} else {
					return false;
				}
				intCount++;
			}
		}
	}

	if (intCount != 2) return false;

	//Once we have intersection points and indexes, we reconstruct the two polygons
	pgon1.clear();
	pgon2.clear();
	int pgonVtxIte;
	int plineVtxIte;

	//If first polygon segment intersected has an index greater
	//	than second segment, modify indexes for correct reconstruction
	if (firstIntContourIdx > secondIntContourIdx) {
		secondIntContourIdx += contourSz;
	}

	//==== Reconstruct first polygon
	//-- append polygon contour
	pgon1.push_back(firstIntPt);
	pgonVtxIte = firstIntContourIdx;
	while (pgonVtxIte < secondIntContourIdx) {
		pgon1.push_back(at((pgonVtxIte+1)%contourSz));
		pgonVtxIte++;
	}
	pgon1.push_back(secondIntPt);
	//-- append polyline points
	plineVtxIte = secondIntPlineIdx;
	while (plineVtxIte > firstIntPlineIdx) {
		pgon1.push_back(pline[(plineVtxIte)]);
		plineVtxIte--;
	}

	//==== Reconstruct second polygon
	//-- append polygon contour
	pgon2.push_back(secondIntPt);
	pgonVtxIte = secondIntContourIdx;
	while (pgonVtxIte < firstIntContourIdx + contourSz) {
		pgon2.push_back(at((pgonVtxIte+1) % contourSz));
		pgonVtxIte++;
	}
	pgon2.push_back(firstIntPt);
	//-- append polyline points
	plineVtxIte = firstIntPlineIdx;
	while (plineVtxIte < secondIntPlineIdx) {
		pgon2.push_back(pline[(plineVtxIte + 1)]);
		plineVtxIte++;
	}

	//verify that two polygons are created after the split. If not, return false
	if (pgon1.size() < 3 || pgon2.size() < 3) {
		return false;
	}

	return polylineIntersectsPolygon;
}

/**
 * this function measures the minimum distance from the vertices of a contour A
 * to the edges of a contour B, i.e., it measures the distances from each vertex of A
 * to all the edges in B, and returns the minimum of such distances
 */
float Polygon3D::distanceXYfromContourAVerticesToContourB(const Polygon3D& pB) {
	float minDist = FLT_MAX;
	float dist;

	for (size_t i = 0; i < size(); ++i) {
		dist = boost::geometry::distance(pB, this->at(i));
		if (dist < minDist) {
			minDist = dist;
		}
	}
	return minDist;
}

/**
* Get polygon oriented bounding box
**/
void Polygon3D::getLoopOBB(const Polygon3D& pin, QVector3D& size, QMatrix4x4& xformMat) {
	float alpha = 0.0f;			
	float deltaAlpha = 0.05 * M_PI;
	float bestAlpha;

	Polygon3D rotLoop;
	QMatrix4x4 rotMat;
	QVector3D minPt, maxPt;
	QVector3D origMidPt;
	QVector3D boxSz;
	QVector3D bestBoxSz;
	float curArea;
	float minArea = FLT_MAX;

	rotLoop = pin;
	//Polygon3D::getLoopAABB(rotLoop, minPt, maxPt);
	//origMidPt = 0.5f*(minPt + maxPt);
	origMidPt = rotLoop.envelope().midPt();

	int cSz = pin.size();
	QVector3D difVec;
	for (int i=0; i<pin.size(); ++i) {
		difVec = (pin.at((i+1) % cSz) - pin.at(i)).normalized();
		alpha = atan2(difVec.x(), difVec.y());
		rotMat.setToIdentity();				
		rotMat.rotate(Util::rad2deg(alpha), 0.0f, 0.0f, 1.0f);				

		transformLoop(pin, rotLoop, rotMat);
		//boxSz = Polygon3D::getLoopAABB(rotLoop, minPt, maxPt);
		BBox boxSz = rotLoop.envelope();
		curArea = boxSz.dx() * boxSz.dy();
		if (curArea < minArea) {
			minArea = curArea;
			bestAlpha = alpha;
			//bestBoxSz = boxSz;
			bestBoxSz.setX(boxSz.dx());
			bestBoxSz.setY(boxSz.dy());
		}
		//alpha += deltaAlpha;
	}

	xformMat.setToIdentity();											
	xformMat.rotate(Util::rad2deg(bestAlpha), 0.0f, 0.0f, 1.0f);
	xformMat.setRow(3, QVector4D(origMidPt.x(), origMidPt.y(), origMidPt.z(), 1.0f));			
	size = bestBoxSz;
}

void Polygon3D::transformLoop(const Polygon3D& pin, Polygon3D& pout, QMatrix4x4& transformMat) {
	pout = pin;
	for (int i=0; i<pin.size(); ++i) {
		pout.at(i) = transformMat * pin.at(i);
	}
}