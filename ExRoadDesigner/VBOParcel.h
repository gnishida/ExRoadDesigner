/************************************************************************************************
*		VBO Parcel
*		@author igarciad
************************************************************************************************/
#pragma once

#include <QSettings>
#include "VBOBuilding.h"
#include <QTextStream>
#include "Polygon3D.h"

const int PAR_PARK=0;
const int PAR_WITH_BLDG=1;

/**
* Parcel.
**/
class Parcel{

public:

	/**
	* Interface
	**/
	Polygon3D parcelContour;
	Polygon3D parcelBuildableAreaContour;

	bool isPark;
	Building myBuilding;

	/**
	* Methods
	**/

	Parcel();
	~Parcel();

	Parcel(const Parcel &ref){					
		parcelContour = ref.parcelContour;
		parcelBuildableAreaContour = ref.parcelBuildableAreaContour;
		myBuilding = ref.myBuilding;
		bbox = ref.bbox;
		myPlaceTypeIdx = ref.myPlaceTypeIdx;
		initializeParcel();
	}


	inline Parcel &operator=(const Parcel &ref){	
		parcelContour = ref.parcelContour;
		parcelBuildableAreaContour = ref.parcelBuildableAreaContour;
		myBuilding = ref.myBuilding;				

		bbox = ref.bbox;
		initializeParcel();
		myPlaceTypeIdx = ref.myPlaceTypeIdx;
		initializeParcel();
		return (*this);
	}

	/**
	* Set Contour
	**/
	inline void setContour(Polygon3D &inContour)
	{
		this->parcelContour = inContour;
		if(parcelContour.contour.size()>0){
			boost::geometry::assign(bg_parcelContour, parcelContour.contour);
		}
		initializeParcel();
	}

	/**
	* Initialize parcel
	**/
	void initializeParcel()
	{
		if(parcelContour.contour.size()>0){
			boost::geometry::assign(bg_parcelContour, parcelContour.contour);
			boost::geometry::correct(parcelContour.contour);
		}
		/*QVector3D minPt,maxPt;

		parcelContour.getBBox3D(minPt, maxPt);
		bbox.minPt=minPt.toVector2D();
		bbox.maxPt=maxPt.toVector2D();*/
		parcelContour.getBBox3D(bbox.minPt, bbox.maxPt);
	}


	/**
	* @brief: Returns true if parcels are adjacent			
	**/
	bool intersectsParcel(Parcel &other);

	/**			
	* @brief: Compute union of this parcel with a given parcel. The contour of the current parcel is modified. The other parcel is left unchanged.
	* @param[in] other: Parcel to union this parcel with.
	* @param[out] bool: True if union was computed. False if union fails. Union fails if parcels are not adjacent.
	**/
	int unionWithParcel(Parcel &other);

	/**
	* Export parcel contour to OBJ
	**/
	void writeParcelContourToOBJ(QTextStream &objStream);

	/**
	* Export parcel building to OBJ
	**/
	void writeParcelBuildingToOBJ(QTextStream &objStream);


	/**
	* Compute Parcel Buildable Area
	**/
	float computeBuildableArea(float frontSetback, float rearSetback, float sideSetback,
		std::vector<int> &frontEdges, 
		std::vector<int> &rearEdges, 
		std::vector<int> &sideEdges,
		Loop3D &pgonInset);


	inline void setMyPlaceTypeIdx(int inIdx){
		myPlaceTypeIdx = inIdx;
	}

	inline int getMyPlaceTypeIdx(void){
		return myPlaceTypeIdx;
	}

	BBox3D bbox;
	boost::geometry::ring_type<Polygon3D>::type bg_parcelContour;
private:	
	int myPlaceTypeIdx;

};

