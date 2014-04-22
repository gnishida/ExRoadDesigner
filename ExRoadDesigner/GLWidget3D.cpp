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
/*
#include <render/Terrain.h>
#include <render/SimpleSkyBox.h>
#include <render/RendererHelper.h>
*/
#include "GraphUtil.h"
#include "MainWindow.h"
#include <gl/GLU.h>


GLWidget3D::GLWidget3D(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	camera.resetCamera();
	//camera3D.resetCamera();

	spaceRadius=30000.0;
	farPlaneToSpaceRadiusFactor=5.0f;//N 5.0f

	rotationSensitivity = 0.4f;
	zoomSensitivity = 10.0f;

	controlPressed=false;
	shiftPressed=false;
	altPressed=false;
	keyMPressed=false;

	camera.setRotation(0, 0, 0);
	camera.setTranslation(0, 0, 8000);

	vertexSelected = false;
	edgeSelected = false;
}

GLWidget3D::~GLWidget3D() {
}

QSize GLWidget3D::minimumSizeHint() const {
	return QSize(200, 200);
}

QSize GLWidget3D::sizeHint() const {
	return QSize(400, 400);
}

void GLWidget3D::mousePressEvent(QMouseEvent *event) {
	QVector2D pos;

	this->setFocus();

	lastPos = event->pos();
	mouseTo2D(event->x(), event->y(), pos);

	if (event->buttons() & Qt::LeftButton) {
		switch (mainWin->mode) {
		case MainWindow::MODE_AREA_SELECT:
			if (altPressed) {
				float change = 5;
				if (event->buttons() & Qt::RightButton) {
					change = -change;
				}

				//mainWin->urbanGeometry->terrain->addValue(pos.x(), pos.y(), change);

				updateGL();
			} else {
				//mainWin->urbanGeometry->areas.selectArea(pos);

				// 各エリアについて、マウスポインタの近くに頂点またはエッジがあるかチェックし、あれば、それを選択する
				bool found = false;
				/*
				for (int i = 0; i < mainWin->urbanGeometry->areas.size(); ++i) {
					if (GraphUtil::getVertex(mainWin->urbanGeometry->areas[i]->roads, pos, 10, selectedVertexDesc)) {
						selectVertex(mainWin->urbanGeometry->areas[i]->roads, selectedVertexDesc);
						found = true;
						break;
					} else if (GraphUtil::getEdge(mainWin->urbanGeometry->areas[i]->roads, pos, 10, selectedEdgeDesc)) {
						selectEdge(mainWin->urbanGeometry->areas[i]->roads, selectedEdgeDesc);
						found = true;
						break;
					}
				}

				// どのエリアにも、近くに頂点またはエッジがなければ、roadsをチェック
				if (!found) {
					if (GraphUtil::getVertex(mainWin->urbanGeometry->roads, pos, 10, selectedVertexDesc)) {
						selectVertex(mainWin->urbanGeometry->roads, selectedVertexDesc);
					} else if (GraphUtil::getEdge(mainWin->urbanGeometry->roads, pos, 10, selectedEdgeDesc)) {
						selectEdge(mainWin->urbanGeometry->roads, selectedEdgeDesc);
					} else {
						vertexSelected = false;
						edgeSelected = false;
					}
				}
				*/
			}
			break;
		case MainWindow::MODE_AREA_CREATE:
			/*
			if (!mainWin->urbanGeometry->areaBuilder.selecting()) {
				mainWin->urbanGeometry->areaBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->areaBuilder.selecting()) {
				mainWin->urbanGeometry->areaBuilder.addPoint(pos);
			}

			mainWin->urbanGeometry->areas.selectedIndex = -1;
			*/
			break;
		case MainWindow::MODE_HINT_LINE:
			/*
			// まだエリアが選択されていない場合は、現在のマウス位置を使ってエリアを選択する
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
			*/
			break;
		case MainWindow::MODE_AVENUE_SKETCH:
			/*
			if (!mainWin->urbanGeometry->avenueBuilder.selecting()) {
				mainWin->urbanGeometry->avenueBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (mainWin->urbanGeometry->avenueBuilder.selecting()) {
				mainWin->urbanGeometry->avenueBuilder.addPoint(pos);
			}
			*/
			break;
		case MainWindow::MODE_DEBUG:
			/*
			if (GraphUtil::getVertex(mainWin->urbanGeometry->roads, pos, 10, selectedVertexDesc)) {
				selectedVertex = mainWin->urbanGeometry->roads.graph[selectedVertexDesc];
				mainWin->propertyWidget->setRoadVertex(mainWin->urbanGeometry->roads, selectedVertexDesc, selectedVertex);
				mainWin->propertyWidget->resetRoadEdge();
			} else {
				if (GraphUtil::getEdge(mainWin->urbanGeometry->roads, pos, 10, selectedEdgeDesc)) {
					selectedEdge = mainWin->urbanGeometry->roads.graph[selectedEdgeDesc];
					mainWin->propertyWidget->resetRoadVertex();
					mainWin->propertyWidget->setRoadEdge(selectedEdge);
				}
			}
			*/
			break;
		}
	}
}

void GLWidget3D::mouseReleaseEvent(QMouseEvent *event) {
	event->ignore();

	updateGL();

	return;
}

void GLWidget3D::mouseMoveEvent(QMouseEvent *event) {
	QVector2D pos;
	mouseTo2D(event->x(), event->y(), pos);

	float dx = (float)(event->x() - lastPos.x());
	float dy = (float)(event->y() - lastPos.y());
	float camElevation = camera.getCamElevation();

	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_SELECT:
		if (altPressed) {	// editing
			float change = 5;
			if (event->buttons() & Qt::RightButton) {
				change = -change;
			}
			//mainWin->urbanGeometry->terrain->addValue(pos.x(), pos.y(), change);
			//mainWin->urbanGeometry->adaptToTerrain();
		} else if (event->buttons() & Qt::LeftButton) {	// Rotate
			camera.changeXRotation(rotationSensitivity * dy);
			camera.changeZRotation(-rotationSensitivity * dx);    
			updateCamera();
			lastPos = event->pos();
		} else if (event->buttons() & Qt::MidButton) {
			camera.changeXYZTranslation(-dx, dy, 0);
			updateCamera();
			lastPos = event->pos();
		} else if (event->buttons() & Qt::RightButton) {	// Zoom
			camera.changeXYZTranslation(0, 0, -zoomSensitivity * dy);
			updateCamera();
			lastPos = event->pos();
		}
		break;
	case MainWindow::MODE_AREA_CREATE:
		/*if (mainWin->urbanGeometry->areaBuilder.selecting()) {	// Move the last point of the selected polygonal area
			mainWin->urbanGeometry->areaBuilder.moveLastPoint(pos);
		}
		*/
		break;
	case MainWindow::MODE_HINT_LINE:
		/*
		if (mainWin->urbanGeometry->hintLineBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->hintLineBuilder.moveLastPoint(pos);
		}
		*/
		break;
	case MainWindow::MODE_AVENUE_SKETCH:
		/*
		if (mainWin->urbanGeometry->avenueBuilder.selecting()) {	// Move the last point of the hint line
			mainWin->urbanGeometry->avenueBuilder.moveLastPoint(pos);
		}
		*/

		break;
	}

	updateGL();
}

void GLWidget3D::mouseDoubleClickEvent(QMouseEvent *e) {
	setMouseTracking(false);

	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_CREATE:
		/*
		mainWin->urbanGeometry->areaBuilder.end();
		mainWin->urbanGeometry->areas.add(RoadAreaPtr(new RoadArea(mainWin->urbanGeometry->areaBuilder.polygon())));
		mainWin->urbanGeometry->areas.selectLastArea();
		mainWin->urbanGeometry->areas.selectedArea()->adaptToTerrain(mainWin->urbanGeometry->terrain);

		mainWin->mode = MainWindow::MODE_AREA_SELECT;
		mainWin->ui.actionAreaSelect->setChecked(true);
		mainWin->ui.actionAreaCreate->setChecked(false);
		mainWin->ui.actionHintLine->setChecked(false);
		*/
		break;
	case MainWindow::MODE_HINT_LINE:
		/*
		mainWin->urbanGeometry->hintLineBuilder.end();
		mainWin->urbanGeometry->areas.selectedArea()->hintLine = mainWin->urbanGeometry->hintLineBuilder.polyline();

		mainWin->urbanGeometry->areas.selectedArea()->adaptToTerrain(mainWin->urbanGeometry->terrain);

		mainWin->mode = MainWindow::MODE_AREA_SELECT;
		mainWin->ui.actionAreaSelect->setChecked(true);
		mainWin->ui.actionAreaCreate->setChecked(false);
		mainWin->ui.actionHintLine->setChecked(false);
		*/
		break;
	case MainWindow::MODE_AVENUE_SKETCH:
		/*
		mainWin->urbanGeometry->avenueBuilder.end();

		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_AVENUE, mainWin->urbanGeometry->avenueBuilder.polyline(), 2);

		mainWin->mode = MainWindow::MODE_AREA_SELECT;
		mainWin->ui.actionAreaSelect->setChecked(true);
		mainWin->ui.actionAreaCreate->setChecked(false);
		mainWin->ui.actionHintLine->setChecked(false);
		mainWin->ui.actionAvenueSketch->setChecked(false);
		*/
		updateGL();
		break;
	}
}

void GLWidget3D::initializeGL() {
	/*qglClearColor(QColor(113, 112, 117));
	glClearDepth(1.0f);

	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//---- lighting ----
	glDisable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	static GLfloat lightPosition[4] = { 0.0f, 0.0f, 100.0f, 0.0f };
	static GLfloat lightAmbient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat lightDiffuse[4] = { 0.75f, 0.75f, 0.5f, 1.0f };

	glEnable(GL_COLOR_MATERIAL);	
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	static GLfloat materialSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static GLfloat materialEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, materialSpecular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialEmission);*/
	qglClearColor(QColor(113, 112, 117));
	//qglClearColor(QColor(255, 255, 255));
	////////////////////////////////////////
		//---- GLEW extensions ----
		GLenum err = glewInit();
		if (GLEW_OK != err){// Problem: glewInit failed, something is seriously wrong.
			qDebug() << "Error: " << glewGetErrorString(err);
		}
		qDebug() << "Status: Using GLEW " << glewGetString(GLEW_VERSION);
		if (glewIsSupported("GL_VERSION_4_2"))
			printf("Ready for OpenGL 4.2\n");
		else {
			printf("OpenGL 4.2 not supported\n");
			exit(1);
		}
		const GLubyte* text=
			glGetString(GL_VERSION);
		printf("VERSION: %s\n",text);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glPointSize(10.0f);

		////////////////////////////////
		vboRenderManager.init();
		updateCamera();
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);//SHADOW: Disable

		G::global()["3d_render_mode"]=1;
		// init hatch tex
		if(G::global().getInt("3d_render_mode")==1){//hatch
			std::vector<QString> hatchFiles;
			for(int i=0;i<=6;i++){//5 hatch + perlin + water normals
				hatchFiles.push_back("../data/textures/LC/hatch/h"+QString::number(i)+"b.png");
			}
			for(int i=0;i<=0;i++){//1 win (3 channels)
				hatchFiles.push_back("../data/textures/LC/hatch/win"+QString::number(i)+"b.png");//win0b
				//hatchFiles.push_back("../data/textures/LC/hatch/win_"+QString::number(i)+".png");//win_0
			}
			vboRenderManager.loadArrayTexture("hatching_array",hatchFiles);
		}
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

	drawScene();		
}

void GLWidget3D::drawScene() {
	mainWin->urbanGeometry->render(vboRenderManager);
	/*
	static GLfloat lightPosition[4] = { 10.0f, 100.0f, 100.0f, 0.0f };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	mainWin->urbanGeometry->render(textureManager);

	// draw the selected vertex and edge
	if (vertexSelected) {
		RendererHelper::renderPoint(selectedVertex->pt, QColor(0, 0, 255), selectedVertex->pt3D.z() + 10.0f);
	}
	if (edgeSelected) {
		Polyline3D polyline(selectedEdge->polyline3D);
		for (int i = 0; i < polyline.size(); ++i) polyline[i].setZ(polyline[i].z() + 10.0f);
		RendererHelper::renderPolyline(polyline, QColor(0, 0, 255), GL_LINE_STRIP);
	}

	//Sky
	skyBox->render(&myCam, textureManager);

	//disable depth buffer 
	glDepthFunc(GL_ALWAYS);
	glDisable(GL_TEXTURE_2D);

	glFlush();
	*/
}

/**
 * 頂点を選択する
 */
/*void GLWidget3D::selectVertex(RoadGraph &roads, RoadVertexDesc v_desc) {
	selectedVertex = roads.graph[v_desc];
	mainWin->propertyWidget->setRoadVertex(roads, v_desc, selectedVertex);
	mainWin->propertyWidget->resetRoadEdge();

	vertexSelected = true;
	edgeSelected = false;
}

void GLWidget3D::selectEdge(RoadGraph &roads, RoadEdgeDesc e_desc) {
	selectedEdge = roads.graph[e_desc];
	mainWin->propertyWidget->resetRoadVertex();
	mainWin->propertyWidget->setRoadEdge(selectedEdge);

	vertexSelected = false;
	edgeSelected = true;
}
*/
void GLWidget3D::keyPressEvent( QKeyEvent *e ){
	//printf("k\n");
	/*float sensitivityFactor;
	float camElevation = camera.getCamElevation();
	sensitivityFactor = (0.01f+0.01*camElevation)*spaceRadius*0.001;
	*/

	shiftPressed=false;
	controlPressed=false;
	altPressed=false;
	keyMPressed=false;

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
		break;
	case Qt::Key_Escape:
		//mainWin->urbanGeometry->areaBuilder.cancel();
		updateGL();
		break;
	case Qt::Key_R:
		printf("Reseting camera pose\n");
		camera.resetCamera();
		break;
	case Qt::Key_W:
		camera.moveKey(0);updateCamera();updateGL();
		break;
	case Qt::Key_S:
		camera.moveKey(1);updateCamera();updateGL();
		break;
	case Qt::Key_D:
		camera.moveKey(2);updateCamera();updateGL();
		break;
	case Qt::Key_A:
		camera.moveKey(3);updateCamera();updateGL();
		break;
	case Qt::Key_Q:
		camera.moveKey(4);updateCamera();updateGL();
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
	default:
		;
	}
}

void GLWidget3D::mouseTo2D(int x,int y, QVector2D &result) {
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];

	// retrieve the matrices
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	// retrieve the projected z-buffer of the origin
	GLdouble origX, origY, origZ;
	gluProject(0, 0, 0, modelview, projection, viewport, &origX, &origY, &origZ);

	// set up the projected point
	GLfloat winX = (float)x;
	GLfloat winY = (float)viewport[3] - (float)y;
	GLfloat winZ = origZ;
	
	// unproject the image plane coordinate to the model space
	GLdouble posX, posY, posZ;
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	result.setX(posX);
	result.setY(posY);
}

// this method should be called after any camera transformation (perspective or modelview)
// it will update viewport, perspective, view matrix, and update the uniforms
void GLWidget3D::updateCamera(){
	// update matrices
	int height = this->height() ? this->height() : 1;
	glViewport(0, 0, (GLint)this->width(), (GLint)this->height());
	camera.updatePerspective(this->width(),height);
	camera.updateCamMatrix();
	if(G::global()["rend_mode"]==1)
		camera.updateCamMatrix();
	// update uniforms
	float mvpMatrixArray[16];
	float mvMatrixArray[16];

	for(int i=0;i<16;i++){
		mvpMatrixArray[i]=camera.mvpMatrix.data()[i];
		mvMatrixArray[i]=camera.mvMatrix.data()[i];	
	}
	float normMatrixArray[9];
	for(int i=0;i<9;i++){
		normMatrixArray[i]=camera.normalMatrix.data()[i];
	}

	//glUniformMatrix4fv(mvpMatrixLoc,  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvpMatrix"),  1, false, mvpMatrixArray);
	glUniformMatrix4fv(glGetUniformLocation(vboRenderManager.program, "mvMatrix"),  1, false, mvMatrixArray);
	glUniformMatrix3fv(glGetUniformLocation(vboRenderManager.program, "normalMatrix"),  1, false, normMatrixArray);

	// light poss
	QVector3D light_position=-camera.light_dir.toVector3D();
	glUniform3f(glGetUniformLocation(vboRenderManager.program, "lightPosition"),light_position.x(),light_position.y(),light_position.z());
}//