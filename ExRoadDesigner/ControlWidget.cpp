#include "ControlWidget.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "UrbanGeometry.h"
#include "GLWidget3D.h"
#include "global.h"
#include "GraphUtil.h"
#include "ExFeature.h"
#include "RoadGeneratorHelper.h"
#include "BBox.h"
#include "SmallBlockRemover.h"

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	ui.TabWidget->setCurrentIndex(0);
	ui.lineEditNumAvenueIterations->setText("1500");
	ui.lineEditNumStreetIterations->setText("6000");
	ui.lineEditNumExamples->setText("1");
	ui.checkBoxCleanAvenues->setChecked(true);
	ui.checkBoxCleanStreets->setChecked(false);
	ui.checkBoxLocalStreets->setChecked(true);
	ui.checkBoxCropping->setChecked(false);
	ui.checkBoxUseLayer->setChecked(false);
	ui.lineEditMinBlockSize->setText("10000");
	ui.lineEditHoughScale->setText("500.0");
	ui.lineEditPatchDistance1->setText("80");
	ui.lineEditPatchDistance2->setText("20");
	ui.lineEditInterpolateSigma1->setText("0.2");
	ui.lineEditInterpolateSigma2->setText("0.2");
	ui.lineEditRotationAngle->setText("0.0");
	ui.lineEditRoadAngleTolerance->setText("60");
	ui.lineEditSlopeTolerance->setText("36.4");
	ui.lineEditAcrossRiverTolerance->setText("300");
	ui.lineEditMaxBlockSizeForPark->setText("250000");
	ui.checkBoxSaveRoadImages->setChecked(false);
	ui.checkBoxSavePatchImages->setChecked(false);
	
	ui.terrainPaint_changeEdit->setText("200");
	ui.terrainPaint_changeSlider->setMinimum(0);
	ui.terrainPaint_changeSlider->setMaximum(1000);
	ui.terrainPaint_changeSlider->setValue(200);
	ui.terrainPaint_sizeSlider->setMinimum(1);
	ui.terrainPaint_sizeSlider->setMaximum(1000);
	ui.terrainPaint_sizeSlider->setValue(500);

	ui.render_2DparcelLineSlider->setMinimum(0);
	ui.render_2DparcelLineSlider->setMaximum(50);
	ui.render_2DparcelLineSlider->setSingleStep(1);
	ui.render_2DparcelLineSlider->setValue(5);

	// register the event handlers
	connect(ui.pushButtonGenerateEx, SIGNAL(clicked()), this, SLOT(generateRoadsEx()));
	connect(ui.pushButtonGenerateWarp, SIGNAL(clicked()), this, SLOT(generateRoadsWarp()));
	connect(ui.pushButtonGeneratePM, SIGNAL(clicked()), this, SLOT(generateRoadsPM()));
	connect(ui.pushButtonGenerateAliaga, SIGNAL(clicked()), this, SLOT(generateRoadsAliaga()));
	connect(ui.pushButtonClear, SIGNAL(clicked()), this, SLOT(clear()));
	connect(ui.pushButtonConnect2, SIGNAL(clicked()), this, SLOT(connectRoads2()));

	connect(ui.pushButtonMerge, SIGNAL(clicked()), this, SLOT(mergeRoads()));
	connect(ui.pushButtonTrim, SIGNAL(clicked()), this, SLOT(trimRoads()));
	connect(ui.pushButtonClearBoundaryFlag, SIGNAL(clicked()), this, SLOT(clearBoundaryFlag()));
	connect(ui.pushButtonRemoveDanglingEdges, SIGNAL(clicked()), this, SLOT(removeDanglingEdges()));
	connect(ui.pushButtonRemoveCloseEdges, SIGNAL(clicked()), this, SLOT(removeCloseEdges()));

	// terrain
	connect(ui.terrainPaint_sizeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.terrainPaint_changeEdit, SIGNAL(textChanged(const QString &)),this, SLOT(updateTerrainEdit(const QString &)));
	connect(ui.terrainPaint_changeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.terrain_2DShader, SIGNAL(stateChanged(int)),this, SLOT(changeTerrainShader(int)));
	connect(ui.render_2DroadsStrokeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DroadsExtraWidthSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparksSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparcelLineSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.content_checkbox, SIGNAL(stateChanged(int)),this, SLOT(contentDesign(int)));

	connect(ui.content_0, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_1, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_7, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_8, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_9, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_10, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));
	connect(ui.content_11, SIGNAL(clicked()), this, SLOT(contentDesignLevel()));

	updateRender2D(-1); // update just labels
	updateTerrainLabels(-1);
	contentDesignLevel();

	hide();

	
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void ControlWidget::generateRoadsEx() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["seaLevel"] = 60.0f;
	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["slopeTolerance"] = ui.lineEditSlopeTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["acrossRiverTolerance"] = ui.lineEditAcrossRiverTolerance->text().toFloat();
	G::global()["saveRoadImages"] = ui.checkBoxSaveRoadImages->isChecked();
	G::global()["savePatchImages"] = ui.checkBoxSavePatchImages->isChecked();

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, false);
		features[i].ex_id = i;
	}

	mainWin->urbanGeometry->generateRoadsEx(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsWarp() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["seaLevel"] = 60.0f;
	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["slopeTolerance"] = ui.lineEditSlopeTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["acrossRiverTolerance"] = ui.lineEditAcrossRiverTolerance->text().toFloat();
	G::global()["saveRoadImages"] = ui.checkBoxSaveRoadImages->isChecked();
	G::global()["savePatchImages"] = ui.checkBoxSavePatchImages->isChecked();

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, false);
		features[i].ex_id = i;
	}

	mainWin->urbanGeometry->generateRoadsWarp(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsPM() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["seaLevel"] = 60.0f;
	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["acrossRiverTolerance"] = ui.lineEditAcrossRiverTolerance->text().toFloat();
	G::global()["slopeTolerance"] = ui.lineEditSlopeTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["saveRoadImages"] = ui.checkBoxSaveRoadImages->isChecked();
	G::global()["savePatchImages"] = ui.checkBoxSavePatchImages->isChecked();

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, true);
	}

	mainWin->urbanGeometry->generateRoadsPM(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsAliaga() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();

	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["saveRoadImages"] = ui.checkBoxSaveRoadImages->isChecked();

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, true);
	}

	mainWin->urbanGeometry->generateRoadsAliaga(features);
	
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

void ControlWidget::trimRoads() {
	BBox bbox;
	bbox.addPoint(QVector2D(mainWin->glWidget->vboRenderManager.minPos.x() + 20, mainWin->glWidget->vboRenderManager.minPos.y() + 20));
	bbox.addPoint(QVector2D(mainWin->glWidget->vboRenderManager.maxPos.x() - 20, mainWin->glWidget->vboRenderManager.maxPos.y() - 20));
	Polygon2D polygon;
	boost::geometry::convert(bbox, polygon);
	GraphUtil::trim(mainWin->urbanGeometry->roads, polygon);

	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->updateGL();
}

/**
 * エリア間の境界上で、エッジができる限りつながるように、微調整する。
 */
void ControlWidget::connectRoads2() {
	RoadGeneratorHelper::connectRoads3(mainWin->urbanGeometry->roads, &mainWin->glWidget->vboRenderManager, 1000.0f, 200.0f);
	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->updateGL();
}

void ControlWidget::clearBoundaryFlag() {
	RoadGeneratorHelper::clearBoundaryFlag(mainWin->urbanGeometry->roads);
}

void ControlWidget::removeDanglingEdges() {
	RoadGeneratorHelper::removeDanglingEdges(mainWin->urbanGeometry->roads);
	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->updateGL();
}

void ControlWidget::removeCloseEdges() {
	RoadGeneratorHelper::removeCloseEdges(mainWin->urbanGeometry->roads, 0.6f);
	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->updateGL();
}

void ControlWidget::removeSmallBlocks() {
	SmallBlockRemover::remove(mainWin->urbanGeometry->roads, ui.lineEditMinBlockSize->text().toFloat());
	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->updateGL();
}

void ControlWidget::updateTerrainLabels(int newValue) {
	int size = ui.terrainPaint_sizeSlider->value();
	ui.terrainPaint_sizeLabel->setText("Size: "+QString::number(size,'f',0)+"m");
	G::global()["2DterrainEditSize"] = size;

	float change = ui.terrainPaint_changeSlider->value();
	ui.terrainPaint_changeEdit->setText(QString::number(change, 'f', 0));
}

void ControlWidget::updateTerrainEdit(const QString &text) {
	ui.terrainPaint_changeSlider->setValue(ui.terrainPaint_changeEdit->text().toFloat());
}

void ControlWidget::updateRender2D(int newValue) {
	float stroke=ui.render_2DroadsStrokeSlider->value()*0.1f;
	ui.render_2DroadsStrokeLabel->setText("Stroke: "+QString::number(stroke,'f',1)+"");
	G::global()["2DroadsStroke"]=stroke;

	float extraWidth=ui.render_2DroadsExtraWidthSlider->value()*0.1f;
	ui.render_2DroadsExtraWidthLabel->setText("R Width: "+QString::number(extraWidth,'f',1)+"");
	G::global()["2DroadsExtraWidth"]=extraWidth;

	int parkPer=ui.render_2DparksSlider->value();
	ui.render_2DparksLabel->setText("Park: "+QString::number(parkPer)+"%");
	G::global()["2d_parkPer"]=parkPer;

	G::global()["maxBlockSizeForPark"] = ui.lineEditMaxBlockSizeForPark->text().toFloat();

	float parcelLine=ui.render_2DparcelLineSlider->value() * 0.1;
	ui.render_2DparcelLineLabel->setText("Par. Line: "+QString::number(parcelLine,'f',1)+"");
	G::global()["2d_parcelLine"] = parcelLine;

	if(newValue!=-1){//init
		mainWin->urbanGeometry->roads.modified=true;//force 
		mainWin->glWidget->updateGL();
	}
}

void ControlWidget::changeTerrainShader(int) {
	bool shader2D = ui.terrain_2DShader->isChecked();
	G::global()["shader2D"] = shader2D;
	int terrainMode;
	if (shader2D) {
		terrainMode = 0;
		if (ui.content_checkbox->isChecked()) terrainMode=2;
	} else  {
		terrainMode=1;
	}

	printf("terrainMode %d\n", terrainMode);
	mainWin->glWidget->vboRenderManager.changeTerrainShader(terrainMode);
	mainWin->urbanGeometry->update(mainWin->glWidget->vboRenderManager);
	mainWin->glWidget->shadow.makeShadowMap(mainWin->glWidget);
	mainWin->glWidget->updateGL();
}

void ControlWidget::contentDesign(int) {
	if (ui.content_checkbox->isChecked()) {
		ui.terrain_2DShader->setChecked(true);
	} else {
		ui.terrain_2DShader->setChecked(false);//check 2D
	}
	changeTerrainShader(0);
}

void ControlWidget::contentDesignLevel(){
	int newLevel = 0;
	if (ui.content_1->isChecked()) {
		newLevel=1;
	} else if (ui.content_7->isChecked()) {
		newLevel=7;
	} else if (ui.content_8->isChecked()) {
		newLevel=8;
	} else if (ui.content_9->isChecked()) {
		newLevel=9;
	} else if (ui.content_10->isChecked()) {
		newLevel=10;
	} else if (ui.content_11->isChecked()) {
		newLevel=11;
	}
	G::global()["content_terrainLevel"] = newLevel * 7.0f;
	printf("New Level %d\n",newLevel);
}
