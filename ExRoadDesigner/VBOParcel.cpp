/************************************************************************************************
*		VBO Parcel
*		@author igarciad
************************************************************************************************/
#include "VBOParcel.h"


Parcel::Parcel(){
	myPlaceTypeIdx = -1;
	isPark = false;
}


Parcel::~Parcel(){
}

bool Parcel::intersectsParcel(Parcel &other)
{
	if(this->bbox.overlapsWithBBox3DXY(other.bbox))
	{
		if(boost::geometry::intersects(this->bg_parcelContour, other.bg_parcelContour)){
			return true;
		}					
	}
	return false;
}

int Parcel::unionWithParcel(Parcel &other)
{
	if(!(this->intersectsParcel(other))){
		return 0;
	}
	typedef boost::geometry::model::d2::point_xy<double> point_2d;
	typedef boost::geometry::model::polygon<point_2d> polygon_2d;


	typedef std::vector<polygon_2d> polygon_list;
	polygon_list pl;
	boost::geometry::union_<boost::geometry::ring_type<Polygon3D>::type,boost::geometry::ring_type<Polygon3D>::type>(this->bg_parcelContour, other.bg_parcelContour, pl);//std::back_inserter(pl));///!!!!!!!!!

	if(pl.size()!=1){
		return 0;
	} else {
		Polygon3D newContour;
		QVector3D tmpPt;
		for(int i=0; i< (pl[0].outer()).size(); ++i){						
			tmpPt.setX( (pl[0].outer())[i].x() );
			tmpPt.setY( (pl[0].outer())[i].y() );
			newContour.contour.push_back(tmpPt);						
		}

		this->setContour(newContour);
		this->initializeParcel();
	}
	return 1;
}

/**
* Compute Parcel Buildable Area
**/
float Parcel::computeBuildableArea(float frontSetback, float rearSetback, float sideSetback,
	std::vector<int> &frontEdges, 
	std::vector<int> &rearEdges, 
	std::vector<int> &sideEdges,
	Loop3D &pgonInset)
{
	float buildableArea = 0.0f;

	int contourSz = this->parcelContour.contour.size();

	if(contourSz < 3){
		return 0.0f;
	}

	//get parcel back edge(s)

	//put together offset values to pass to irregular offset function

	//--- first, initialize values to side setback (most edges are sides)
	std::vector<float> offsetValues(contourSz, sideSetback);

	//--- then, append front ant back values
	for(int i=0; i<frontEdges.size(); ++i){
		if(frontEdges[i]<offsetValues.size()){
			offsetValues[frontEdges[i]] = frontSetback;
		}
	}

	for(int i=0; i<rearEdges.size(); ++i){
		if(rearEdges[i]<offsetValues.size()){
			offsetValues[rearEdges[i]] = rearSetback;
		}
	}

	for(int i=0; i<sideEdges.size(); ++i){
		if(sideEdges[i]<offsetValues.size()){
			offsetValues[sideEdges[i]] = sideSetback;
		}
	}

	//compute irregular offset and the inset area
	buildableArea = parcelContour.computeInset(offsetValues, pgonInset);
	//std::cout << buildableArea << " ";

	if(buildableArea < 0)
		buildableArea = 0;

	return buildableArea;
}

/**
* Export parcel contour to OBJ
**/
void Parcel::writeParcelContourToOBJ(QTextStream &objStream)
{
	std::vector<QVector3D> points = this->parcelContour.contour;
	int size = points.size();

	objStream << "g " << "Lot" << "\n";

	// write vertices of polygon to file
	for(int i=0; i<size; i++)
	{
		objStream << "v " << points[i].x() << " ";
		// z and y axes must be switched, y flipped to match CityEngine axes
		objStream << points[i].z() << " ";
		objStream << (points[i].y()*-1.0) << "\n";
	}

	// write face for this polygon to file
	if(size > 0)
	{
		objStream << "f ";

		for(int i=size; i>=1; i--)
			objStream << "-" << i << " ";
		//Need to add in vertex normal values
	}
	objStream << "\n";

}

/**
* Export parcel buildings to OBJ
**/
void Parcel::writeParcelBuildingToOBJ(QTextStream &objStream)
{
	//std::vector<QVector3D> points = this->parcelContour.contour;
	std::vector<QVector3D> points = this->parcelBuildableAreaContour.contour;
	int size = points.size();

	int height = this->myBuilding.numStories;
	//For larger numbers, round to nearest 5, cut off at 200
	if(height>10)
	{
		height = height - (height % 5);
		if(height > 200)
			height = 200;
	}


	objStream << "g " << "Lot" << height << "\n";

	// write vertices of polygon to file
	for(int i=0; i<size; i++)
	{	
		objStream << "v " << points[i].x() << " ";
		// z and y axes must be switched, y flipped to match CityEngine axes
		objStream << points[i].z() << " ";
		objStream << (points[i].y()*-1.0) << "\n";
	}

	// write face for this polygon to file
	if(size > 0)
	{
		objStream << "f ";

		for(int i=size; i>=1; i--)
			objStream << "-" << i << " ";


		//Need to add in vertex normal values
	}
	objStream << "\n";

}//

