#include <QMessageBox>
#include <common/global.h>
#include <road/GraphUtil.h>
#include <road/feature/ExFeatureExtractor.h>
#include "ControlWidget.h"
#include "MainWindow.h"
#include "GLWidget.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.checkBoxRoadTypeHighway->setChecked(true);
	ui.checkBoxRoadTypeBoulevard->setChecked(true);
	ui.checkBoxRoadTypeAvenue->setChecked(true);
	ui.checkBoxRoadTypeLocalStreet->setChecked(true);

	// register the event handlers
	connect(ui.checkBoxRoadTypeHighway, SIGNAL(stateChanged(int)), this, SLOT(showRoad(int)));
	connect(ui.checkBoxRoadTypeBoulevard, SIGNAL(stateChanged(int)), this, SLOT(showRoad(int)));
	connect(ui.checkBoxRoadTypeAvenue, SIGNAL(stateChanged(int)), this, SLOT(showRoad(int)));
	connect(ui.checkBoxRoadTypeLocalStreet, SIGNAL(stateChanged(int)), this, SLOT(showRoad(int)));
	connect(ui.pushButtonExtractFeature, SIGNAL(clicked()), this, SLOT(extractFeature()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void ControlWidget::showRoad(int flag) {
	mainWin->glWidget->roads.showHighways = ui.checkBoxRoadTypeHighway->isChecked();
	mainWin->glWidget->roads.showBoulevards = ui.checkBoxRoadTypeBoulevard->isChecked();
	mainWin->glWidget->roads.showAvenues = ui.checkBoxRoadTypeAvenue->isChecked();
	mainWin->glWidget->roads.showLocalStreets = ui.checkBoxRoadTypeLocalStreet->isChecked();
	mainWin->glWidget->roads.setModified();

	mainWin->glWidget->updateGL();
}

/**
 * Event handler for button [Detect Grid]
 */
void ControlWidget::detectGrid() {
	/*
	int roadType = (ui.checkBoxRoadTypeAvenue->isChecked() ? 2 : 0) + (ui.checkBoxRoadTypeLocalStreet->isChecked() ? 1 : 0);

	int maxIteration = ui.lineEditGridMaxIteration->text().toInt();
	int numBins = ui.lineEditNumBins->text().toInt();
	float minTotalLength = ui.lineEditMinTotalLength->text().toFloat();
	float minMaxBinRatio = ui.lineEditMinMaxBinRatio->text().toFloat();
	float angleThreshold = ui.lineEditGridAngleThreshold->text().toFloat();
	float votingThreshold = ui.lineEditGridVotingThreshold->text().toFloat();
	float extendingDistanceThreshold = ui.lineEditGridExtendingDistanceThreshold->text().toFloat();
	float minOBBLength = ui.lineEditGridMinOBBLength->text().toFloat();

	GraphUtil::copyRoads(mainWin->glWidget->origRoads, mainWin->glWidget->roads);
	RoadSegmentationUtil::detectGrid(mainWin->glWidget->roads, mainWin->glWidget->selectedArea, roadType, mainWin->glWidget->roadFeature, maxIteration, numBins, minTotalLength, minMaxBinRatio, angleThreshold, votingThreshold, extendingDistanceThreshold, minOBBLength);

	mainWin->glWidget->roadFeature.normalize();
	mainWin->glWidget->roadFeature.save("grid_feature1.xml");

	mainWin->glWidget->updateGL();
	*/
}

/**
 * Event handler for button [Detect Radial]
 */
void ControlWidget::detectRadial() {
	/*
	int roadType = (ui.checkBoxRoadTypeAvenue->isChecked() ? 2 : 0) + (ui.checkBoxRoadTypeLocalStreet->isChecked() ? 1 : 0);

	int maxIteration = ui.lineEditRadialMaxIteration->text().toInt();
	float scale1 = ui.lineEditScale1->text().toFloat();
	float scale2 = ui.lineEditScale2->text().toFloat();
	float centerErrorTol2 = ui.lineEditCenterErrorTol2->text().toFloat();
	float angleThreshold2 = ui.lineEditAngleThreshold2->text().toFloat();
	float scale3 = ui.lineEditScale3->text().toFloat();
	float centerErrorTol3 = ui.lineEditCenterErrorTol3->text().toFloat();
	float angleThreshold3 = ui.lineEditAngleThreshold3->text().toFloat();
	float votingThreshold = ui.lineEditRadialVotingThreshold->text().toFloat();
	float seedDistance = ui.lineEditRadialSeedDistance->text().toFloat();
	float minSeedDirection = ui.lineEditRadialMinSeedDirections->text().toFloat();
	float extendingAngleThreshold = ui.lineEditRadialExtendingAngleThreshold->text().toFloat();

	GraphUtil::copyRoads(mainWin->glWidget->origRoads, mainWin->glWidget->roads);
	RoadSegmentationUtil::detectRadial(mainWin->glWidget->roads, mainWin->glWidget->selectedArea, roadType, mainWin->glWidget->roadFeature, maxIteration, scale1, scale2, centerErrorTol2, angleThreshold2, scale3, centerErrorTol3, angleThreshold3, 150.0f, votingThreshold, seedDistance, minSeedDirection, extendingAngleThreshold);

	mainWin->glWidget->roadFeature.normalize();
	mainWin->glWidget->roadFeature.save("radial_feature.xml");

	mainWin->glWidget->updateGL();
	*/
}

void ControlWidget::extractFeature() {
	ExFeatureExtractor::extractFeature(mainWin->glWidget->roads, mainWin->glWidget->selectedArea, mainWin->glWidget->hintLine, mainWin->glWidget->roadFeature);

	QString filename = QFileDialog::getSaveFileName(this, tr("Save feature file..."), "", tr("Feature Files (*.xml)"));

	if (filename.isEmpty()) {
		return;
	}

	mainWin->glWidget->roadFeature.save(filename);
}

