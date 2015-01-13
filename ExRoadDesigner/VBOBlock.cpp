/************************************************************************************************
*		VBO Block Class
*		@author igarciad
************************************************************************************************/

#include "VBOBlock.h"
//#include "../RoadGraph/roadGraphVertex.h"
#include <QVector2D>


/**
* Constructor.
**/

Block::Block()
{		
	//myParcels.m_vertices[0].m_out_edges
	myColor = QVector3D(((float)qrand()/(RAND_MAX)), 0.0f, 0.0f);
	isPark=false;
}

/**
* Destructor.
**/

Block::~Block()
{
	this->clear();
}

void Block::clear(void)
{
	this->blockContour.contour.clear();
	this->blockContourRoadsWidths.clear();
	this->myParcels.clear();
}

/**
* Compute parcel adjacency graph
**/
void Block::computeParcelAdjacencyGraph(void)
{
	parcelGraphVertexIter vi, viEnd;
	parcelGraphVertexIter vOther, vOtherEnd;

	for(boost::tie(vi, viEnd) = boost::vertices(myParcels);
		vi != viEnd; ++vi)
	{
		for(vOther = vi + 1; vOther != viEnd; ++vOther)
		{
			if( myParcels[*vi].intersectsParcel( myParcels[*vOther]) ){
				boost::add_edge(*vi, *vOther, myParcels);
			}
		}				
	}
	computeMyBBox3D();
}

void Block::buildableAreaMock(void)
{	
	parcelGraphVertexIter vi, viEnd;
	Loop3D pContourCpy;

	for(boost::tie(vi, viEnd) = boost::vertices(myParcels);
		vi != viEnd; ++vi)
	{
		std::vector<int> frontEdges;
		std::vector<int> backEdges;

		pContourCpy = myParcels[*vi].parcelContour.contour;
		myParcels[*vi].parcelContour.contour.clear();

		Polygon3D::cleanLoop(pContourCpy, myParcels[*vi].parcelContour.contour, 1.0f);
		Polygon3D::reorientFace(myParcels[*vi].parcelContour.contour);

		findParcelFrontAndBackEdges(*this, myParcels[*vi], frontEdges, backEdges, backEdges);

		float area = myParcels[*vi].computeBuildableArea(3.0f, 0.0f, 0.5f,
			frontEdges, backEdges, backEdges, pContourCpy);

	}
}

void Block::findParcelFrontAndBackEdges(Block &inBlock, Parcel &inParcel,
	std::vector<int> &frontEdges,
	std::vector<int> &rearEdges,
	std::vector<int> &sideEdges )
{
	QVector3D midPt;
	typedef boost::geometry::model::d2::point_xy<double> point_2d;
	point_2d bg_midPt;
	boost::geometry::model::polygon<point_2d> bg_pgon;

	float dist;
	float minDist;
	const float tol = 0.01f;
	int next;

	frontEdges.clear();
	rearEdges.clear();
	sideEdges.clear();

	float distPtThis;
	float distPtNext;
	float distPtMid;
	float kDistTol = 0.01f;

	for(int i=0; i<inParcel.parcelContour.contour.size(); ++i)
	{
		next = ((i+1))%inParcel.parcelContour.contour.size();

		midPt = 0.5f*(inParcel.parcelContour.contour.at(i) +
			inParcel.parcelContour.contour.at(next));

		distPtThis = Polygon3D::distanceXYToPoint( inBlock.blockContour.contour, 
			inParcel.parcelContour.contour.at(i) );

		distPtNext = Polygon3D::distanceXYToPoint( inBlock.blockContour.contour, 
			inParcel.parcelContour.contour.at(next) );

		distPtMid = Polygon3D::distanceXYToPoint( inBlock.blockContour.contour, 
			midPt );

		int numPtsThatAreClose =
			(int)(distPtThis < kDistTol) + (int)(distPtMid < kDistTol) + (int)(distPtNext < kDistTol);

		bool midPtIsClose = (distPtThis < kDistTol);

		switch(numPtsThatAreClose){
			//if neither one is close to block boundary, then segment is a rear segment
		case 0:
			rearEdges.push_back(i);
			break;
			//if only one between this and next is close to block boundary, then segment is a side segment
		case 1:					
		case 2:
			sideEdges.push_back(i);
			break;
			//if this or next are very close to block boundary, then segment is a front segment
		case 3:
			frontEdges.push_back(i);
			break;
		}

	}

}


/**
* Are parcels adjacent in block parcel graph
**/
bool Block::areParcelsAdjacent(parcelGraphVertexIter &p0, parcelGraphVertexIter &p1)
{
	std::pair<Block::parcelGraphAdjIter,Block::parcelGraphAdjIter> p=boost::adjacent_vertices(*p0, this->myParcels);

	parcelGraphAdjIter vi;

	for(vi=p.first;vi!=p.second;vi++){
		if(*vi == *p1){
			return true;
		}
	}
	return false;

}

float loopArea(Loop3D &loop)
{
	float area = 0.0f;

	//We will triangulate the polygon
	//into triangles with points p[0],p[i],p[i+1]
	for(int i = 1; i+1<loop.size(); ++i){
		float x1 = loop[i].x() - loop[0].x();
		float y1 = loop[i].y() - loop[0].y();
		float x2 = loop[i+1].x() - loop[0].x();
		float y2 = loop[i+1].y() - loop[0].y();
		float cross = x1*y2 - x2*y1;
		area += cross;
	}
	return (area/2.0f);
}

/**
* Adapt block to vboRenderManager
**/
/*void Block::adaptBlockToTerrain(MTC::geometry::ElevationGrid *elGrid)
{
Block::parcelGraphVertexIter vi, viEnd;

for(boost::tie(vi, viEnd) = boost::vertices(myParcels);
vi != viEnd; ++vi)
{
myParcels[*vi].adaptParcelToTerrain(elGrid);
}
computeMyBBox3D();
}*/

void getRoadSegmentGeoRightAndLeft(std::vector<QVector3D> &roadSegGeo,
	std::vector<QVector3D> &roadSegGeoRight, std::vector<QVector3D> &roadSegGeoLeft, float width)
{
	QVector3D ptR;
	QVector3D ptL;
	QVector2D dirVec;

	for(int i=0; i<roadSegGeo.size(); ++i){
		if( (i!=0) && (i!=roadSegGeo.size()-1) ){
			dirVec = QVector2D(roadSegGeo[i+1] - roadSegGeo[i-1]);
		} else if(i==0){
			dirVec = QVector2D(roadSegGeo[i+1] - roadSegGeo[i]);
		} else {
			dirVec = QVector2D(roadSegGeo[i] - roadSegGeo[i-1]);
		}
		QVector2D dirVecPerp(dirVec.y(), -dirVec.x());
		dirVecPerp.normalize();
		ptR = roadSegGeo[i] + 0.5f*width*dirVecPerp;
		ptL = roadSegGeo[i] - 0.5f*width*dirVecPerp;
		roadSegGeoRight.push_back(ptR);
		roadSegGeoLeft.push_back(ptL);
	}
}


void Block::computeMyBBox3D(void)
{	
	this->bbox.resetMe();

	parcelGraphVertexIter vi, viEnd;
	for(boost::tie(vi, viEnd) = boost::vertices(myParcels); vi != viEnd; ++vi)
	{
		this->bbox.combineWithBBox3D(myParcels[*vi].bbox);
	}
}

