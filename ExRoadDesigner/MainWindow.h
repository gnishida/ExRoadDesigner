#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "glew.h"
#include <QtGui/QMainWindow>
#include "ui_MainWindow.h"
#include "ControlWidget.h"
#include "PropertyWidget.h"
#include "GLWidget3D.h"
#include "UrbanGeometry.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static enum { MODE_AREA_SELECT = 0, MODE_AREA_CREATE, MODE_HINT_LINE, MODE_AVENUE_SKETCH, MODE_DEBUG };

public:
	Ui::MainWindow ui;
	ControlWidget* controlWidget;
	PropertyWidget* propertyWidget;
	GLWidget3D* glWidget;
	UrbanGeometry* urbanGeometry;

	int mode;

public:
	MainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~MainWindow();

protected:
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

public slots:
	void onNewTerrain();
	void onLoadTerrain();
	void onSaveTerrain();
	void onLoadRoads();
	void onSaveRoads();
	void onClearRoads();
	void onLoadAreas();
	void onSaveAreas();
	void onSaveImage();
	void onAreaMenu();
	void onAreaSelect();
	void onAreaCreate();
	void onAreaDelete();
	void onHintLine();
	void onAvenueSketch();
	void onDebug();
	void onGenerateBlocks();
	void onGenerateParcels();
	void onDisplayRoads();
	void onRenderingDefault();
	void onRenderingTexture();
	void onRenderingGroupBy();
	void onRenderingGenerationType();
	void onShowControlWidget();
	void onShowPropertyWidget();
};

#endif // MAINWINDOW_H
