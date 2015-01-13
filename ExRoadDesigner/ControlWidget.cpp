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

ControlWidget::ControlWidget(MainWindow* mainWin) : QDockWidget("Control Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditNumAvenueIterations->setText("1500");
	ui.lineEditNumStreetIterations->setText("6000");
	ui.lineEditNumExamples->setText("1");
	ui.checkBoxCleanAvenues->setChecked(false);
	ui.checkBoxCleanStreets->setChecked(false);
	ui.checkBoxLocalStreets->setChecked(true);
	ui.checkBoxCropping->setChecked(false);
	ui.checkBoxUseLayer->setChecked(true);
	ui.checkBoxRemoveSmallBlocks->setChecked(false);
	ui.lineEditMinBlockSize->setText("10000");
	ui.lineEditHoughScale->setText("500.0");
	ui.lineEditPatchDistance1->setText("100");
	ui.lineEditPatchDistance2->setText("20");
	ui.lineEditInterpolateSigma1->setText("0.2");
	ui.lineEditInterpolateSigma2->setText("0.2");
	ui.lineEditInterpolateThreshold1->setText("0.0");
	ui.lineEditRotationAngle->setText("0.0");
	ui.lineEditRoadSnapFactor->setText("0.7");
	ui.lineEditRoadAngleTolerance->setText("20");
	ui.lineEditRotationForSteepSlope->setText("30");
	ui.lineEditMaxBlockSizeForPark->setText("250000");

	// register the event handlers
	connect(ui.pushButtonGenerateTest, SIGNAL(clicked()), this, SLOT(generateRoadsTest()));
	connect(ui.pushButtonGenerateTestWarp, SIGNAL(clicked()), this, SLOT(generateRoadsTestWarp()));
	connect(ui.pushButtonGeneratePatch, SIGNAL(clicked()), this, SLOT(generateRoadsPatch()));
	connect(ui.pushButtonGeneratePatchWarp2, SIGNAL(clicked()), this, SLOT(generateRoadsPatchWarp2()));
	connect(ui.pushButtonGeneratePM, SIGNAL(clicked()), this, SLOT(generateRoadsPM()));
	connect(ui.pushButtonGenerateAliaga, SIGNAL(clicked()), this, SLOT(generateRoadsAliaga()));
	connect(ui.pushButtonClear, SIGNAL(clicked()), this, SLOT(clear()));
	//connect(ui.pushButtonConnect, SIGNAL(clicked()), this, SLOT(connectRoads()));
	connect(ui.pushButtonConnect2, SIGNAL(clicked()), this, SLOT(connectRoads2()));
	connect(ui.pushButtonRemoveIntersectionOnRiver, SIGNAL(clicked()), this, SLOT(removeIntersectionOnRiver()));
	connect(ui.pushButtonRemoveIntersectingEdge, SIGNAL(clicked()), this, SLOT(removeIntersectingEdge()));
	connect(ui.pushButtonPlanarGraph, SIGNAL(clicked()), this, SLOT(planarGraph()));

	connect(ui.pushButtonMerge, SIGNAL(clicked()), this, SLOT(mergeRoads()));
	connect(ui.pushButtonTrim, SIGNAL(clicked()), this, SLOT(trimRoads()));
	//connect(ui.pushButtonCamera3D, SIGNAL(clicked()), this, SLOT(camera3D()));
	// terrain
	connect(ui.terrainPaint_sizeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.terrainPaint_changeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateTerrainLabels(int)));
	connect(ui.terrain_2DShader, SIGNAL(stateChanged(int)),this, SLOT(changeTerrainShader(int)));
	connect(ui.render_2DroadsStrokeSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DroadsExtraWidthSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparksSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.render_2DparcelLineSlider, SIGNAL(valueChanged(int)),this, SLOT(updateRender2D(int)));
	connect(ui.terrain_smooth, SIGNAL(clicked()),this, SLOT(smoothTerrain()));
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

void ControlWidget::generateRoadsTest() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, false);
		features[i].ex_id = i;
	}

	mainWin->urbanGeometry->generateRoadsTest(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsTestWarp() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

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

void ControlWidget::generateRoadsPatch() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, false);
		//features[i].detectShapes(ui.lineEditPatchDistance1->text().toFloat(), ui.lineEditPatchDistance2->text().toFloat());
	}

	mainWin->urbanGeometry->generateRoadsPatchMulti(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsPatchWarp2() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["houghScale"] = ui.lineEditHoughScale->text().toFloat();
	G::global()["avenuePatchDistance"] = ui.lineEditPatchDistance1->text().toFloat();
	G::global()["streetPatchDistance"] = ui.lineEditPatchDistance2->text().toFloat();
	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

	std::vector<ExFeature> features;
	features.resize(numExamples);
	for (int i = 0; i < numExamples; ++i) {
		QString filename = QFileDialog::getOpenFileName(this, tr("Open Feature file..."), "", tr("StreetMap Files (*.xml)"));
		if (filename.isEmpty()) return;
	
		features[i].load(filename, false);
		//features[i].detectShapes(ui.lineEditPatchDistance1->text().toFloat(), ui.lineEditPatchDistance2->text().toFloat());
	}

	mainWin->urbanGeometry->generateRoadsPatchWarp2(features);
	
	mainWin->glWidget->updateGL();
}

void ControlWidget::generateRoadsPM() {
	if (mainWin->urbanGeometry->areas.selectedIndex == -1) return;

	G::global()["numAvenueIterations"] = ui.lineEditNumAvenueIterations->text().toInt();
	G::global()["numStreetIterations"] = ui.lineEditNumStreetIterations->text().toInt();
	int numExamples = ui.lineEditNumExamples->text().toInt();
	G::global()["cleanAvenues"] = ui.checkBoxCleanAvenues->isChecked();
	G::global()["cleanStreets"] = ui.checkBoxCleanStreets->isChecked();
	G::global()["generateLocalStreets"] = ui.checkBoxLocalStreets->isChecked();
	G::global()["cropping"] = ui.checkBoxCropping->isChecked();
	G::global()["useLayer"] = ui.checkBoxUseLayer->isChecked();
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

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
	G::global()["removeSmallBlocks"] = ui.checkBoxRemoveSmallBlocks->isChecked();
	G::global()["minBlockSize"] = ui.lineEditMinBlockSize->text().toFloat();

	G::global()["interpolationSigma1"] = ui.lineEditInterpolateSigma1->text().toFloat();
	G::global()["interpolationSigma2"] = ui.lineEditInterpolateSigma2->text().toFloat();
	G::global()["interpolationThreshold1"] = ui.lineEditInterpolateThreshold1->text().toFloat();
	G::global()["rotationAngle"] = ui.lineEditRotationAngle->text().toFloat() / 180.0f * M_PI;
	G::global()["roadSnapFactor"] = ui.lineEditRoadSnapFactor->text().toFloat();
	G::global()["roadAngleTolerance"] = ui.lineEditRoadAngleTolerance->text().toFloat() / 180.0f * M_PI;
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

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

	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}

/**
 * エリア間の境界上で、エッジができる限りつながるように、微調整する。
 */
void ControlWidget::connectRoads() {
	mainWin->urbanGeometry->connectRoads();

	mainWin->glWidget->updateGL();
}

void ControlWidget::connectRoads2() {
	G::global()["rotationForSteepSlope"] = ui.lineEditRotationForSteepSlope->text().toFloat() / 180.0f * M_PI;

	RoadGeneratorHelper::connectRoads3(mainWin->urbanGeometry->roads, &mainWin->glWidget->vboRenderManager, 1000.0f, 200.0f);
	mainWin->urbanGeometry->adaptToTerrain();

	mainWin->glWidget->updateGL();
}

void ControlWidget::removeIntersectionOnRiver() {
	RoadGeneratorHelper::removeIntersectionsOnRiver(mainWin->urbanGeometry->roads, &mainWin->glWidget->vboRenderManager, G::getFloat("seaLevelForStreet"));
	GraphUtil::removeIsolatedEdges(mainWin->urbanGeometry->roads);
	GraphUtil::removeIsolatedVertices(mainWin->urbanGeometry->roads);
	GraphUtil::reduce(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);
	
	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}

void ControlWidget::removeIntersectingEdge() {
	GraphUtil::removeSelfIntersectingRoads(mainWin->urbanGeometry->roads);
	GraphUtil::removeIsolatedEdges(mainWin->urbanGeometry->roads);
	GraphUtil::removeIsolatedVertices(mainWin->urbanGeometry->roads);
	GraphUtil::reduce(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);

	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}

void ControlWidget::planarGraph() {
	GraphUtil::planarify(mainWin->urbanGeometry->roads);
	GraphUtil::removeIsolatedEdges(mainWin->urbanGeometry->roads);
	GraphUtil::removeIsolatedVertices(mainWin->urbanGeometry->roads);
	GraphUtil::reduce(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);

	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}

void ControlWidget::updateTerrainLabels(int newValue){
		int size=ui.terrainPaint_sizeSlider->value();
		ui.terrainPaint_sizeLabel->setText("Size: "+QString::number(size)+"%");
		G::global()["2DterrainEditSize"]=size/100.0f;

		float change=ui.terrainPaint_changeSlider->value()*1785/100.0f;
		ui.terrainPaint_changeLabel->setText("Ch: "+QString::number(change,'f',0)+"m");
		G::global()["2DterrainEditChange"]=change;
}//

void ControlWidget::updateRender2D(int newValue){
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

		float parcelLine=ui.render_2DparcelLineSlider->value()*0.1f;
		ui.render_2DparcelLineLabel->setText("Par. Line: "+QString::number(parcelLine,'f',1)+"");
		G::global()["2d_parcelLine"]=parcelLine;

		if(newValue!=-1){//init
			mainWin->urbanGeometry->roads.modified=true;//force 
			mainWin->glWidget->updateGL();
		}
}//

void ControlWidget::changeTerrainShader(int){
	bool shader2D=ui.terrain_2DShader->isChecked();
	G::global()["shader2D"] = shader2D;
	int terrainMode;
	if(shader2D==true){
		terrainMode=0;
		if(ui.content_checkbox->isChecked()==true)
			terrainMode=2;
	}else 
		terrainMode=1;
	printf("terrainMode %d\n",terrainMode);
	mainWin->glWidget->vboRenderManager.changeTerrainShader(terrainMode);//could have used !shader2D
	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}//

void ControlWidget::smoothTerrain(){
	mainWin->glWidget->vboRenderManager.vboTerrain.smoothTerrain();
	mainWin->urbanGeometry->adaptToTerrain();
	mainWin->glWidget->updateGL();
}//

void ControlWidget::contentDesign(int){
	printf("Content design clicked\n");
	bool contentD=ui.content_checkbox->isChecked();
	if(contentD==true){//set content design
		printf("Content design ENABLED\n");
		//if(ui.terrain_2DShader->isChecked()==false)
			ui.terrain_2DShader->setChecked(true);//check 2D

			/*glActiveTexture(GL_TEXTURE7); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glActiveTexture(GL_TEXTURE0);*/
		//int terrainMode=2;
		//mainWin->glWidget->vboRenderManager.changeTerrainShader(terrainMode);//could have used !shader2D
		//mainWin->urbanGeometry->adaptToTerrain();
		//mainWin->glWidget->updateGL();
	}else{//restore normal
		ui.terrain_2DShader->setChecked(false);//check 2D

		/*glActiveTexture(GL_TEXTURE7); 
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR );
			glActiveTexture(GL_TEXTURE0);*/
		/*bool shader2D=ui.terrain_2DShader->isChecked();
		G::global()["shader2D"] = shader2D;
		int terrainMode;
		if(shader2D==true)
			terrainMode=0;
		else 
			terrainMode=1;
		mainWin->glWidget->vboRenderManager.changeTerrainShader(terrainMode);//could have used !shader2D
		mainWin->urbanGeometry->adaptToTerrain();
		mainWin->glWidget->updateGL();*/

	}
	changeTerrainShader(0);
}//

void ControlWidget::contentDesignLevel(){
	int newLevel=0;
	/*if(ui.content_0->isChecked()){
		newLevel=0;
	}*/
	if(ui.content_1->isChecked()){
		newLevel=1;
	}
	if(ui.content_7->isChecked()){
		newLevel=7;
	}
	if(ui.content_8->isChecked()){
		newLevel=8;
	}
	if(ui.content_9->isChecked()){
		newLevel=9;
	}
	if(ui.content_10->isChecked()){
		newLevel=10;
	}
	if(ui.content_11->isChecked()){
		newLevel=11;
	}
	G::global()["content_terrainLevel"]=newLevel;
	printf("New Level %d\n",newLevel);
}//

/*
void ControlWidget::camera3D() {
	mainWin->glWidget->camera=&mainWin->glWidget->camera3D;
	G::global()["rend_mode"]=1;
	update3D();

	mainWin->glWidget->updateCamera();
}
*/