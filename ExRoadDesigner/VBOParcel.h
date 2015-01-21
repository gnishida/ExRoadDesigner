/************************************************************************************************
*		VBO Parcel
*		@author igarciad
************************************************************************************************/
#pragma once

#include <QSettings>
#include "VBOBuilding.h"
#include <QTextStream>
#include "Polygon3D.h"

class Parcel {
public:
	Polygon3D parcelContour;
	Polygon3D parcelBuildableAreaContour;

	bool isPark;
	Building myBuilding;
	
	//BBox3D bbox;

public:
	Parcel();

	Parcel(const Parcel &ref){					
		parcelContour = ref.parcelContour;
		parcelBuildableAreaContour = ref.parcelBuildableAreaContour;
		myBuilding = ref.myBuilding;
		//bbox = ref.bbox;
		isPark = ref.isPark;
		initializeParcel();
	}
	
	inline Parcel &operator=(const Parcel &ref) {
		parcelContour = ref.parcelContour;
		parcelBuildableAreaContour = ref.parcelBuildableAreaContour;
		myBuilding = ref.myBuilding;				

		//bbox = ref.bbox;
		initializeParcel();
		isPark = ref.isPark;
		initializeParcel();
		return (*this);
	}

	inline void setContour(Polygon3D &inContour) {
		this->parcelContour = inContour;
		initializeParcel();
	}

	void initializeParcel() {
		if(parcelContour.contour.size()>0){
			boost::geometry::correct(parcelContour.contour);
		}

		//parcelContour.getBBox3D(bbox.minPt, bbox.maxPt);
	}

	float computeBuildableArea(float frontSetback, float rearSetback, float sideSetback, std::vector<int> &frontEdges, std::vector<int> &rearEdges, std::vector<int> &sideEdges, Loop3D &pgonInset);
};

