#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_TerrainSizeInputDialog.h"

class MainWindow;

class TerrainSizeInputDialog : public QDialog {
Q_OBJECT

private:
	Ui::TerrainSizeInputDialog ui;
	MainWindow* mainWin;

public:
	int width;
	int depth;
	int cellLength;

public:
	TerrainSizeInputDialog(MainWindow* mainWin);

	//void setRoadVertex(RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex);
	//void setRoadEdge(RoadEdgePtr selectedEdge);

public slots:
	void onOK();
	void onCancel();
};

