/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPm.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>
#include <QTime>

#include "VBOPmBlocks.h"
#include "VBOPmParcels.h"
#include "VBOPmBuildings.h"
#include "BlockSet.h"
#include "VBOGeoBuilding.h"
#include "VBOVegetation.h"
#include "Polygon3D.h"
#include "Util.h"

/**
 * 道路網から、ブロック情報を抽出する。
 */
bool VBOPm::generateBlocks(VBORenderManager& rendManager,RoadGraph &roadGraph, BlockSet& blocks) {
	if (!VBOPmBlocks::generateBlocks(roadGraph, blocks)) {
		printf("ERROR: generateBlocks\n");
		return false;
	}

	return true;
}

/**
 * Block情報から、Parcel情報を計算する。
 */
bool VBOPm::generateParcels(VBORenderManager& rendManager, BlockSet& blocks) {
	if (!VBOPmParcels::generateParcels(rendManager, blocks.blocks)) {
		printf("ERROR: generateParcels\n");
		return false;
	}

	// ビルのfootprintを計算する
	if (!VBOPmBuildings::generateBuildings(rendManager, blocks.blocks)) {
		printf("ERROR: generateBuildings\n");
		return false;
	}
		
	return true;
}

bool VBOPm::generateBuildings(VBORenderManager& rendManager, BlockSet& blocks) {
	rendManager.removeStaticGeometry("3d_building");
		
	Block::parcelGraphVertexIter vi, viEnd;
	for (int bN = 0; bN < blocks.size(); bN++) {
		if (blocks[bN].isPark) continue;

		for (boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi) {
			if (blocks[bN].myParcels[*vi].isPark) continue;
			if (blocks[bN].myParcels[*vi].myBuilding.buildingFootprint.contour.size() < 3) continue;

			float c = rand() % 192;
			blocks[bN].myParcels[*vi].myBuilding.color = QColor(c, c, c);

			int building_type = 1;//placeTypes.myPlaceTypes[blocks[bN].getMyPlaceTypeIdx()].getInt("building_type");
			VBOGeoBuilding::generateBuilding(rendManager,blocks[bN].myParcels[*vi].myBuilding, building_type);				
		}
	}
	printf("Building generation is done.\n");

	return true;
}

bool VBOPm::generateVegetation(VBORenderManager& rendManager, BlockSet& blocks) {
	VBOVegetation::generateVegetation(rendManager, blocks.blocks);

	return true;
}
