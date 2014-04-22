#pragma once

#include "VBORenderManager.h"

#include "Camera.h"//2D Camera
#include "Camera3D.h"
#include "RoadGraph.h"
#include "Sketch.h"
#include "ExampleRoadGraph.h"
#include <QGLWidget>
#include <qstring.h>
#include <qvector3d.h>

#include "global.h"

class MyMainWindow;

class GLWidget : public QGLWidget {

public:
	MyMainWindow* mainWin;
	Camera* camera;
	Camera camera2D;
	Camera3D camera3D;

	RoadGraph* roads;
	Sketch* sketch;
	//RoadGraph* ref_roads;
	//float ref_roads_range;			// the range of the ref_roads [1% - 100%]
	ExampleRoadGraph* ex_roads;
	QPoint lastPos;

	VBORenderManager vboRenderManager;

public:
	GLWidget(MyMainWindow* mainWin);
	~GLWidget();

	void drawScene();
	void newRoad();
	void openRoad(QString filename);
	void makeRoadsFromSketch();

	// 3D
	void generate3DGeometry();
	void updateCamera();
protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *e);
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

private:
	void mouseTo2D(int x, int y, QVector2D *result);
	
};

