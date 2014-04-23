#pragma once

#include "common.h"
#include "BBox.h"
#include "Polyline3D.h"

class Polygon3D : public std::vector<QVector3D> {
public:
	Polygon3D() {}
	~Polygon3D() {}

	const QVector3D & last() const;
	QVector3D & last();
	void correct();
	float area() const;
	QVector2D centroid() const;
	bool contains(const QVector3D &pt) const;
	bool contains(const QVector3D &pt);
	bool contains(const Polygon3D &polygon) const;
	BBox envelope() const;
	void translate(float x, float y);
	void translate(float x, float y, Polygon3D &ret) const;
	void rotate(float angle);
	void rotate(float angle, Polygon3D &ret) const;
	void rotate(float angle, const QVector2D &orig);

	std::vector<Polygon3D> tessellate() const;
	void findEdge(float x, float y, int& v1, int& v2, float& s) const;


	void computeInset(float offsetDistance, Polygon3D &pgonInset);
	void computeInset(std::vector<float> offsetDistances, Polygon3D &pgonInset);
	bool splitMeWithPolyline(const Polyline3D& pline, Polygon3D &pgon1, Polygon3D &pgon2);
	float distanceXYfromContourAVerticesToContourB(const Polygon3D& pB);
	static void getLoopOBB(const Polygon3D& pin, QVector3D& size, QMatrix4x4& xformMat);
	static void transformLoop(const Polygon3D& pin, Polygon3D& pout, QMatrix4x4& transformMat);

	inline void getBBox3D(QVector3D &ptMin, QVector3D &ptMax)
	{
		ptMin.setX(FLT_MAX);
		ptMin.setY(FLT_MAX);
		ptMin.setZ(FLT_MAX);
		ptMax.setX(-FLT_MAX);
		ptMax.setY(-FLT_MAX);
		ptMax.setZ(-FLT_MAX);

		for(size_t i=0; i<size(); ++i){
			if(at(i).x() < ptMin.x()){ ptMin.setX(at(i).x()); }
			if(at(i).y() < ptMin.y()){ ptMin.setY(at(i).y()); }
			if(at(i).z() < ptMin.z()){ ptMin.setZ(at(i).z()); }

			if(at(i).x() > ptMax.x()){ ptMax.setX(at(i).x()); }
			if(at(i).y() > ptMax.y()){ ptMax.setY(at(i).y()); }
			if(at(i).z() > ptMax.z()){ ptMax.setZ(at(i).z()); }
		}
	}
};

/**
 * Polygon3DをBoostのringの代替として使用
 */
BOOST_GEOMETRY_REGISTER_RING(Polygon3D)

namespace boost {
namespace geometry {
namespace traits {

template<>
struct point_order<Polygon3D>
{
    static const order_selector value = counterclockwise; 
};

template<>
struct closure<Polygon3D>
{
    static const closure_selector value = open;
};

}
}
} //namespace boost { namespace geometry