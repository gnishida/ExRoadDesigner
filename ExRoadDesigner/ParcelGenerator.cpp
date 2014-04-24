/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "ParcelGenerator.h"
#include <common/Polygon3D.h>
#include <common/global.h>
#include <pm/Block.h>
#include <pm/Parcel.h>
#include <pm/LandUse.h>
#include <common/Util.h>
#include "UrbanGeometry.h"
#include <QtTest/qtest.h>
#include <QMatrix4x4>

ParcelGenerator::ParcelGenerator(MainWindow* mainWin) {
	this->mainWin = mainWin;
}

ParcelGenerator::~ParcelGenerator() {
}

void ParcelGenerator::run() {
	std::vector<Block*>& blocks = mainWin->urbanGeometry->blocks;

	int count = 0;
	for (int i = 0; i < blocks.size(); ++i) {
		subdivideBlockIntoParcels(blocks[i]);
	}
}

void ParcelGenerator::subdivideBlockIntoParcels(Block* block) {	
	std::vector<Parcel*> tmpParcels;

	block->clearParcels();

	subdivideParcel(block, block->getContour(), G::getFloat("parcelAreaMean"), G::getFloat("parcelAreaDeviation")/100.0f, G::getFloat("parcelSplitDeviation"), tmpParcels);

	for (int i = 0; i < tmpParcels.size(); ++i) {
		block->addParcel(tmpParcels[i]);
	}
}

/**
* Parcel subdivision
* @desc: Recursive subdivision of a parcel using OBB technique
* @return: true if parcel was successfully subdivided, false otherwise
* @areaMean: mean target area of parcels after subdivision
* @areaVar: variance of parcels area after subdivision (normalized in 0-1)
* @splitIrregularity: A normalized value 0-1 indicating how far
*					from the middle point the split line should be
**/
bool ParcelGenerator::subdivideParcel(Block* block, Polygon3D& contour, float areaMean, float areaStd, float splitIrregularity, std::vector<Parcel*>& outParcels) {
	float thresholdArea = areaMean + Util::genRand(-1.0f, 1.0f) * areaStd * areaMean;

	float area = contour.area();

	if (contour.area() < thresholdArea) {
		QVector3D center = contour.centroid();

		Parcel* parcel = new Parcel(block, contour);

		/*if (parcUtil::genRand(0, 1) < G::getFloat("parksRatio")) {
			parcel->setLandUse(LandUse::kLUPark);		// Park
		} else*/ if (300 < 100 + Util::genRand(0, 200)) {
			parcel->setLandUse(LandUse::kLUResS);		// Residential area
		} else if (300 < 600 + Util::genRand(0, 200)) {
			parcel->setLandUse(LandUse::kLUResM);	// Residential area
		} else {
			parcel->setLandUse(LandUse::kLUResL);	// Residential
		}

		outParcels.push_back(parcel);
		return false;
	}

	//compute OBB
	QVector3D obbSize;
	QMatrix4x4 obbMat;
	//contour.getMyOBB(obbSize, obbMat);
	Polygon3D::getLoopOBB(contour, obbSize, obbMat);

	//compute split line passing through center of OBB TODO (+/- irregularity)
	//		and with direction parallel/perpendicular to OBB main axis
	QVector3D slEndPoint;
	QVector3D dirVectorInit, dirVector, dirVectorOrthogonal;
	QVector3D midPt(0.0f, 0.0f, 0.0f);
	QVector3D auxPt(1.0f, 0.0f, 0.0f);
	QVector3D midPtNoise(0.0f, 0.0f, 0.0f);
	Polyline3D splitLine;	

	midPt = midPt * obbMat;

	dirVectorInit = auxPt * obbMat - midPt;
	dirVectorInit.normalize();
	if (obbSize.x() > obbSize.y()) {
		dirVector.setX( -dirVectorInit.y() );
		dirVector.setY(  dirVectorInit.x() );
	} else {
		dirVector = dirVectorInit;
	}

	midPtNoise.setX( splitIrregularity * Util::genRand(-10.0f, 10.0f) );
	midPtNoise.setY( splitIrregularity * Util::genRand(-10.0f, 10.0f) );
	midPt = midPt + midPtNoise;

	slEndPoint = midPt + 10000.0f * dirVector;
	splitLine.push_back(slEndPoint);
	slEndPoint = midPt - 10000.0f * dirVector;
	splitLine.push_back(slEndPoint);

	//split parcel with line and obtain two new parcels
	Polygon3D pgon1, pgon2;

	float kDistTol = 0.01f;

	if (contour.splitMeWithPolyline(splitLine, pgon1, pgon2)) {
		//check if new contours of pgon1 and pgon2 "touch" the boundary of the block
		if (pgon1.distanceXYfromContourAVerticesToContourB(block->getContour()) > kDistTol 
			|| pgon2.distanceXYfromContourAVerticesToContourB(block->getContour()) > kDistTol) {
			splitLine.clear();
			pgon1.clear();
			pgon2.clear();

			//if they don't have street access, rotate split line by 90 degrees and recompute
			dirVectorOrthogonal.setX(-dirVector.y());
			dirVectorOrthogonal.setY(dirVector.x());

			slEndPoint = midPt + 10000.0f * dirVectorOrthogonal;
			splitLine.push_back(slEndPoint);
			slEndPoint = midPt - 10000.0f * dirVectorOrthogonal;
			splitLine.push_back(slEndPoint);			

			contour.splitMeWithPolyline(splitLine, pgon1, pgon2);
		}

		subdivideParcel(block, pgon1, areaMean, areaStd, splitIrregularity, outParcels);
		subdivideParcel(block, pgon2, areaMean, areaStd, splitIrregularity, outParcels);
	} else {
		return false;
	}

	return true;
}

/*
void ParcelGenerator::setParcelsAsParks(std::vector<Block*>& blocks) {
	std::vector<Parcel*> parcelPtrs;

	bool isFirst = true;
	int seedOfFirstBlock = 0;

	//get all the parcels of that place type in an array
	for (int j = 0; j < blocks.size(); ++j) {
		ParcelGraphVertexIter vi, viEnd;
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[j]->getParcels()); vi != viEnd; ++vi) {
			//blocks[j].getParcel(*vi).getBuildingAttributes().att_is_park = false;
			parcelPtrs.push_back(blocks[j]->getParcel(*vi));
		}
	}

	srand(seedOfFirstBlock);

	float parkPercentage = G::getFloat("parkPercentage");

	//shuffle and select first parkPercentage %
	int numToSetAsParks = (int)(parkPercentage * (float)parcelPtrs.size());
	std::random_shuffle( parcelPtrs.begin(), parcelPtrs.end() );

	int countMax = std::min<float>( parcelPtrs.size(), numToSetAsParks );
	for (int i = 0; i < countMax ; ++i) {
		parcelPtrs[i]->setLandUse(LandUse::kLUPark);
	}
}
*/

