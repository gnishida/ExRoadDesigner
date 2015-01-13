#include "MainWindow.h"
#include <road/GraphUtil.h>

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags) {
	ui.setupUi(this);

	// setup the docking widgets
	controlWidget = new ControlWidget(this);

	// setup the toolbar
	ui.fileToolBar->addAction(ui.actionNew);
	ui.fileToolBar->addAction(ui.actionOpen);
	ui.areaToolBar->addAction(ui.actionHintLine);

	// register the menu's action handlers
	connect(ui.actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
	connect(ui.actionLoadArea, SIGNAL(triggered()), this, SLOT(onLoadArea()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
	connect(ui.actionCreateArea, SIGNAL(triggered()), this, SLOT(onCreateArea()));
	connect(ui.actionHintLine, SIGNAL(triggered()), this, SLOT(onHintLine()));
	connect(ui.actionControlWidget, SIGNAL(triggered()), this, SLOT(onShowControlWidget()));

	// setup the GL widget
	glWidget = new GLWidget(this);
	setCentralWidget(glWidget);

	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);

	mode = MODE_AREA_CREATE;
}

MainWindow::~MainWindow() {
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void MainWindow::keyPressEvent(QKeyEvent* e) {
	glWidget->keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent* e) {
	glWidget->keyReleaseEvent(e);
}

void MainWindow::onNew() {
	glWidget->roads.clear();

	glWidget->updateGL();
}

void MainWindow::onOpen() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open StreetMap file..."), "", tr("StreetMap Files (*.gsm)"));

	if (filename.isEmpty()) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);
	GraphUtil::loadRoads(glWidget->roads, filename);
	GraphUtil::copyRoads(glWidget->roads, glWidget->origRoads);
	glWidget->updateGL();
	QApplication::restoreOverrideCursor();
}

void MainWindow::onLoadArea() {
	QString filename = QFileDialog::getOpenFileName(this, tr("Open Area file..."), "", tr("Area Files (*.xml)"));

	if (filename.isEmpty()) return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	{
		ExFeature feature;
		feature.load(filename, false);

		glWidget->selectedArea.clear();
		if (feature.area.size() > 0) {
			glWidget->selectedAreaBuilder.start(feature.area[0]);
			for (int i = 1; i < feature.area.size(); ++i) {
				glWidget->selectedAreaBuilder.addPoint(feature.area[i]);
			}
			glWidget->selectedAreaBuilder.end();
			glWidget->selectedArea = feature.area;
		}

		glWidget->hintLine.clear();
		if (feature.hintLine.size() > 0) {
			glWidget->hintLineBuilder.start(feature.hintLine[0]);
			for (int i = 1; i < feature.hintLine.size(); ++i) {
				glWidget->hintLineBuilder.addPoint(feature.hintLine[i]);
			}
			glWidget->hintLineBuilder.end();
			glWidget->hintLine = feature.hintLine;
		}
	}

	glWidget->updateGL();
	QApplication::restoreOverrideCursor();
}

void MainWindow::onCreateArea() {
	mode = MODE_AREA_CREATE;
}

void MainWindow::onHintLine() {
	mode = MODE_HINT_LINE;
}

void MainWindow::onShowControlWidget() {
	controlWidget->show();
	addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
}
