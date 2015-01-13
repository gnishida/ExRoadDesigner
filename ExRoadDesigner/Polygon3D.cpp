#include "Polygon3D.h"
#include <QVector2D>
#include <QMatrix4x4>
#include "Util.h"

#include "clipper.hpp"
//#include "clipper.cpp"

//**** Polygon3D
void Polygon3D::renderContour(void)
{	
	glBegin(GL_LINE_LOOP);
	for(size_t i=0; i<contour.size(); ++i){
		glVertex3f(contour[i].x(),
			contour[i].y(),
			contour[i].z());
	}
	glEnd();	


}


void Polygon3D::render(void)
{		
	glBegin(GL_POLYGON);
	for(size_t i=0; i<contour.size(); ++i){
		glVertex3f(contour[i].x(),
			contour[i].y(),
			contour[i].z());
	}
	glEnd();			
}

void Polygon3D::renderNonConvex(bool reComputeNormal,
	float nx, float ny, float nz)
{
	QVector3D myNormal;
	if(reComputeNormal)
	{
		myNormal = this->getNormalVector();
	} else {
		myNormal.setX(nx);
		myNormal.setY(ny);
		myNormal.setZ(nz);
	}

	//Render inside fill			
	if(contour.size() == 3){
		glBegin(GL_TRIANGLES);	
		for(size_t i=0; i<contour.size(); i++){	
			glNormal3f(myNormal.x(), myNormal.y(), myNormal.z());
			glVertex3f(contour[i].x(), contour[i].y(), contour[i].z());			
		}
		glEnd();
	} else if(contour.size() == 4){
		glBegin(GL_QUADS);	
		for(int i=0; i<contour.size(); i++){	
			glNormal3f(myNormal.x(), myNormal.y(), myNormal.z());
			glVertex3f(contour[i].x(), contour[i].y(), contour[i].z());			
		}
		glEnd();
	} else {

		// create tessellator
		GLUtesselator *tess = gluNewTess();

		double *vtxData = new double[3*contour.size()];
		for(size_t i=0; i<contour.size(); i++){
			vtxData[3*i]=contour[i].x();
			vtxData[3*i+1]=contour[i].y();
			vtxData[3*i+2]=contour[i].z();
		}

		// register callback functions
		gluTessCallback(tess, GLU_TESS_BEGIN, 
			(void (__stdcall *)(void))glBegin);
		gluTessCallback(tess, GLU_TESS_VERTEX,
			(void (__stdcall *)(void))glVertex3dv);
		gluTessCallback(tess, GLU_TESS_END, glEnd);

		// describe non-convex polygon
		gluTessBeginPolygon(tess, NULL);


		// contour
		gluTessBeginContour(tess);

		for(size_t i=0; i<contour.size(); i++){
			//HACK
			glNormal3f(myNormal.x(), myNormal.y(), fabs(myNormal.z()));
			gluTessVertex(tess, &vtxData[3*i], &vtxData[3*i]);
		}
		gluTessEndContour(tess);

		gluTessEndPolygon(tess);

		// delete tessellator after processing
		gluDeleteTess(tess);

		delete [] vtxData;
	}

	//Render contour wire
	/*glColor3f(0.5f, 0.5f, 1.0f);			
	glBegin(GL_LINES);
	for(int i=0; i<contour.size()-1; ++i){
	if(i==contour.size()-2)//last
	glColor3f(0.5f, 1.0f, 1.0f);
	else
	glColor3f(0.5f, 0.5f, 1.0f);
	if(ClientGlobalVariable::gV()->render_adaptGeometry==false){
	glVertex3f(contour[i].x(),contour[i].y(),contour[i].z());
	glVertex3f(contour[i+1].x(),contour[i+1].y(),contour[i+1].z());
	}
	else{
	glVertex3f(contour[i].x(),contour[i].y(),0);
	glVertex3f(contour[i+1].x(),contour[i+1].y(),0);
	}
	}*/

	/*
	glColor3f(0.5f, 0.5f, 1.0f);
	glBegin(GL_LINE_LOOP);
	for(size_t i=0; i<contour.size(); ++i){

	if(ClientGlobalVariable::gV()->render_adaptGeometry==false)
	glVertex3f(contour[i].x(),contour[i].y(),0.0f);
	else
	glVertex3f(contour[i].x(),contour[i].y(),contour[i].z());
	}
	glEnd();
	*/
}

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
bool is2DRingWithin2DRing( boost::geometry::ring_type<Polygon3D>::type &contourA, boost::geometry::ring_type<Polygon3D>::type &contourB)
{
	for(int i=0; i<contourA.size(); ++i){
		if( !boost::geometry::within( contourA[i], contourB) ){
			return false;
		}
	}
	return true;
}

using namespace ClipperLib;

float Polygon3D::computeInset(std::vector<float> &offsetDistances, Loop3D &pgonInset, bool computeArea)
{
	Loop3D cleanPgon; 
	double tol = 0.01f;

	cleanPgon = this->contour;

	int prev, next;
	int cSz = cleanPgon.size();

	if(cSz < 3){
		return 0.0f;
	}

	if(reorientFace(cleanPgon)){				
		std::reverse(offsetDistances.begin(), offsetDistances.end() - 1);
	}

	//if offsets are zero, add a small epsilon just to avoid division by zero
	for(size_t i=0; i<offsetDistances.size(); ++i){
		if(fabs(offsetDistances[i]) < tol){
			offsetDistances[i] = tol;
		}
	}

	//pgonInset.resize(cSz);

	QVector3D intPt;


	/*
	// GEN CODE--> It leads to self-intersection very often with non-convex polygons
	for(int cur=0; cur<cSz; ++cur){
		//Some geometry and trigonometry

		//point p1 is the point with index cur
		prev = (cur-1+cSz)%cSz; //point p0
		next = (cur+1)%cSz;	  //point p2

		if (Util::diffAngle(cleanPgon[prev] - cleanPgon[cur], cleanPgon[next] - cleanPgon[cur]) < 0.1f) {
			// For deanend edge
			QVector3D vec = cleanPgon[cur] - cleanPgon[prev];
			QVector3D vec2(-vec.y(), vec.x(), 0);

			float angle = atan2f(vec2.y(), vec2.x());
			for (int i = 0; i <= 10; ++i) {
				float a = angle - (float)i * M_PI / 10.0f;
				intPt = QVector3D(cleanPgon[cur].x() + cosf(a) * offsetDistances[cur], cleanPgon[cur].y() + sinf(a) * offsetDistances[cur], cleanPgon[cur].z());
				pgonInset.push_back(intPt);
			}
		} else {
			Util::getIrregularBisector(cleanPgon[prev], cleanPgon[cur], cleanPgon[next], offsetDistances[prev], offsetDistances[cur], intPt);
			
			// For acute angle
			if (pgonInset.size() >= 2) {
				if (Util::diffAngle(pgonInset[pgonInset.size() - 2] - pgonInset[pgonInset.size() - 1], intPt - pgonInset[pgonInset.size() - 1]) < 0.1f) {
					pgonInset.erase(pgonInset.begin() + pgonInset.size() - 1);
				}
			}

			pgonInset.push_back(intPt);
		}
	}*/
	
	// Old Code
	pgonInset.resize(cSz);
	for(int cur=0; cur<cSz; ++cur){
		//Some geometry and trigonometry

		//point p1 is the point with index cur
		prev = (cur-1+cSz)%cSz; //point p0
		next = (cur+1)%cSz;	  //point p2

		getIrregularBisector(cleanPgon[prev], cleanPgon[cur], cleanPgon[next],
			offsetDistances[prev], offsetDistances[cur], intPt);

		pgonInset[cur] = intPt;
	}



	//temp
	

	//Compute inset area
	if(computeArea){

		boost::geometry::ring_type<Polygon3D>::type bg_contour;
		boost::geometry::ring_type<Polygon3D>::type bg_contour_inset;
		float contArea;
		float contInsetArea;

		if(pgonInset.size()>0){
			boost::geometry::assign(bg_contour_inset, pgonInset);
			boost::geometry::correct(bg_contour_inset);

			if(boost::geometry::intersects(bg_contour_inset)){
				//printf("INSET: intersects\n");
				pgonInset.clear();
				//return 0.0f;
			} else {

				boost::geometry::assign(bg_contour, cleanPgon);
				boost::geometry::correct(bg_contour);
				//if inset is not within polygon
				if( !is2DRingWithin2DRing(bg_contour_inset, bg_contour) ){
					pgonInset.clear();
					//printf("INSET: ringWithRing\n");
					//return 0.0f;
				} else {
					contArea = fabs(boost::geometry::area(bg_contour));
					contInsetArea = fabs(boost::geometry::area(bg_contour_inset));

					if(contInsetArea < contArea){// OK EXIT
						//return boost::geometry::area(bg_contour_inset);
						return contInsetArea;
					} else {
						//printf("INSET: contInsetArea < contArea\n");
						pgonInset.clear();
						//return 0.0f;
					}
				}
			}
		} else {
			//printf("INSET: sides <0\n");
			pgonInset.clear();
			//return 0.0f;
		}
		// IT FAILED TRY SECOND METHOD
		{
			Path subj;
			Paths solution;
			for(int cur=0; cur<cSz; ++cur){
				subj << IntPoint(cleanPgon[cur].x()*1000,cleanPgon[cur].y()*1000);
			}
			/*subj << 
				ClipperLib::IntPoint(348,257) << IntPoint(364,148) << IntPoint(362,148) << 
				IntPoint(326,241) << IntPoint(295,219) << IntPoint(258,88) << 
				IntPoint(440,129) << IntPoint(370,196) << IntPoint(372,275);*/
			ClipperOffset co;
			co.AddPath(subj, jtSquare, etClosedPolygon);
			co.Execute(solution, -1000*7.5);
			pgonInset.resize(solution[0].size());
			for(int sN=0;sN<solution[0].size();sN++){
				pgonInset[sN]=QVector3D(solution[0][sN].X/1000.0f,solution[0][sN].Y/1000.0f,0);
			}
			//printf("Solutions %d\n",solution.size());
			return Area(solution[0]);
		}

	}
	return 0.0f;

}

QVector3D calculateNormal(QVector3D& p0,QVector3D& p1,QVector3D& p2)
{
	return (QVector3D::normal((p1-p0),(p2-p1)));
}

QVector3D Polygon3D::getLoopNormalVector(Loop3D &pin)
{
	if(pin.size() >= 3){
		return (calculateNormal(pin[0], pin[1], pin[2]));
	}
	return ( QVector3D(0, 0, 0) );
}

QVector3D Polygon3D::getNormalVector()
{	
	if(this->normalVec.isNull())
	{
		normalVec = getLoopNormalVector(this->contour);
	}
	return normalVec;
}

const float MTC_FLOAT_TOL = 1e-6f;

/**
* Computes the intersection between two line segments on the XY plane
* Segments must intersect within their extents for the intersection to be valid
* z coordinate is ignored
**/
bool Polygon3D::segmentSegmentIntersectXY(QVector2D &a, QVector2D &b, QVector2D &c, QVector2D &d,
	float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint)
{
	QVector2D u = b - a;
	QVector2D v = d - c;

	if( u.lengthSquared() < MTC_FLOAT_TOL  ||  v.lengthSquared() < MTC_FLOAT_TOL )
	{
		return false;
	}

	float numer = v.x()*(c.y()-a.y()) + v.y()*(a.x()-c.x());
	float denom = u.y()*v.x() - u.x()*v.y();

	if (denom == 0.0f)  {
		// they are parallel
		*tab = 0.0f;
		*tcd = 0.0f;
		return false;
	}

	float t0 = numer / denom;

	QVector2D ipt = a + t0*u;
	QVector2D tmp = ipt - c;
	float t1;
	if (QVector2D::dotProduct(tmp, v) > 0.0f){
		t1 = tmp.length() / v.length();
	}
	else {
		t1 = -1.0f * tmp.length() / v.length();
	}

	//Check if intersection is within segments
	if( !( (t0 >= MTC_FLOAT_TOL) && (t0 <= 1.0f-MTC_FLOAT_TOL) && (t1 >= MTC_FLOAT_TOL) && (t1 <= 1.0f-MTC_FLOAT_TOL) ) ){
		return false;
	}

	*tab = t0;
	*tcd = t1;
	QVector2D dirVec = b-a;

	intPoint = a+(*tab)*dirVec;
	return true;
}

void Polygon3D::transformLoop(Loop3D &pin, Loop3D &pout, QMatrix4x4 &transformMat)
{
	pout = pin;
	for(int i=0; i<pin.size(); ++i){
		pout.at(i) = transformMat*pin.at(i);
	}
}

//Only works for polygons with no holes in them
bool Polygon3D::splitMeWithPolyline(std::vector<QVector3D> &pline, Loop3D &pgon1, Loop3D &pgon2)
{
	bool polylineIntersectsPolygon = false;

	int plineSz = pline.size();
	int contourSz = this->contour.size();

	if(plineSz < 2 || contourSz < 3){
		//std::cout << "ERROR: Cannot split if polygon has fewer than three vertices of if polyline has fewer than two points\n.";
		return false;
	}

	QVector2D tmpIntPt;
	QVector2D firstIntPt;
	QVector2D secondIntPt;
	float tPline, tPgon;
	int firstIntPlineIdx    = -1;
	int secondIntPlineIdx   = -1;
	int firstIntContourIdx  = -1;
	int secondIntContourIdx = -1;
	int intCount = 0;


	//iterate along polyline
	for(int i=0; i<plineSz-1; ++i){
		int iNext = i+1;

		for(int j=0; j<contourSz; ++j){
			int jNext = (j+1)%contourSz;

			if (segmentSegmentIntersectXY( QVector2D(pline[i]), QVector2D(pline[iNext]),
				QVector2D(contour[j]), QVector2D(contour[jNext]),
				&tPline, &tPgon, true, tmpIntPt) ) 
			{
				polylineIntersectsPolygon = true;

				//first intersection
				if(intCount == 0){
					firstIntPlineIdx = i;
					firstIntContourIdx = j;
					firstIntPt = tmpIntPt;
				} else if(intCount == 1) {
					secondIntPlineIdx = i;
					secondIntContourIdx = j;
					secondIntPt = tmpIntPt;
				} else {
					//std::cout << "Cannot split - Polyline intersects polygon at more than two points.\n";
					return false;
				}
				intCount++;
			}
		}
	}

	if(intCount != 2){
		//std::cout << "Cannot split - Polyline intersects polygon at " << intCount <<" points\n";
		return false;
	}

	//Once we have intersection points and indexes, we reconstruct the two polygons
	pgon1.clear();
	pgon2.clear();
	int pgonVtxIte;
	int plineVtxIte;

	//If first polygon segment intersected has an index greater
	//	than second segment, modify indexes for correct reconstruction
	if(firstIntContourIdx > secondIntContourIdx){
		secondIntContourIdx += contourSz;
	}

	//==== Reconstruct first polygon
	//-- append polygon contour
	pgon1.push_back(firstIntPt);
	pgonVtxIte = firstIntContourIdx;
	while( pgonVtxIte < secondIntContourIdx){
		pgon1.push_back(contour[(pgonVtxIte+1)%contourSz]);
		pgonVtxIte++;
	}
	pgon1.push_back(secondIntPt);
	//-- append polyline points
	plineVtxIte = secondIntPlineIdx;
	while(plineVtxIte > firstIntPlineIdx){
		pgon1.push_back(pline[(plineVtxIte)]);
		plineVtxIte--;
	}

	//==== Reconstruct second polygon
	//-- append polygon contour
	pgon2.push_back(secondIntPt);
	pgonVtxIte = secondIntContourIdx;
	while( pgonVtxIte < firstIntContourIdx + contourSz){
		pgon2.push_back(contour[(pgonVtxIte+1)%contourSz]);
		pgonVtxIte++;
	}
	pgon2.push_back(firstIntPt);
	//-- append polyline points
	plineVtxIte = firstIntPlineIdx;
	while(plineVtxIte < secondIntPlineIdx){
		pgon2.push_back(pline[(plineVtxIte + 1)]);
		plineVtxIte++;
	}


	//verify that two polygons are created after the split. If not, return false
	/////
	if(pgon1.size() < 3 || pgon2.size() < 3){
		//std::cout << "Invalid split - Resulting polygons have fewer than three vertices.\n";
		return false;
	}

	return polylineIntersectsPolygon;
}

/**
* @brief: Reorient polygon faces so that they are CCW
* @in: If only check is true, the polygon is not modified
* @out: True if polygon had to be reoriented
**/
bool Polygon3D::reorientFace(Loop3D &pface, bool onlyCheck)
{
	int pfaceSz = pface.size();
	int next;
	float tmpSum = 0.0f;

	for(int i=0; i<pfaceSz; ++i){
		next = (i+1)%pfaceSz;
		tmpSum = tmpSum + ( pface[next].x() - pface[i].x() )*( pface[next].y() + pface[i].y() );
	}			

	if(tmpSum > 0.0f)
	{				
		if(!onlyCheck){
			std::reverse(pface.begin(), pface.end());
		}
		return true;
	}
	return false;
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
float Polygon3D::computeInset(float offsetDistance, Loop3D &pgonInset, bool computeArea)
{
	if(contour.size() < 3) return 0.0f;				
	std::vector<float> offsetDistances(contour.size(), offsetDistance);

	return computeInset(offsetDistances, pgonInset, computeArea);
}

//Distance from segment ab to point c
float pointSegmentDistanceXY(QVector3D &a, QVector3D &b, QVector3D &c, QVector3D &closestPtInAB)
{
	float dist;		

	float r_numerator = (c.x()-a.x())*(b.x()-a.x()) + (c.y()-a.y())*(b.y()-a.y());
	float r_denomenator = (b.x()-a.x())*(b.x()-a.x()) + (b.y()-a.y())*(b.y()-a.y());
	float r = r_numerator / r_denomenator;
	//
	float px = a.x() + r*(b.x()-a.x());
	float py = a.y() + r*(b.y()-a.y());
	//    
	float s =  ((a.y()-c.y())*(b.x()-a.x())-(a.x()-c.x())*(b.y()-a.y()) ) / r_denomenator;

	float distanceLine = fabs(s)*sqrt(r_denomenator);

	//
	// (xx,yy) is the point on the lineSegment closest to (cx,cy)
	//
	closestPtInAB.setX(px);
	closestPtInAB.setY(py);
	closestPtInAB.setZ(0.0f);

	if ( (r >= 0) && (r <= 1) )
	{
		dist = distanceLine;
	}
	else
	{
		float dist1 = (c.x()-a.x())*(c.x()-a.x()) + (c.y()-a.y())*(c.y()-a.y());
		float dist2 = (c.x()-b.x())*(c.x()-b.x()) + (c.y()-b.y())*(c.y()-b.y());
		if (dist1 < dist2)
		{	
			dist = sqrt(dist1);
		}
		else
		{
			dist = sqrt(dist2);
		}
	}

	return abs(dist);
}

float pointSegmentDistanceXY(QVector3D &a, QVector3D &b, QVector3D &c)
{
	QVector3D closestPt;
	return pointSegmentDistanceXY(a, b, c, closestPt);

}

//Shortest distance from a point to a polygon
float Polygon3D::distanceXYToPoint(Loop3D &pin, QVector3D &pt)
{
	float minDist = FLT_MAX;
	float dist;
	int idxNext;

	for(size_t i=0; i<pin.size(); ++i){
		idxNext = (i+1)%(pin.size());
		dist = pointSegmentDistanceXY(pin.at(i), pin.at(idxNext), pt);
		if(dist < minDist){
			minDist = dist;
		}
	}
	return minDist;
}

//this function measures the minimum distance from the vertices of a contour A
//	to the edges of a contour B, i.e., it measures the distances from each vertex of A
//  to all the edges in B, and returns the minimum of such distances
float Polygon3D::distanceXYfromContourAVerticesToContourB(Loop3D &pA, Loop3D &pB)
{
	float minDist = FLT_MAX;
	float dist;

	for(size_t i=0; i<pA.size(); ++i){
		dist = Polygon3D::distanceXYToPoint(pB, pA.at(i));
		if(dist < minDist){
			minDist = dist;
		}
	}
	return minDist;
}

/**
* @brief: Merge consecutive vertices that are within a distance threshold to each other
**/
int Polygon3D::cleanLoop(Loop3D &pin, Loop3D &pout, float threshold=1.0f)
{	
	float thresholdSq = threshold*threshold;

	if(pin.size()<3){
		return 1;
	}

	boost::geometry::ring_type<Polygon3D>::type bg_pin;
	boost::geometry::ring_type<Polygon3D>::type bg_pout;
	boost::geometry::assign(bg_pin, pin);
	boost::geometry::correct(bg_pin);
	boost::geometry::simplify(bg_pin, bg_pout, threshold);

	//strategy::simplify::douglas_peucker

	//copy points back
	QVector3D tmpPt;
	for(size_t i=0; i< bg_pout.size(); ++i){						
		tmpPt.setX( bg_pout[i].x() );
		tmpPt.setY( bg_pout[i].y() );
		pout.push_back(tmpPt);						
	}

	//remove last point
	if( (pout[0] - pout[pout.size()-1]).lengthSquared() < thresholdSq ){
		pout.pop_back();				
	}

	//clean angles
	int next, prev;
	QVector3D cur_prev, cur_next;
	float ang;
	float angleThreshold = 0.01f;
	for(size_t i=0; i<pout.size(); ++i){
		next = (i+1)%pout.size();
		prev = (i-1+pout.size())%pout.size();
		cur_prev = pout[prev]-pout[i];
		cur_next = pout[next]-pout[i];

		ang = angleBetweenVectors(cur_prev, cur_next);
		if( (fabs(ang) < angleThreshold) 
			|| (fabs(ang - 3.14159265f ) < angleThreshold)
			|| (!(ang==ang) ) )
		{
			//std::cout << ang << " ";
			pout.erase(pout.begin() + i);
			--i;
		}
	}


	return 0;
}//

/**
* Get polygon oriented bounding box
**/
void Polygon3D::getLoopOBB(Loop3D &pin, QVector3D &size, QMatrix4x4 &xformMat){
	float alpha = 0.0f;			
	float deltaAlpha = 0.05*3.14159265359f;
	float bestAlpha;

	Loop3D rotLoop;
	QMatrix4x4 rotMat;
	QVector3D minPt, maxPt;
	QVector3D origMidPt;
	QVector3D boxSz;
	QVector3D bestBoxSz;
	float curArea;
	float minArea = FLT_MAX;

	rotLoop = pin;
	Polygon3D::getLoopAABB(rotLoop, minPt, maxPt);
	origMidPt = 0.5f*(minPt + maxPt);

	//while(alpha < 0.5f*_PI){
	int cSz = pin.size();
	QVector3D difVec;
	for(int i=0; i<pin.size(); ++i){
		difVec = (pin.at((i+1)%cSz) - pin.at(i)).normalized();
		alpha = atan2(difVec.x(), difVec.y());
		rotMat.setToIdentity();				
		rotMat.rotate(57.2957795f*(alpha), 0.0f, 0.0f, 1.0f);//57.2957795 rad2degree				

		transformLoop(pin, rotLoop, rotMat);
		boxSz = Polygon3D::getLoopAABB(rotLoop, minPt, maxPt);
		curArea = boxSz.x() * boxSz.y();
		if(curArea < minArea){
			minArea = curArea;
			bestAlpha = alpha;
			bestBoxSz = boxSz;
		}
		//alpha += deltaAlpha;
	}

	xformMat.setToIdentity();											
	xformMat.rotate(57.2957795f*(bestAlpha), 0.0f, 0.0f, 1.0f);//57.2957795 rad2degree
	xformMat.setRow(3, QVector4D(origMidPt.x(), origMidPt.y(), origMidPt.z(), 1.0f));			
	size = bestBoxSz;
}//

void Polygon3D::getMyOBB(QVector3D &size, QMatrix4x4 &xformMat){
	Polygon3D::getLoopOBB(this->contour, size, xformMat);
}//

/**
* @brief: Get polygon axis aligned bounding box
* @return: The dimensions of the AABB 
**/
QVector3D Polygon3D::getLoopAABB(Loop3D &pin, QVector3D &minCorner, QVector3D &maxCorner)
{
	maxCorner = QVector3D(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	minCorner = QVector3D( FLT_MAX,  FLT_MAX,  FLT_MAX);

	QVector3D curPt;

	for(int i=0; i<pin.size(); ++i){
		curPt = pin.at(i);
		if( curPt.x() > maxCorner.x() ){
			maxCorner.setX(curPt.x());
		}
		if( curPt.y() > maxCorner.y() ){
			maxCorner.setY(curPt.y());
		}
		if( curPt.z() > maxCorner.z() ){
			maxCorner.setZ(curPt.z());
		}
		//------------
		if( curPt.x() < minCorner.x() ){
			minCorner.setX(curPt.x());
		}
		if( curPt.y() < minCorner.y() ){
			minCorner.setY(curPt.y());
		}
		if( curPt.z() < minCorner.z() ){
			minCorner.setZ(curPt.z());
		}
	}
	return QVector3D(maxCorner - minCorner);
}//

bool Polygon3D::isSelfIntersecting(void){
	boost::geometry::ring_type<Polygon3D>::type bg_pgon;
	boost::geometry::assign(bg_pgon, this->contour);
	boost::geometry::correct(bg_pgon);
	return boost::geometry::intersects(bg_pgon);
}//


