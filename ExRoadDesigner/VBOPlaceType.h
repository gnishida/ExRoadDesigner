/************************************************************************************************
*		Place Type Description
*		@author igarciad
************************************************************************************************/
#pragma once

#include <QVariant>
#include <QSettings>
#include <qfile>
#include <qvector3d>
#include <vector>
#include <qvariant.h>

#include <QTextStream>
#include "polygon_3D.h"
#include "global.h"

#include <boost/geometry/geometries/ring.hpp>


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

/**
* PlaceType class contains a place type instance in the city.
**/
class PlaceType
{
public:
	PlaceType(void);
	~PlaceType(void);

	void initializePlaceType(void);

	PlaceType(const PlaceType &ref)
	{	
		attributes = ref.attributes;		
		initializePlaceType();		
	}
	inline PlaceType &operator=(const PlaceType &ref)
	{	
		attributes = ref.attributes;
		initializePlaceType();		
		return (*this);
	}

	void savePlaceType( QTextStream &stream );
	bool loadPlaceType( QString &line, int versionNumber);

	QVector3D getExternalHandlerPos(void);
	QVector3D getExternalHandler2Pos(void);
	QVector3D getlengthUHandlerPos(void);
	QVector3D getlengthVHandlerPos(void);

	static const int kNumPTAttributes = 27;

	//returns true if bounding rectangle contains testPt
	bool containsPoint (QVector3D &testPt);


	//bounding rectangle
	Loop3D boundingRectangle;
	boost::geometry::ring_type<Polygon3D>::type bg_boudingRectangle;

	void updateBoundingRectangle(void);

	QVariant operator [](QString i) const    {return attributes[i];}
	QVariant & operator [](QString i) {return attributes[i];}

	QVector3D getQVector3D(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return QVector3D();}
		return attributes[i].value<QVector3D>();}
	float getFloat(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return 0;}
		return attributes[i].toFloat();}
	float getInt(QString i){
		if(!attributes.contains(i)){printf("PlaceType does not contain type %s\n",i.toAscii().constData());return 0;}
		return attributes[i].toInt();}

private:
	//attributes
	QHash<QString, QVariant> attributes;



};

//////////////////////////////////////////////////
////////////////////////////////////////////////
/**
* Main container class for place types
**/
class PlaceTypesMainClass
{
public:
	PlaceTypesMainClass(){
	}

	~PlaceTypesMainClass(){
	}

	/**
	* Copy constructor.
	**/
	PlaceTypesMainClass(const PlaceTypesMainClass &ref)
	{	
		myPlaceTypes = ref.myPlaceTypes;
	}

	/**
	* Assignment operator.
	**/
	inline PlaceTypesMainClass &operator=(const PlaceTypesMainClass &ref)
	{	
		myPlaceTypes = ref.myPlaceTypes;
		return (*this);
	}

	bool savePlaceTypeInstancesToFile ( QString filename );
	int loadPlaceTypeInstancesFromFile ( QString filename );
	int loadPlaceTypeInstancesFromFile ( QString filename, Polygon3D &pgon );

	std::vector<PlaceType> myPlaceTypes;

	int addPlaceType(int ptIdx);
	int removePlaceType(int ptIdx);
	int sendPlaceTypeToTop(int ptIdx);
	int sendPlaceTypeOneUp(int ptIdx);
	int sendPlaceTypeOneDown(int ptIdx);
	int sendPlaceTypeToBottom(int ptIdx);	

	static const int kVersionNumber = 4;
};





