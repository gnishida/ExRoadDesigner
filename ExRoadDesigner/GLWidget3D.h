/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#pragma once

#include "glew.h"
#include "VBORenderManager.h"
#include "GLWidget3D_Shadows.h"

#include <QGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>
#include "Camera.h"
#include "Camera2D.h"
#include "Camera3D.h"
#include "FlyThroughCamera.h"
#include "RoadGraph.h"

class MainWindow;

class GLWidget3D : public QGLWidget {
public:
	MainWindow* mainWin;
	Camera2D camera2D;
	Camera3D camera3D;
	FlyThroughCamera flyCamera;
	Camera* camera;

	bool shiftPressed;
	bool controlPressed;
	bool altPressed;
	bool keyMPressed;
	QPoint lastPos;
	float farPlaneToSpaceRadiusFactor;
	float spaceRadius;
	float rotationSensitivity;
	float zoomSensitivity;

	RoadVertexDesc selectedVertexDesc;
	RoadVertexPtr selectedVertex;
	RoadEdgeDesc selectedEdgeDesc;
	RoadEdgePtr selectedEdge;
	bool vertexSelected;
	bool edgeSelected;

	VBORenderManager vboRenderManager;
	GLWidgetSimpleShadow shadow;

public:
	GLWidget3D(MainWindow *parent);
	~GLWidget3D();

	void updateMe();
	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	void keyPressEvent(QKeyEvent* e);
	void keyReleaseEvent(QKeyEvent* e);

	void resetMyCam();

	void setLightPosition(double altitude, double azimuth);
	void setLightPosition(GLfloat x, GLfloat y, GLfloat z);

	void mouseTo2D(int x, int y, QVector2D &result);

	void drawScene(int drawMode);
	void selectVertex(RoadGraph &roads, RoadVertexDesc v_desc);
	void selectEdge(RoadGraph &roads, RoadEdgeDesc e_desc);

	void updateCamera();
	void generate2DGeometry();
	void generate3DGeometry(bool justRoads=false);
	void generateBlocks();
	void generateParcels();
	void generateBuildings();

protected:
	void initializeGL();

	void resizeGL(int width, int height);
	void paintGL();    

	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
	void mouseDoubleClickEvent(QMouseEvent *e);



};

