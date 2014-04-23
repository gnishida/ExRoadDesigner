#include "MainWindow.h"
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);
	propertyWidget = new PropertyWidget(this);

	// setup the toolbar
	ui.fileToolBar->addAction(ui.actionNewTerrain);
	ui.fileToolBar->addAction(ui.actionOpenTerrain);
	ui.areaToolBar->addAction(ui.actionAreaSelect);
	ui.areaToolBar->addAction(ui.actionAreaCreate);
	ui.areaToolBar->addAction(ui.actionHintLine);
	ui.areaToolBar->addAction(ui.actionAvenueSketch);
	ui.areaToolBar->addAction(ui.actionDebug);

	ui.actionAreaSelect->setChecked(true);

	// register the menu's action handlers
	connect(ui.actionNewTerrain, SIGNAL(triggered()), this, SLOT(onNewTerrain()));
	connect(ui.actionOpenTerrain, SIGNAL(triggered()), this, SLOT(onLoadTerrain()));
	connect(ui.actionSaveTerrain, SIGNAL(triggered()), this, SLOT(onSaveTerrain()));
	connect(ui.actionLoadRoads, SIGNAL(triggered()), this, SLOT(onLoadRoads()));
	connect(ui.actionSaveRoads, SIGNAL(triggered()), this, SLOT(onSaveRoads()));
	connect(ui.actionClearRoads, SIGNAL(triggered()), this, SLOT(onClearRoads()));
	connect(ui.actionLoadAreas, SIGNAL(triggered()), this, SLOT(onLoadAreas()));
	connect(ui.actionSaveAreas, SIGNAL(triggered()), this, SLOT(onSaveAreas()));
	connect(ui.actionSaveImage, SIGNAL(triggered()), this, SLOT(onSaveImage()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.menuArea, SIGNAL(aboutToShow()), this, SLOT(onAreaMenu()));
	connect(ui.actionAreaSelect, SIGNAL(triggered()), this, SLOT(onAreaSelect()));
	connect(ui.actionAreaCreate, SIGNAL(triggered()), this, SLOT(onAreaCreate()));
	connect(ui.actionAreaDelete, SIGNAL(triggered()), this, SLOT(onAreaDelete()));
	connect(ui.actionHintLine, SIGNAL(triggered()), this, SLOT(onHintLine()));
	connect(ui.actionAvenueSketch, SIGNAL(triggered()), this, SLOT(onAvenueSketch()));
	connect(ui.actionDebug, SIGNAL(triggered()), this, SLOT(onDebug()));
	connect(ui.actionGenerateBlocks, SIGNAL(triggered()), this, SLOT(onGenerateBlocks()));
	connect(ui.actionGenerateParcels, SIGNAL(triggered()), this, SLOT(onGenerateParcels()));
	connect(ui.actionDisplayHighway, SIGNAL(triggered()), this, SLOT(onDisplayRoads()));
	connect(ui.actionDisplayBoulevard, SIGNAL(triggered()), this, SLOT(onDisplayRoads()));
	connect(ui.actionRenderingDefault, SIGNAL(triggered()), this, SLOT(onRenderingDefault()));
	connect(ui.actionRenderingTexture, SIGNAL(triggered()), this, SLOT(onRenderingTexture()));
	connect(ui.actionRenderingGroupBy, SIGNAL(triggered()), this, SLOT(onRenderingGroupBy()));
	connect(ui.actionRenderingGenerationType, SIGNAL(triggered()), this, SLOT(onRenderingGenerationType()));
	connect(ui.actionControlWidget, SIGNAL(triggered()), this, SLOT(onShowControlWidget()));
	connect(ui.actionPropertyWidget, SIGNAL(triggered()), this, SLOT(onShowPropertyWidget()));

	// setup the GL widget
	glWidget = new GLWidget3D(this);
	setCentralWidget(glWidget);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);

	urbanGeometry = new UrbanGeometry(this);

	mode = MODE_AREA_SELECT;
}

MainWindow::~MainWindow() {
}

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

void MainWindow::onNewTerrain() {
	/*
	TerrainSizeInputDialog dlg(this);
	if (dlg.exec() == QDialog::Accepted) {
		urbanGeometry->newTerrain(dlg.width, dlg.depth, dlg.cellLength);
		glWidget->updateGL();
	}
	*/
}

void MainWindow::onLoadTerrain() {
	/*
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Terrain file..."), "", tr("Terrain Files (*.trn)"));

	if (filename.isEmpty()) {
		return;
	}

	urbanGeometry->loadTerrain(filename);

	glWidget->updateGL();
	*/
}

void MainWindow::onSaveTerrain() {
	/*
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Terrain file..."), "", tr("Terrain Files (*.trn)"));

	if (filename.isEmpty()) {
		return;
	}

	urbanGeometry->saveTerrain(filename);
	*/
}

void MainWindow::onLoadRoads() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Street Map file..."), "", tr("StreetMap Files (*.gsm)"));

	if (filename.isEmpty()) {
		return;
	}

	urbanGeometry->loadRoads(filename);

	glWidget->updateGL();
}

void MainWindow::onSaveRoads() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Street Map file..."), "", tr("StreetMap Files (*.gsm)"));

	if (filename.isEmpty()) {
		printf("Unable to open file\n");
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	urbanGeometry->saveRoads(filename);

	QApplication::restoreOverrideCursor();
}

void MainWindow::onClearRoads() {
	urbanGeometry->clearRoads();
	glWidget->updateGL();
}

void MainWindow::onLoadAreas() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Area file..."), "", tr("Area Files (*.area)"));

	if (filename.isEmpty()) {
		return;
	}

	urbanGeometry->loadAreas(filename);

	glWidget->updateGL();
}

void MainWindow::onSaveAreas() {
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Area Map file..."), "", tr("Area Files (*.area)"));

	if (filename.isEmpty()) {
		printf("Unable to open file\n");
		return;
	}

	QApplication::setOverrideCursor(Qt::WaitCursor);

	urbanGeometry->saveAreas(filename);

	QApplication::restoreOverrideCursor();
}

void MainWindow::onSaveImage() {
	/*
	QString filename = QFileDialog::getSaveFileName(this, tr("Save image file..."), "", tr("Jpeg Image (*.jpg *.jpeg)"));

	if (filename.isEmpty()) {
		printf("Unable to open file\n");
		return;
	}

	QPixmap image = QPixmap::grabWidget(glWidget);
	image.save(filename, "JPG");
	*/
}

void MainWindow::onAreaMenu() {
	ui.actionAreaSelect->setChecked(mode == MODE_AREA_SELECT);
	ui.actionAreaCreate->setChecked(mode == MODE_AREA_CREATE);
	ui.actionHintLine->setChecked(mode == MODE_HINT_LINE);
	ui.actionAvenueSketch->setChecked(mode == MODE_AVENUE_SKETCH);
	ui.actionDebug->setChecked(mode == MODE_DEBUG);
}

void MainWindow::onAreaSelect() {
	mode = MODE_AREA_SELECT;
	ui.actionAreaCreate->setChecked(false);
	ui.actionHintLine->setChecked(false);
	ui.actionAvenueSketch->setChecked(false);
	ui.actionDebug->setChecked(false);
}

void MainWindow::onAreaCreate() {
	mode = MODE_AREA_CREATE;
	ui.actionAreaSelect->setChecked(false);
	ui.actionHintLine->setChecked(false);
	ui.actionAvenueSketch->setChecked(false);
	ui.actionDebug->setChecked(false);
}

void MainWindow::onAreaDelete() {
	urbanGeometry->areas.deleteArea();

	glWidget->updateGL();
}

void MainWindow::onHintLine() {
	mode = MODE_HINT_LINE;
	ui.actionAreaCreate->setChecked(false);
	ui.actionAreaSelect->setChecked(false);
	ui.actionHighwaySketch->setChecked(false);
	ui.actionAvenueSketch->setChecked(false);
	ui.actionDebug->setChecked(false);
}

void MainWindow::onAvenueSketch() {
	mode = MODE_AVENUE_SKETCH;
	ui.actionAreaCreate->setChecked(false);
	ui.actionAreaSelect->setChecked(false);
	ui.actionHighwaySketch->setChecked(false);
	ui.actionDebug->setChecked(false);
}

void MainWindow::onDebug() {
	mode = MODE_DEBUG;
	ui.actionAreaSelect->setChecked(false);
	ui.actionAreaCreate->setChecked(false);
	ui.actionHintLine->setChecked(false);
	ui.actionAvenueSketch->setChecked(false);
}

void MainWindow::onGenerateBlocks() {
	/*
	urbanGeometry->generateBlocks();

	glWidget->updateGL();
	*/
}

void MainWindow::onGenerateParcels() {
	/*
	urbanGeometry->generateParcels();

	glWidget->updateGL();
	*/
}

void MainWindow::onDisplayRoads() {
	urbanGeometry->roads.showHighways = ui.actionDisplayHighway->isChecked();
	urbanGeometry->roads.showBoulevards = ui.actionDisplayBoulevard->isChecked();
	urbanGeometry->roads.showAvenues = ui.actionDisplayAvenue->isChecked();
	urbanGeometry->roads.showLocalStreets = ui.actionDisplayLocalStreet->isChecked();
	urbanGeometry->roads.setModified();

	glWidget->updateGL();
}

void MainWindow::onRenderingDefault() {
	ui.actionRenderingTexture->setChecked(false);
	ui.actionRenderingGroupBy->setChecked(false);
	ui.actionRenderingGenerationType->setChecked(false);

	urbanGeometry->roads.renderMode = RoadGraph::RENDER_DEFAULT;
	urbanGeometry->roads.setModified();

	glWidget->updateGL();
}

void MainWindow::onRenderingTexture() {
	ui.actionRenderingDefault->setChecked(false);
	ui.actionRenderingGroupBy->setChecked(false);
	ui.actionRenderingGenerationType->setChecked(false);

	urbanGeometry->roads.renderMode = RoadGraph::RENDER_TEXTURE;
	urbanGeometry->roads.setModified();

	glWidget->updateGL();
}

void MainWindow::onRenderingGroupBy() {
	ui.actionRenderingDefault->setChecked(false);
	ui.actionRenderingTexture->setChecked(false);
	ui.actionRenderingGenerationType->setChecked(false);

	urbanGeometry->roads.renderMode = RoadGraph::RENDER_GROUPBY;
	urbanGeometry->roads.setModified();

	glWidget->updateGL();
}

void MainWindow::onRenderingGenerationType() {
	ui.actionRenderingDefault->setChecked(false);
	ui.actionRenderingTexture->setChecked(false);
	ui.actionRenderingGroupBy->setChecked(false);

	urbanGeometry->roads.renderMode = RoadGraph::RENDER_GENERATION_TYPE;
	urbanGeometry->roads.setModified();

	glWidget->updateGL();
}

void MainWindow::onShowControlWidget() {
	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}

void MainWindow::onShowPropertyWidget() {
	propertyWidget->show();
	addDockWidget(Qt::RightDockWidgetArea, propertyWidget);
}

