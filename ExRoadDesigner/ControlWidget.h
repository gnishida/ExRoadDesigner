#pragma once

#include <QDockWidget>
#include "ui_ControlWidget.h"

class MainWindow;

class ControlWidget : public QDockWidget {
Q_OBJECT

private:
	Ui::ControlWidget ui;
	MainWindow* mainWin;

public:
	ControlWidget(MainWindow* mainWin);

	//void setRoadVertex(RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex);
	//void setRoadEdge(RoadEdgePtr selectedEdge);

public slots:
	void updateInterpolationFactor(int value);
	void generateRoadsMultiEx();
	void generateRoadsMultiIntEx();
	void generateRoadsInterpolation();
	void generateRoadsUShape();
	void generateRoadsWarp();
	void generateRoadsSmoothWarp();
	void generateRoadsVerySmoothWarp();
	void clear();
	void mergeRoads();
	void connectRoads();
	void update3D();
	//void camera3D();
};

