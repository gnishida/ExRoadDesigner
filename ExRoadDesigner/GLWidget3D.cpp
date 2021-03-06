﻿/*********************************************************************
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

#include "GLWidget3D.h"
#include "Util.h"
#include "GraphUtil.h"
#include "MainWindow.h"
#include <gl/GLU.h>
#include "RendererHelper.h"
#include "VBORoadGraph.h"
#include "VBOPm.h"
#include "BSpline.h"
#include "RoadGeneratorHelper.h"
#include "BlockMeshGenerator.h"

GLWidget3D::GLWidget3D(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	camera2D.resetCamera();
	flyCamera.resetCamera();
	camera = &camera2D;
	//camera = &flyCamera;

	spaceRadius=30000.0;
	farPlaneToSpaceRadiusFactor=5.0f;//N 5.0f

	rotationSensitivity = 0.4f;
	zoomSensitivity = 10.0f;

	controlPressed=false;
	shiftPressed=false;
	altPressed=false;

	camera->setRotation(0, 0, 0);
	camera->setTranslation(0, 0, G::getFloat("MAX_Z"));//6000);

	vertexSelected = false;
	edgeSelected = false;
}

QSize GLWidget3D::minimumSizeHint() const {
	return QSize(200, 200);
}

QSize GLWidget3D::sizeHint() const {
	return QSize(400, 400);
}

void GLWidget3D::mousePressEvent(QMouseEvent *event) {
	QVector2D pos;

	if (Qt::ControlModifier == event->modifiers()) {
		controlPressed = true;
	} else {
		controlPressed = false;
	}

	this->setFocus();

	lastPos = event->pos();
	mouseTo2D(event->x(), event->y(), pos);

	if (event->buttons() /*& Qt::LeftButton*/) {
		switch (mainWin->mode) {
		case MainWindow::MODE_AREA_SELECT:
			if (altPressed) {
				if(mainWin->controlWidget->ui.content_checkbox->isChecked()){
					// content edition
					float xM = pos.x() / vboRenderManager.side + 0.5f;
					float yM = pos.y() / vboRenderManager.side + 0.5f;
					float newValue = G::global().getFloat("content_terrainLevel");
					float radi = mainWin->controlWidget->ui.terrainPaint_sizeSlider->value() / vboRenderManager.side;

					vboRenderManager.vboTerrain.excavate(xM, yM, newValue,radi) ;
					shadow.makeShadowMap(this);
					updateGL();
				}else{
					// normal Gaussian edition
					float height = mainWin->controlWidget->ui.terrainPaint_changeSlider->value();
					float xM = pos.x() / vboRenderManager.side + 0.5f;
					float yM = pos.y() / vboRenderManager.side + 0.5f;
					float radi = mainWin->controlWidget->ui.terrainPaint_sizeSlider->value() / vboRenderManager.side;

					if (event->buttons() & Qt::LeftButton) {
						vboRenderManager.vboTerrain.updateGaussian(xM, yM, height, radi);
					} else if (event->buttons() & Qt::RightButton) {
						vboRenderManager.vboTerrain.updateGaussian(xM, yM, -height, radi);
					} else if (event->buttons() & Qt::MiddleButton) {
						vboRenderManager.vboTerrain.excavate(xM, yM, height, radi);
					}

					shadow.makeShadowMap(this);
					updateGL();
				}
			} else if (controlPressed) {
				if (mainWin->urbanGeometry->areas.selectedIndex >= 0) {
					mainWin->urbanGeometry->areas.selectedArea()->selectAreaPoint(pos);
				}
			} else {
				mainWin->urbanGeometry->areas.selectArea(pos);

				// select a vertex or an edge
				bool found = false;
				
				if (shiftPressed) {	// select am edge
					for (int i = 0; i < mainWin->urbanGeometry->areas.size(); ++i) {
						if (GraphUtil::getEdge(mainWin->urbanGeometry->areas[i]->roads, pos, 30, selectedEdgeDesc)) {
							selectEdge(mainWin->urbanGeometry->areas[i]->roads, selectedEdgeDesc);
							found = true;
							break;
						}
					}

					// select a vertex or an edge
					if (!found) {
						if (GraphUtil::getEdge(mainWin->urbanGeometry->roads, pos, 30, selectedEdgeDesc)) {
							selectEdge(mainWin->urbanGeometry->roads, selectedEdgeDesc);
						} else {
							vertexSelected = false;
							edgeSelected = false;
						}
					}
				} else {	// select a vertex
					for (int i = 0; i < mainWin->urbanGeometry->areas.size(); ++i) {
						if (GraphUtil::getVertex(mainWin->urbanGeometry->areas[i]->roads, pos, 30, selectedVertexDesc)) {
							selectVertex(mainWin->urbanGeometry->areas[i]->roads, selectedVertexDesc);
							found = true;
							break;
						}
					}

					// select a vertex or an edge
					if (!found) {
						if (GraphUtil::getVertex(mainWin->urbanGeometry->roads, pos, 30, selectedVertexDesc)) {
							selectVertex(mainWin->urbanGeometry->roads, selectedVertexDesc);
						} else {
							vertexSelected = false;
							edgeSelected = false;
						}
					}
				}
			}
			break;
		case MainWindow::MODE_AREA_CREATE:
			if (!mainWin->urbanGeometry->areaBuilder.selecting()) {
				mainWin->urbanGeometry->areaBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->areaBuilder.selecting()) {
				mainWin->urbanGeometry->areaBuilder.addPoint(pos);
			}

			mainWin->urbanGeometry->areas.selectedIndex = -1;
			
			break;
		case MainWindow::MODE_HINT_LINE:
			// select an area
			if (mainWin->urbanGeometry->areas.selectedIndex == -1) {
				mainWin->urbanGeometry->areas.selectArea(pos);
			}

			if (!mainWin->urbanGeometry->hintLineBuilder.selecting()) {
				mainWin->urbanGeometry->hintLineBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->hintLineBuilder.selecting()) {
				mainWin->urbanGeometry->hintLineBuilder.addPoint(pos);
			}
			
			break;
		case MainWindow::MODE_HIGHWAY_SKETCH:
			if (!mainWin->urbanGeometry->highwayBuilder.selecting()) {
				mainWin->urbanGeometry->highwayBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->highwayBuilder.selecting()) {
				mainWin->urbanGeometry->highwayBuilder.addPoint(pos);
			}
			
			break;
		case MainWindow::MODE_AVENUE_SKETCH:
			if (!mainWin->urbanGeometry->avenueBuilder.selecting()) {
				mainWin->urbanGeometry->avenueBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->avenueBuilder.selecting()) {
				mainWin->urbanGeometry->avenueBuilder.addPoint(pos);
			}
			
			break;
		case MainWindow::MODE_STREET_SKETCH:
			if (!mainWin->urbanGeometry->streetBuilder.selecting()) {
				mainWin->urbanGeometry->streetBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->streetBuilder.selecting()) {
				mainWin->urbanGeometry->streetBuilder.addPoint(pos);
			}
			
			break;
		case MainWindow::MODE_CONTROL_POINTS:
			// select an area
			if (mainWin->urbanGeometry->areas.selectedIndex == -1) {
				mainWin->urbanGeometry->areas.selectArea(pos);
			}

			if (controlPressed) {
				float min_dist = std::numeric_limits<float>::max();
				int min_index;
				for (int i = 0; i < mainWin->urbanGeometry->controlPoints.size(); ++i) {
					float dist = (pos - mainWin->urbanGeometry->controlPoints[i]).lengthSquared();
					if (dist < min_dist) {
						min_dist = dist;
						min_index = i;
					}
				}

				if (min_dist < 10000) {
					mainWin->urbanGeometry->controlPointSelected = true;
					mainWin->urbanGeometry->controlPointSelectedIndex = min_index;
					GraphUtil::copyRoads(mainWin->urbanGeometry->roads, mainWin->urbanGeometry->origRoads);
					mainWin->urbanGeometry->origControlPoints = mainWin->urbanGeometry->controlPoints;
				}
			} else {
				if (!mainWin->urbanGeometry->controlPointsBuilder.selecting()) {
					mainWin->urbanGeometry->controlPointsBuilder.start(pos);
					setMouseTracking(true);
				}
		
				if (mainWin->urbanGeometry->controlPointsBuilder.selecting()) {
					mainWin->urbanGeometry->controlPointsBuilder.addPoint(pos);
				}
			}
			
			break;
		}
	}
}

void GLWidget3D::mouseReleaseEvent(QMouseEvent *event) {
	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_SELECT:
		if (controlPressed) {
			if (mainWin->urbanGeometry->areas.selectedIndex >= 0) {
				mainWin->urbanGeometry->areas.selectedArea()->areaPointSelected = false;
			}
		}
		break;
	}

	updateGL();

	return;
}

void GLWidget3D::mouseMoveEvent(QMouseEvent *event) {
	QVector2D pos;
	mouseTo2D(event->x(), event->y(), pos);

	float dx = (float)(event->x() - lastPos.x());
	float dy = (float)(event->y() - lastPos.y());
	float camElevation = camera->getCamElevation();

	vboRenderManager.mousePos3D=pos.toVector3D();

	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_SELECT:
		if (altPressed) {	// editing
			if (event->buttons() & Qt::RightButton||event->buttons() & Qt::LeftButton||event->buttons() & Qt::MiddleButton) {//make sure something is clicking
				if(mainWin->controlWidget->ui.content_checkbox->isChecked()){
					// content edition
					float xM = pos.x() / vboRenderManager.side + 0.5f;
					float yM = pos.y() / vboRenderManager.side + 0.5f;
					float newValue = G::global().getFloat("content_terrainLevel");
					float radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value() / vboRenderManager.side;

					vboRenderManager.vboTerrain.excavate(xM, yM, newValue, radi);
					shadow.makeShadowMap(this);
				} else {
					// normal Gaussian edition
					float height = mainWin->controlWidget->ui.terrainPaint_changeSlider->value();
					float radi = mainWin->controlWidget->ui.terrainPaint_sizeSlider->value() / vboRenderManager.side;
					float xM = pos.x() / vboRenderManager.side + 0.5f;
					float yM = pos.y() / vboRenderManager.side + 0.5f;

					if (event->buttons() & Qt::LeftButton) {
						vboRenderManager.vboTerrain.updateGaussian(xM, yM, height * 0.2f, radi);
					} else if (event->buttons() & Qt::RightButton) {
						vboRenderManager.vboTerrain.updateGaussian(xM, yM, -height * 0.2f, radi);
					} else if (event->buttons() & Qt::MiddleButton) {
						vboRenderManager.vboTerrain.excavate(xM, yM, height, radi);
					}

					shadow.makeShadowMap(this);
				}
			}
		} else if (controlPressed) {
			if (mainWin->urbanGeometry->areas.selectedIndex >= 0) {
				mainWin->urbanGeometry->areas.selectedArea()->updateAreaPoint(pos);
				mainWin->urbanGeometry->areas.selectedArea()->adaptToTerrain(&vboRenderManager);
			}
		} else if (event->buttons() & Qt::LeftButton) {	// Rotate
			camera->changeXRotation(rotationSensitivity * dy);
			camera->changeZRotation(-rotationSensitivity * dx);    
			updateCamera();
			lastPos = event->pos();
		} else if (event->buttons() & Qt::MidButton) {
			camera->changeXYZTranslation(-dx, dy, 0);
			updateCamera();
			lastPos = event->pos();
		} else if (event->buttons() & Qt::RightButton) {	// Zoom
			camera->changeXYZTranslation(0, 0, -zoomSensitivity * dy);
			updateCamera();
			lastPos = event->pos();
		}
		break;
	case MainWindow::MODE_AREA_CREATE:
		if (mainWin->urbanGeometry->areaBuilder.selecting()) {	// Move the last point of the selected polygonal area
			mainWin->urbanGeometry->areaBuilder.moveLastPoint(pos);
		}
		break;
	case MainWindow::MODE_HINT_LINE:
		if (mainWin->urbanGeometry->hintLineBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->hintLineBuilder.moveLastPoint(pos);
		}
		break;
	case MainWindow::MODE_HIGHWAY_SKETCH:
		if (mainWin->urbanGeometry->highwayBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->highwayBuilder.moveLastPoint(pos);
		}
		break;
	case MainWindow::MODE_AVENUE_SKETCH:
		if (mainWin->urbanGeometry->avenueBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->avenueBuilder.moveLastPoint(pos);
		}
		break;
	case MainWindow::MODE_STREET_SKETCH:
		if (mainWin->urbanGeometry->streetBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->streetBuilder.moveLastPoint(pos);
		}
		break;
	case MainWindow::MODE_CONTROL_POINTS:
		if (controlPressed) {
			if (mainWin->urbanGeometry->controlPointSelected) {
				mainWin->urbanGeometry->controlPoints[mainWin->urbanGeometry->controlPointSelectedIndex] = pos;

				//RoadGeneratorHelper::elasticTransform(mainWin->urbanGeometry->areas.selectedArea()->origRoads, BSpline::spline(mainWin->urbanGeometry->areas.selectedArea()->origControlPoints), BSpline::spline(mainWin->urbanGeometry->areas.selectedArea()->controlPoints), mainWin->urbanGeometry->areas.selectedArea()->roads);
			}
		} else {
			if (mainWin->urbanGeometry->controlPointsBuilder.selecting()) {	// Move the last point of the hint line
				mainWin->urbanGeometry->controlPointsBuilder.moveLastPoint(pos);
			}
		}
		break;
	}

	updateGL();
}

void GLWidget3D::mouseDoubleClickEvent(QMouseEvent *e) {
	setMouseTracking(false);

	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_CREATE:
		mainWin->urbanGeometry->areaBuilder.end();
		mainWin->urbanGeometry->areas.add(RoadAreaPtr(new RoadArea(mainWin->urbanGeometry->areaBuilder.polygon())));
		mainWin->urbanGeometry->areas.selectLastArea();
		mainWin->urbanGeometry->areas.selectedArea()->adaptToTerrain(&vboRenderManager);

		mainWin->mode = MainWindow::MODE_AREA_SELECT;
		mainWin->ui.actionAreaSelect->setChecked(true);
		mainWin->ui.actionAreaCreate->setChecked(false);

		break;
	case MainWindow::MODE_HINT_LINE:
		mainWin->urbanGeometry->hintLineBuilder.end();
		mainWin->urbanGeometry->areas.selectedArea()->hintLine = mainWin->urbanGeometry->hintLineBuilder.polyline();

		mainWin->urbanGeometry->areas.selectedArea()->adaptToTerrain(&vboRenderManager);

		mainWin->mode = MainWindow::MODE_AREA_SELECT;
		mainWin->ui.actionAreaSelect->setChecked(true);
		mainWin->ui.actionHintLine->setChecked(false);
		
		break;
	case MainWindow::MODE_HIGHWAY_SKETCH:
		mainWin->urbanGeometry->highwayBuilder.end();
		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_HIGHWAY, mainWin->urbanGeometry->highwayBuilder.polyline(), 3);
		mainWin->urbanGeometry->update(vboRenderManager);
		
		break;
	case MainWindow::MODE_AVENUE_SKETCH:
		mainWin->urbanGeometry->avenueBuilder.end();
		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_AVENUE, mainWin->urbanGeometry->avenueBuilder.polyline(), 2);
		mainWin->urbanGeometry->update(vboRenderManager);

		break;
	case MainWindow::MODE_STREET_SKETCH:
		mainWin->urbanGeometry->streetBuilder.end();
		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_STREET, mainWin->urbanGeometry->streetBuilder.polyline(), 1);
		mainWin->urbanGeometry->update(vboRenderManager);
		
		break;
	case MainWindow::MODE_CONTROL_POINTS:
		mainWin->urbanGeometry->controlPointsBuilder.end();
		mainWin->urbanGeometry->controlPoints = mainWin->urbanGeometry->controlPointsBuilder.polyline();
		
		break;
	}
}

void GLWidget3D::initializeGL() {
	//qglClearColor(QColor(113, 112, 117));
	qglClearColor(QColor(0, 0, 0));

	//---- GLEW extensions ----
	GLenum err = glewInit();
	if (GLEW_OK != err){// Problem: glewInit failed, something is seriously wrong.
		qDebug() << "Error: " << glewGetErrorString(err);
	}
	qDebug() << "Status: Using GLEW " << glewGetString(GLEW_VERSION);
	if (glewIsSupported("GL_VERSION_4_2")) {
		printf("Ready for OpenGL 4.2\n");
	} else {
		printf("OpenGL 4.2 not supported\n");
		exit(1);
	}
	const GLubyte* text= glGetString(GL_VERSION);
	printf("VERSION: %s\n",text);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glLineWidth(5.0f);
	glPointSize(10.0f);

	///
	vboRenderManager.init();
	updateCamera();
	shadow.initShadow(vboRenderManager.program,this);
	glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);//SHADOW: Disable
	glUniform1i(glGetUniformLocation(vboRenderManager.program, "terrainMode"), 0);//FLAT
}

void GLWidget3D::resizeGL(int width, int height) {
	updateCamera();
}

void GLWidget3D::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);
	
	// NOTE: camera transformation is not necessary here since the updateCamera updates the uniforms each time they are changed

	drawScene(0);		
}

void GLWidget3D::drawScene(int drawMode) {
	///////////////////////////////////
	// GEN MODE
	//if (G::getInt("3d_render_mode") == 0) {
	if (G::getBool("shader2D")) {
		mainWin->urbanGeometry->render(vboRenderManager);
		
		vboRenderManager.vboTerrain.render(true);
		glLineWidth(G::global().getFloat("2d_parcelLine"));
		vboRenderManager.renderStaticGeometry("3d_blocks");
		vboRenderManager.renderStaticGeometry("3d_parks");
		vboRenderManager.renderStaticGeometry("3d_parcels");
		vboRenderManager.renderStaticGeometry("3d_roads");
		vboRenderManager.renderStaticGeometry("3d_roads_inter");
		vboRenderManager.renderStaticGeometry("3d_roads_interCom");

		// draw the selected vertex and edge
		if (vertexSelected) {
			RendererHelper::renderPoint(vboRenderManager, selectedVertex->pt, QColor(0, 0, 255), selectedVertex->pt3D.z() + 2.0f);
		}
		if (edgeSelected) {
			Polyline3D polyline;
			for (int i = 0; i < selectedEdge->polyline.size(); ++i) {
				float z = vboRenderManager.getTerrainHeight(selectedEdge->polyline[i].x(), selectedEdge->polyline[i].y());
				polyline.push_back(QVector3D(selectedEdge->polyline[i].x(), selectedEdge->polyline[i].y(), z + 10.0f));
			}
			RendererHelper::renderPolyline(vboRenderManager, polyline, QColor(0, 0, 255));
		}
	} else {
		// NORMAL
		if (drawMode == 0) {
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);
			vboRenderManager.renderStaticGeometry("sky");
			vboRenderManager.vboWater.render(vboRenderManager);
			mainWin->urbanGeometry->render(vboRenderManager);
			
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 1);
			vboRenderManager.vboTerrain.render(true);
			vboRenderManager.renderStaticGeometry("3d_blocks");
			vboRenderManager.renderStaticGeometry("3d_parks");
			vboRenderManager.renderStaticGeometry("3d_parcels");
			vboRenderManager.renderStaticGeometry("3d_roads");
			vboRenderManager.renderStaticGeometry("3d_roads_inter");
			vboRenderManager.renderStaticGeometry("3d_roads_interCom");
			vboRenderManager.renderStaticGeometry("3d_building");
			vboRenderManager.renderStaticGeometry("3d_building_fac");

			vboRenderManager.renderAllStreetElementName("tree");
			vboRenderManager.renderAllStreetElementName("streetLamp");
		}
		// SHADOWS
		if (drawMode == 1) {
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 2);// SHADOW: From light

			vboRenderManager.vboTerrain.render(false);

			//vboRenderManager.renderStaticGeometry("3d_blocks");
			//vboRenderManager.renderStaticGeometry("3d_parks");
			//vboRenderManager.renderStaticGeometry("3d_parcels");
			//vboRenderManager.renderStaticGeometry("3d_roads");
			//vboRenderManager.renderStaticGeometry("3d_roads_inter");
			//vboRenderManager.renderStaticGeometry("3d_roads_interCom");
			vboRenderManager.renderStaticGeometry("3d_building");
			vboRenderManager.renderStaticGeometry("3d_building_fac");

			vboRenderManager.renderAllStreetElementName("tree");
			vboRenderManager.renderAllStreetElementName("streetLamp");
		}
	}
}

/**
 * select a vertex
 */
void GLWidget3D::selectVertex(RoadGraph &roads, RoadVertexDesc v_desc) {
	selectedVertex = roads.graph[v_desc];
	mainWin->propertyWidget->setRoadVertex(roads, v_desc, selectedVertex);
	mainWin->propertyWidget->resetRoadEdge();

	vertexSelected = true;
	edgeSelected = false;
}

void GLWidget3D::selectEdge(RoadGraph &roads, RoadEdgeDesc e_desc) {
	selectedEdge = roads.graph[e_desc];
	mainWin->propertyWidget->resetRoadVertex();
	mainWin->propertyWidget->setRoadEdge(roads, e_desc, selectedEdge);

	vertexSelected = false;
	edgeSelected = true;
}

void GLWidget3D::keyPressEvent( QKeyEvent *e ){
	shiftPressed=false;
	controlPressed=false;
	altPressed=false;

	switch( e->key() ){
	case Qt::Key_Shift:
		shiftPressed=true;
		//clientMain->statusBar()->showMessage("Shift pressed");
		break;
	case Qt::Key_Control:
		controlPressed=true;
		break;
	case Qt::Key_Alt:
		altPressed=true;
		vboRenderManager.editionMode=true;
		updateGL();
		setMouseTracking(true);
		break;
	case Qt::Key_Escape:
		mainWin->urbanGeometry->areaBuilder.cancel();
		updateGL();
		break;
	case Qt::Key_Delete:
		if (edgeSelected) {
			selectedEdge->valid = false;
			mainWin->urbanGeometry->update(vboRenderManager);
			edgeSelected = false;
			updateGL();
		}
		break;
	case Qt::Key_R:
		printf("Reseting camera pose\n");
		camera->resetCamera();
		break;
	default:
		;
	}
}

void GLWidget3D::keyReleaseEvent(QKeyEvent* e) {
	if (e->isAutoRepeat()) {
		e->ignore();
		return;
	}
	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed=false;
		break;
	case Qt::Key_Control:
		controlPressed=false;
		break;
	case Qt::Key_Alt:
		altPressed=false;
		vboRenderManager.editionMode=false;
		setMouseTracking(false);
		updateGL();
	default:
		;
	}
}

/**
 * Convert the screen space coordinate (x, y) to the model space coordinate.
 */
void GLWidget3D::mouseTo2D(int x,int y, QVector2D &result) {
	updateCamera();
	updateGL();
	GLint viewport[4];

	// retrieve the matrices
	glGetIntegerv(GL_VIEWPORT, viewport);

	// retrieve the projected z-buffer of the origin
	GLfloat winX,winY,winZ;
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	/*glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
	GLdouble posX, posY, posZ;
	gluUnProject( winX, winY, winZ, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &posX, &posY, &posZ);
	// unproject the image plane coordinate to the model space
	
	
		float znear=10.0f,zFar= 10.0f*2000.0f;
		
			GLdouble posXFar, posYFar, posZFar;
			gluUnProject( winX, winY, zFar, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &posXFar, &posYFar, &posZFar);
			QVector3D rayStar(posX,posY,posZ);
			QVector3D rayEnd(posXFar,posYFar,posZFar);
			double t;
			QVector3D q1(0,0,1.0f);
			QVector3D q2(0,0,0);
			QVector3D result3D;
			printf("mouse %d %d win %f %f z1 %f z2 %f Pos %f %f %f PosF %f %f %f\n",x,y,winX,winY,winZ,zFar,posX,posY,posZ,posXFar,posYFar,posZFar);
			if(Util::planeIntersectWithLine(rayStar,rayEnd,q1,q2,t,result3D)!=0){
				result->setX(result3D.x());
				result->setY(result3D.y());
			}else{
				printf("fail hit\n");
			}
			//return;
	
	result->setX(posX);
	result->setY(posY);*/
	GLdouble wx, wy, wz;  /*  returned world x, y, z coords  */
	GLdouble wx2, wy2, wz2;  /*  returned world x, y, z coords  */
	gluUnProject( winX, winY, 0.0f, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &wx, &wy, &wz);
	gluUnProject( winX, winY, 1.0f, camera->mvMatrix.data(), camera->pMatrix.data(), viewport, &wx2, &wy2, &wz2);
	double f = wz / ( wz2 - wz );
	double x2d = wx - f * (wx2 - wx );
	double y2d = wy - f * (wy2 - wy );	
	result.setX(x2d);
	result.setY(y2d);
	//printf("Mouse %d %d\n",x,y);
}

// this method should be called after any camera transformation (perspective or modelview)
// it will update viewport, perspective, view matrix, and update the uniforms
void GLWidget3D::updateCamera(){
	// update matrices
	int height = this->height() ? this->height() : 1;
	glViewport(0, 0, (GLint)this->width(), (GLint)this->height());
	camera->updatePerspective(this->width(),height);
	camera->updateCamMatrix();

	// update uniforms
	float mvpMatrixArray[16];
	float mvMatrixArray[16];

	for(int i=0;i<16;i++){
		mvpMatrixArray[i]=camera->mvpMatrix.data()[i];
		mvMatrixArray[i]=camera->mvMatrix.data()[i];	
	}
	float normMatrixArray[9];
	for(int i=0;i<9;i++){
		normMatrixArray[i]=camera->normalMatrix.data()[i];
	}

	//glUniformMatrix4fv(mvpMatrixLoc,  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvpMatrix"),  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvMatrix"),  1, false, mvMatrixArray);
	glUniformMatrix3fv(glGetUniformLocation(vboRenderManager.program, "normalMatrix"),  1, false, normMatrixArray);

	// light direction
	QVector3D light_dir(-0.40f, 0.81f, -0.51f);
	glUniform3f(glGetUniformLocation(vboRenderManager.program, "lightDir"), light_dir.x(), light_dir.y(), light_dir.z());
}
