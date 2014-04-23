/************************************************************************************************
*		Polygon 3D
*		@author igarciad
************************************************************************************************/
#pragma once

#include "glew.h"
#include <vector>
#include "qvector3d.h"


struct Loop3D : std::vector<QVector3D>{
};

bool isPointWithinLoop(std::vector<QVector3D> &loop, QVector3D &pt);

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
//BOOST_GEOMETRY_REGISTER_RING(Loop3D)

/*
namespace boost {
	namespace geometry {

		namespace traits {

			template<> struct tag<Polygon3D> { typedef polygon_tag type; };

		} // namespace traits

		template<> struct ring_type<Polygon3D> { typedef Loop3D type; };

	} // namespace geometry
} // namespace boost
*/
