#include "MainWindow.h"
#include <QtGui/QApplication>

int main(int argc, char *argv[]) {
	G::global()["showTerrain"] = true;
	G::global()["showRoads"] = true;
	G::global()["showBlocks"] = true;
	G::global()["showParcels"] = true;
	G::global()["showBuildings"] = true;
	G::global()["showParcels"] = true;
	G::global()["showTrees"] = true;

	G::global()["parcelAreaMean"] = 3600;
	G::global()["parcelAreaDeviation"] = 49;
	G::global()["parcelSplitDeviation"] = 0.19;
	G::global()["parkPercentage"] = 0.2f;
	G::global()["parksRatio"] = 0.05f;

	G::global()["buildingHeightMean"] = 12;
	G::global()["buildingHeightDeviation"] = 90;
	G::global()["buildingBaseAboveGround"] = 3.0f;
	G::global()["meanBuildingFloorHeight"] = 3.0f;
	G::global()["buildingHeightFactor"] = 0.067f;
	G::global()["maxHeightBaseAspectRatio"] = 1.0f;
	G::global()["minBuildingArea"] = 10.0f;

	//G::global()["roadAngleTolerance"] = 1.2566f;
	//G::global()["roadAngleTolerance"] = 1.0f;
	//G::global()["roadAngleTolerance"] = 0.8f;
	//G::global()["roadAngleTolerance"] = 0.52f;

	G::global()["seaLevelForAvenue"] = 0.1f;
	G::global()["seaLevelForStreet"] = 48.9f;
	G::global()["seaLevel"] = 70.0f;//48.9f;

	G::global()["MAX_Z"] = 4500.0f;
	G::global()["shader2D"] = true;

	G::global()["sidewalk_width"] = 2.0f;
	G::global()["parcel_area_mean"] = 2000.0f;
	G::global()["parcel_area_min"] = 1000.0f;
	G::global()["parcel_area_deviation"] = 1.0f;
	G::global()["parcel_split_deviation"] = 0.2f;
	G::global()["parcel_setback_front"] = 5.0f;
	G::global()["parcel_setback_rear"] = 5.0f;
	G::global()["parcel_setback_sides"] = 5.0f;
	G::global()["building_max_frontage"] = 0.0f;
	G::global()["building_max_depth"] = 0.0f;
	G::global()["building_stories_deviation"] = 2.0f;
	G::global()["building_stories_mean"] = 4;
	G::global()["tree_setback"] = 1.0f;

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
