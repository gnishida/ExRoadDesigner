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
	static enum { MODE_AREA_SELECT = 0, MODE_AREA_CREATE, MODE_HINT_LINE, MODE_HIGHWAY_SKETCH, MODE_AVENUE_SKETCH, MODE_STREET_SKETCH, MODE_CONTROL_POINTS };

public:
	Ui::MainWindow ui;
	ControlWidget* controlWidget;
	PropertyWidget* propertyWidget;
	GLWidget3D* glWidget;
	UrbanGeometry* urbanGeometry;

	cv::Mat origTerrain;

	int mode;
	//int imgCount;
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
	void onAddRoads();
	void onSaveRoads();
	void onClearRoads();
	void onLoadAreas();
	void onSaveAreas();
	void onSaveImage();
	void onSaveImageHD();
	void onLoadCamera();
	void onSaveCamera();
	void onResetCamera();
	void onAreaMenu();
	void onAreaSelect();
	void onAreaCreate();
	void onAreaDelete();
	void onAreaDeleteAll();
	void onHintLine();
	void onCutRoads();
	void onHighwaySketch();
	void onAvenueSketch();
	void onStreetSketch();
	void onControlPoints();
	void onDetectCircle();
	void onDetectStructure();
	//void onConvert();
	void onGenerate2D();
	void onGenerate3D();
	void onGenerate3DRoads();
	void onDisplayRoads();
	void onRenderingDefault();
	void onRenderingTexture();
	void onRenderingGroupBy();
	void onRenderingGenerationType();
	void onGenerateRegularGrid();
	void onGenerateCurvyGrid();
	void onRotationVideo();
	void onInterpolationVideo();
	void onBlendingVideo();
	void onGrowingVideo();
	void on3DVideo();
	void onMountainVideo();
	void onTerrainGeneration();
	void onUpdateMountain();
	void onTerrainSegmentation();
	void onTerrainDataConverter();
	void onShowControlWidget();
	void onShowPropertyWidget();
	void onDebug();
};

#endif // MAINWINDOW_H
