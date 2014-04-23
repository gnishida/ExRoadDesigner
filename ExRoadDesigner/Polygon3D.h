/************************************************************************************************
*		Polygon 3D
*		@author igarciad
************************************************************************************************/
#pragma once

#include "glew.h"
#include "common.h"
#include <vector>
#include <QVector3D>

#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/geometry/multi/multi.hpp>
#endif

#ifndef BOOST_TYPEOF_SILENT
#define BOOST_TYPEOF_SILENT
#endif//BOOST_TYPEOF_SILENT



/**
* Geometry.
* Classes and functions for geometric data
**/

struct Loop3D : std::vector<QVector3D>{
};

bool isPointWithinLoop(std::vector<QVector3D> &loop, QVector3D &pt);

/**
* Stores a polygon in 3D represented by its
*     exterior contour.
**/
class Polygon3D
{
public:
	/**
	* Constructor.
	**/
	Polygon3D()
	{
		normalVec = QVector3D(0.0f, 0.0f, 0.0f);
		centroid = QVector3D(FLT_MAX, FLT_MAX, FLT_MAX);
	}

	/**
	* Destructor.
	**/
	~Polygon3D()
	{
		contour.clear();
	}

	/**
	* Copy constructor.
	**/
	Polygon3D(const Polygon3D &ref)
	{	
		contour = ref.contour;
		normalVec = ref.normalVec;
		centroid = ref.centroid;
	}

	/**
	* Assignment operator.
	**/
	inline Polygon3D &operator=(const Polygon3D &ref)
	{				
		contour = ref.contour;
		normalVec = ref.normalVec;
		centroid = ref.centroid;
		return (*this);
	}

	/**
	* Acessor to point at index idx
	**/
	inline QVector3D &operator[](const int idx)
	{	
		return contour.at(idx);
	}

	inline void push_back(const QVector3D &pt) {
		contour.push_back(pt);
	}

	/**
	* Get normal vector
	**/
	QVector3D getNormalVector(void);

	/**
	* Get center of vertices
	**/
	QVector3D getCentroid(void)
	{
		if(centroid.x() != FLT_MAX){
			return centroid;
		} else {
			QVector3D newCentroid(0.0f, 0.0f, 0.0f);
			int cSz = contour.size();
			for(int i=0; i<cSz; ++i)
			{
				newCentroid = newCentroid + contour[i];
			}
			if(cSz>0){
				centroid = (newCentroid/cSz);						
			}					
			return centroid;
		}
	}


	inline void getBBox3D(QVector3D &ptMin, QVector3D &ptMax)
	{
		ptMin.setX(FLT_MAX);
		ptMin.setY(FLT_MAX);
		ptMin.setZ(FLT_MAX);
		ptMax.setX(-FLT_MAX);
		ptMax.setY(-FLT_MAX);
		ptMax.setZ(-FLT_MAX);

		for(size_t i=0; i<contour.size(); ++i){
			if(contour[i].x() < ptMin.x()){ ptMin.setX(contour[i].x()); }
			if(contour[i].y() < ptMin.y()){ ptMin.setY(contour[i].y()); }
			if(contour[i].z() < ptMin.z()){ ptMin.setZ(contour[i].z()); }

			if(contour[i].x() > ptMax.x()){ ptMax.setX(contour[i].x()); }
			if(contour[i].y() > ptMax.y()){ ptMax.setY(contour[i].y()); }
			if(contour[i].z() > ptMax.z()){ ptMax.setZ(contour[i].z()); }
		}
	}

	inline float getMeanZValue(void)
	{
		float zVal = 0.0f;
		if(this->contour.size() > 0){
			for(size_t i=0; i<contour.size(); ++i){
				zVal += (contour[i].z());
			}
			return (zVal/((float)contour.size()));
		} else {
			return zVal;
		}
	}

	/**
	* Render polygon
	**/
	void renderContour(void);
	void render(void);
	void renderNonConvex(bool reComputeNormal = true, float nx = 0.0f, float ny = 0.0f, float nz = 1.0f);

	//Is self intersecting
	bool isSelfIntersecting(void);

	//Only works for polygons with no holes in them
	bool splitMeWithPolyline(std::vector<QVector3D> &pline, Loop3D &pgon1, Loop3D &pgon2);

	//Only works for polygons with no holes in them
	float computeInset(float offsetDistance, Loop3D &pgonInset, bool computeArea = true);
	float computeInset(std::vector<float> &offsetDistances, Loop3D &pgonInset, bool computeArea = true);

	float computeArea(bool parallelToXY = false);

	/**
	* Vector containing 3D points of polygon contour
	**/
	Loop3D contour;

	static QVector3D getLoopNormalVector(Loop3D &pin);

	static bool reorientFace(Loop3D &pface, bool onlyCheck = false);

	static int cleanLoop(Loop3D &pin, Loop3D &pout, float threshold);

	static void transformLoop(Loop3D &pin, Loop3D &pout, QMatrix4x4 &transformMat);

	static float computeLoopArea(Loop3D &pin, bool parallelToXY = false);

	static void sampleTriangularLoopInterior(Loop3D &pin, std::vector<QVector3D> &pts, float density);

	static QVector3D getLoopAABB(Loop3D &pin, QVector3D &minCorner, QVector3D &maxCorner);

	static void getLoopOBB(Loop3D &pin, QVector3D &size, QMatrix4x4 &xformMat);

	void getMyOBB(QVector3D &size, QMatrix4x4 &xformMat);

	static bool segmentSegmentIntersectXY(QVector2D &a, QVector2D &b, QVector2D &c, QVector2D &d,
		float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint);

	static void extrudePolygon(Polygon3D &basePgon, float height,
		std::vector<Polygon3D> &pgonExtrusion);

	//Shortest distance from a point to a polygon
	static float distanceXYToPoint(Loop3D &pin, QVector3D &pt);

	//minimum distance from a loop to another loop (this considers the contour only)
	static float distanceXYfromContourAVerticesToContourB(Loop3D &pA, Loop3D &pB);


private:			
	QVector3D normalVec;
	QVector3D centroid;
};	

class BBox3D{
public:

	BBox3D(){
		this->resetMe();	
	}

	~BBox3D(){
	}

	BBox3D(const BBox3D &ref){	
		minPt = ref.minPt;
		maxPt = ref.maxPt;
	}

	inline BBox3D &operator=(const BBox3D &ref){				
		minPt = ref.minPt;
		maxPt = ref.maxPt;				
		return (*this);
	}

	inline void resetMe(void){
		minPt.setX(FLT_MAX);
		minPt.setY(FLT_MAX);
		minPt.setZ(FLT_MAX);
		maxPt.setX(-FLT_MAX);
		maxPt.setY(-FLT_MAX);
		maxPt.setZ(-FLT_MAX);
	}

	inline bool overlapsWithBBox3D(BBox3D &other){
		return  
			( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
			( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) ) &&
			( (this->minPt.z() <= other.maxPt.z()) && (this->maxPt.z() >= other.minPt.z()) );
	}

	void combineWithBBox3D(BBox3D &other){
		minPt.setX(qMin(minPt.x(), other.minPt.x()));
		minPt.setY(qMin(minPt.y(), other.minPt.y()));
		minPt.setZ(qMin(minPt.z(), other.minPt.z()));

		maxPt.setX(qMax(maxPt.x(), other.maxPt.x()));
		maxPt.setY(qMax(maxPt.y(), other.maxPt.y()));
		maxPt.setZ(qMax(maxPt.z(), other.maxPt.z()));

		return;
	}

	void addPoint(QVector3D &newPt){
		minPt.setX(qMin(minPt.x(), newPt.x()));
		minPt.setY(qMin(minPt.y(), newPt.y()));
		minPt.setZ(qMin(minPt.z(), newPt.z()));

		maxPt.setX(qMax(maxPt.x(), newPt.x()));
		maxPt.setY(qMax(maxPt.y(), newPt.y()));
		maxPt.setZ(qMax(maxPt.z(), newPt.z()));
	}

	inline bool overlapsWithBBox3DXY(BBox3D &other){
		return  
			( (this->minPt.x() <= other.maxPt.x()) && (this->maxPt.x() >= other.minPt.x()) ) &&
			( (this->minPt.y() <= other.maxPt.y()) && (this->maxPt.y() >= other.minPt.y()) );					
	}

	inline QVector3D midPt(void){
		return (0.5*(minPt + maxPt));
	}

	QVector3D minPt;
	QVector3D maxPt;
};


// We can conveniently use macro's to register point and ring
//BOOST_GEOMETRY_REGISTER_POINT_2D_GET_SET(QVector3D, double, boost::geometry::cs::cartesian, x, y, setX, setY)
BOOST_GEOMETRY_REGISTER_RING(Loop3D)

	namespace boost {
		namespace geometry {

			namespace traits {

				template<> struct tag<Polygon3D> { typedef polygon_tag type; };

			} // namespace traits

			template<> struct ring_type<Polygon3D> { typedef Loop3D type; };

		} // namespace geometry
} // namespace boost

