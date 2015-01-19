#pragma once

#include <QDockWidget>
#include "ui_ControlWidget.h"

class MainWindow;

class ControlWidget : public QDockWidget {
Q_OBJECT

private:
	
	MainWindow* mainWin;

public:
	Ui::ControlWidget ui;
	ControlWidget(MainWindow* mainWin);

	//void setRoadVertex(RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex);
	//void setRoadEdge(RoadEdgePtr selectedEdge);

public slots:
	void generateRoadsEx();
	void generateRoadsWarp();
	void generateRoadsPM();
	void generateRoadsAliaga();
	void clear();
	void mergeRoads();
	void trimRoads();
	void connectRoads();
	void connectRoads2();
	void updateTerrainLabels(int newValue);
	void updateTerrainEdit(const QString &text);
	void updateRender2D(int newValue);
	void changeTerrainShader(int);

	void contentDesign(int);
	void contentDesignLevel();

};

