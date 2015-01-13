#pragma once

#include "glew.h"

#include "ui_PropertyWidget.h"
#include <QDockWidget>
#include "RoadGraph.h"

class MainWindow;

class PropertyWidget : public QDockWidget {
Q_OBJECT

private:
	Ui::PropertyWidget ui;
	MainWindow* mainWin;

public:
	PropertyWidget(MainWindow* mainWin);

	void setRoadVertex(RoadGraph &roads, RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex);
	void setRoadEdge(RoadGraph &roads, RoadEdgeDesc edgeDesc, RoadEdgePtr selectedEdge);
	void resetRoadVertex();
	void resetRoadEdge();

public slots:
	void searchVertex();
};

