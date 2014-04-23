#include "BBox3D.h"
#include "qvector2d.h"
#include "qmatrix4x4.h"

double angleBetweenVectors(QVector3D &vec1, QVector3D &vec2)
{	
	return acos( 0.999*(QVector3D::dotProduct(vec1, vec2)) / ( vec1.length() * vec2.length() ) );
}

/**
* Given three non colinear points p0, p1, p2, this function computes
* the intersection between the lines A and B. Line A is the line parallel to the segment p0-p1
* and at a distance d01 from that segment. Line B is the line parallel to the segment
* p1-p2 at a distance d12 from that segment.
* Returns true if point is successfully computed
**/

bool getIrregularBisector(QVector3D &p0,
	QVector3D &p1, QVector3D &p2, float d01, float d12,
	QVector3D &intPt)
{
	double alpha;
	double theta;
	double L;

	QVector3D p1_p0;
	QVector3D p1_p2;
	QVector3D p1_p2_perp;
	QVector3D crossP;

	p1_p0 = p0 - p1;
	p1_p0.setZ(0.0f);

	p1_p2 = p2 - p1;
	p1_p2.setZ(0.0f);

	p1_p2_perp.setX( -p1_p2.y() );
	p1_p2_perp.setY(  p1_p2.x() );
	p1_p2_perp.setZ( 0.0f );

	alpha = angleBetweenVectors(p1_p0, p1_p2);				

	if( !(alpha == alpha) ){
		return false;
	}				

	theta = atan2( sin(alpha), (d01 / d12) + cos(alpha) );				
	L = d12 / sin(theta);

	//This is done to handle convex vs. concave angles in polygon
	crossP = QVector3D::crossProduct(p1_p2, p1_p0);

	if(crossP.z() > 0){
		//CCW polygon (A + B + C)
		//CW  polygon (A - B - C)
		intPt = p1 + (p1_p2.normalized())*L*cos(theta) +
			(p1_p2_perp.normalized())*d12;
	}
	else {
		//CCW polygon (A - B + C)
		//CW  polygon (A + B - C)
		intPt = p1 - (p1_p2.normalized())*L*cos(theta) +
			(p1_p2_perp.normalized())*d12;
	}
	return true;
}

/**
* Checks if contour A is within contour B
**/
/*bool is2DRingWithin2DRing( boost::geometry::ring_type<Polygon3D>::type &contourA, boost::geometry::ring_type<Polygon3D>::type &contourB)
{
	for(int i=0; i<contourA.size(); ++i){
		if( !boost::geometry::within( contourA[i], contourB) ){
			return false;
		}
	}
	return true;
}

QVector3D calculateNormal(QVector3D& p0,QVector3D& p1,QVector3D& p2)
{
	return (QVector3D::normal((p1-p0),(p2-p1)));
}

const float MTC_FLOAT_TOL = 1e-6f;

float pointSegmentDistanceXY(QVector3D &a, QVector3D &b, QVector3D &c)
{
	QVector3D closestPt;
	return pointSegmentDistanceXY(a, b, c, closestPt);

}

*/