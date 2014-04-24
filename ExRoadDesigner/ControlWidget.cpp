#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "UrbanGeometry.h"
#include "GLWidget3D.h"
#include "global.h"
#include "GraphUtil.h"
#include "ExFeature.h"


ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditNumAvenueIterations->setText("1000");
	ui.lineEditNumStreetIterations->setText("5000");
	ui.horizontalSliderInterpolationFactor->setMinimum(0);
	ui.horizontalSliderInterpolationFactor->setMaximum(100);
	ui.checkBoxCleanAvenues->setChecked(true);
	ui.checkBoxCleanStreets->setChecked(true);
	ui.checkBoxLocalStreets->setChecked(true);
	ui.checkBoxCropping->setChecked(false);
	ui.checkBoxAnimation->setChecked(false);
	ui.checkBoxFadeOut->setChecked(false);

	// register the event handlers
	connect(ui.horizontalSliderInterpolationFactor, SIGNAL(valueChanged(int)), this, SLOT(updateInterpolationFactor(int)));
	connect(ui.pushButtonGenerateMultiEx, SIGNAL(clicked()), this, SLOT(generateRoadsMultiEx()));
	connect(ui.pushButtonGenerateMultiIntEx, SIGNAL(clicked()), this, SLOT(generateRoadsMultiIntEx()));
	connect(ui.pushButtonGenerateInterpolation, SIGNAL(clicked()), this, SLOT(generateRoadsInterpolation()));
	connect(ui.pushButtonGenerateWarp, SIGNAL(clicked()), this, SLOT(generateRoadsWarp()));
	connect(ui.pushButtonGenerateSmoothWarp, SIGNAL(clicked()), this, SLOT(generateRoadsSmoothWarp()));
	connect(ui.pushButtonGenerateVerySmoothWarp, SIGNAL(clicked()), this, SLOT(generateRoadsVerySmoothWarp()));
	connect(ui.pushButtonClear, SIGNAL(clicked()), this, SLOT(clear()));
	connect(ui.pushButtonConnect, SIGNAL(clicked()), this, SLOT(connectRoads()));
	connect(ui.pushButtonMerge, SIGNAL(clicked()), this, SLOT(mergeRoads()));
	//connect(ui.pushButtonCamera3D, SIGNAL(clicked()), this, SLOT(camera3D()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void ControlWidget::updateInterpolationFactor(int value) {
	QString str;
	str.setNum(value);
	ui.labelInterpolationFactor->setText(str);
}

/**
 * Event handler for button [Generate Roads]
 */
void ControlWidget::generateRoadsMultiEx() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();
	G::global()["fadeOut"] = ui.checkBoxFadeOut->isChecked();

	std::vector<ExFeature> features;
	features.resize(mainWin->urbanGeometry->areas.selectedArea()->hintLine.size());
	for (int i = 0; i < mainWin->urbanGeometry->areas.selectedArea()->hintLine.size(); ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename);
	}

	mainWin->urbanGeometry->generateRoadsMultiEx(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsMultiIntEx() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();
	G::global()["fadeOut"] = ui.checkBoxFadeOut->isChecked();

	std::vector<ExFeature> features;
	features.resize(mainWin->urbanGeometry->areas.selectedArea()->hintLine.size());
	for (int i = 0; i < mainWin->urbanGeometry->areas.selectedArea()->hintLine.size(); ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename);
	}

	mainWin->urbanGeometry->generateRoadsMultiIntEx(features);
	
	mainWin->glWidget->updateGL();
}

/**
 * Event handler for button [Generate Roads]
 */
void ControlWidget::generateRoadsInterpolation() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();

	ExFeature feature;
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
	if (filename.isEmpty()) return;
	
	feature.load(filename);

	mainWin->urbanGeometry->generateRoadsInterpolation(feature);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsWarp() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();

	ExFeature feature;
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
	if (filename.isEmpty()) return;
	
	feature.load(filename);

	mainWin->urbanGeometry->generateRoadsWarp(feature);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsSmoothWarp() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();

	ExFeature feature;
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
	if (filename.isEmpty()) return;
	
	feature.load(filename, false);

	mainWin->urbanGeometry->generateRoadsSmoothWarp(feature);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsVerySmoothWarp() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["roadInterpolationFactor"] = ui.horizontalSliderInterpolationFactor->value() * 0.01f;
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["animation"] = ui.checkBoxAnimation->isChecked();

	ExFeature feature;
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
	if (filename.isEmpty()) return;
	
	feature.load(filename, false);

	mainWin->urbanGeometry->generateRoadsVerySmoothWarp(feature);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::clear() {
	if (mainWin->urbanGeometry->areas.selectedIndex >= 0) {
		mainWin->urbanGeometry->areas.selectedArea()->clear();
	}

	mainWin->glWidget->updateGL();
}

void ControlWidget::mergeRoads() {
	mainWin->urbanGeometry->mergeRoads();

	mainWin->glWidget->updateGL();
}

/**
 * エリア間の境界上で、エッジができる限りつながるように、微調整する。
 */
void ControlWidget::connectRoads() {
	mainWin->urbanGeometry->connectRoads();

	mainWin->glWidget->updateGL();
}

/*
void ControlWidget::camera3D() {
	mainWin->glWidget->camera=&mainWin->glWidget->camera3D;
	G::global()["rend_mode"]=1;
	update3D();

	mainWin->glWidget->updateCamera();
}
*/