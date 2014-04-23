/************************************************************************************************
*		VBO Building Class
*		@author igarciad
************************************************************************************************/
#pragma once

#include <QSettings>
#include "Polygon3D.h"

const int BLDG_NO_BLDG=0;// 0 NO BLDG
const int BLDG_WITH_BLDG=1;	// 1 BLDG


class Building{

public:

	Building(void){
		bldType=-1;
		numStories=-1;
	}

	/**
	* Destructor
	**/
	~Building(void){
	}

	/**
	* Copy Constructor
	**/
	Building(const Building &ref)
	{					
		bldType = ref.bldType;
		numStories = ref.numStories;
		buildingFootprint = ref.buildingFootprint;
	}

	/**
	* Assignment operator.
	**/
	inline Building &operator=(const Building &ref)
	{	
		bldType = ref.bldType;
		numStories = ref.numStories;
		buildingFootprint = ref.buildingFootprint;
		return (*this);
	}

	/**
	* Data members
	**/
	Polygon3D buildingFootprint;
	int bldType;
	int numStories;

};
