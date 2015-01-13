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

GLWidget3D::GLWidget3D(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	camera2D.resetCamera();
	flyCamera.resetCamera();
	camera3D.resetCamera();
	camera = &camera2D;
	//camera = &flyCamera;
	//G::global()["rend_mode"]=0;//2D	setRender2D_3D();

	spaceRadius=30000.0;
	farPlaneToSpaceRadiusFactor=5.0f;//N 5.0f

	rotationSensitivity = 0.4f;
	zoomSensitivity = 10.0f;

	controlPressed=false;
	shiftPressed=false;
	altPressed=false;
	keyMPressed=false;

	camera->setRotation(0, 0, 0);
	camera->setTranslation(0, 0, G::getFloat("MAX_Z"));//6000);

	vertexSelected = false;
	edgeSelected = false;

	shadowEnabled=true;
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
					float xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
					float yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
					float newValue=0;
					newValue=G::global().getFloat("content_terrainLevel");

					float radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
					vboRenderManager.vboTerrain.updateTerrainNewValue(xM,yM,newValue,radi);
					mainWin->urbanGeometry->adaptToTerrain();
					shadow.makeShadowMap(this);
					updateGL();
				}else{
					// normal Gaussian edition
					float change=mainWin->controlWidget->ui.terrainPaint_changeSlider->value()*0.003f;
					float radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
					if (event->buttons() & Qt::RightButton) {
						change = -change;
					}
					if (event->buttons() & Qt::MiddleButton) {
						change=FLT_MAX;//hack: flat terrain
					}
					//mainWin->urbanGeometry->vboRenderManager->addValue(pos.x(), pos.y(), change);
					float xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
					float yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
					vboRenderManager.vboTerrain.updateTerrain(xM,yM,change,radi);//rad,change);
					mainWin->urbanGeometry->adaptToTerrain();/// !! GEN did not have it here (enough in move?)
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
	case MainWindow::MODE_CONTROL_POINTS:
		if (controlPressed) {
			//RoadGraph tempRoads;
			//GraphUtil::copyRoads(mainWin->urbanGeometry->roads, tempRoads);
			RoadGeneratorHelper::elasticTransform(mainWin->urbanGeometry->origRoads, mainWin->urbanGeometry->origControlPoints, mainWin->urbanGeometry->controlPoints, mainWin->urbanGeometry->roads);

			mainWin->urbanGeometry->adaptToTerrain();
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
		/*if(altPressed==false&&G::global()["3d_render_mode"]==1){
			camera3D.motion(dx,dy);
			lastPos = event->pos();
			break;
		}*/
		if (altPressed) {	// editing
			if (event->buttons() & Qt::RightButton||event->buttons() & Qt::LeftButton||event->buttons() & Qt::MiddleButton) {//make sure something is clicking
				if(mainWin->controlWidget->ui.content_checkbox->isChecked()){
					// content edition
					float xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
					float yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
					float newValue=0;
					newValue=G::global().getFloat("content_terrainLevel");

					float radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
					vboRenderManager.vboTerrain.updateTerrainNewValue(xM,yM,newValue,radi);
					mainWin->urbanGeometry->adaptToTerrain();
					shadow.makeShadowMap(this);
					
				}else{
					// normal Gaussian edition
					float change=mainWin->controlWidget->ui.terrainPaint_changeSlider->value()*0.003f;
					float radi=mainWin->controlWidget->ui.terrainPaint_sizeSlider->value()*0.01f;
					change*=0.2f;//while moving, it is not necessary to change much
					if (event->buttons() & Qt::RightButton) {
						change = -change;
					}
					if (event->buttons() & Qt::MiddleButton) {
						change=FLT_MAX;//hack: flat terrain
					}
					//mainWin->urbanGeometry->vboRenderManager->addValue(pos.x(), pos.y(), change);
					float xM=1.0f-(vboRenderManager.side/2.0f-pos.x())/vboRenderManager.side;
					float yM=1.0f-(vboRenderManager.side/2.0f-pos.y())/vboRenderManager.side;
					vboRenderManager.vboTerrain.updateTerrain(xM,yM,change,radi);//rad,change);

					mainWin->urbanGeometry->adaptToTerrain();
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
				mainWin->urbanGeometry->adaptToTerrain();

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
		
		break;
	case MainWindow::MODE_AVENUE_SKETCH:
		mainWin->urbanGeometry->avenueBuilder.end();
		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_AVENUE, mainWin->urbanGeometry->avenueBuilder.polyline(), 2);
		
		break;
	case MainWindow::MODE_STREET_SKETCH:
		mainWin->urbanGeometry->streetBuilder.end();
		mainWin->urbanGeometry->addRoad(RoadEdge::TYPE_STREET, mainWin->urbanGeometry->streetBuilder.polyline(), 1);
		
		break;
	case MainWindow::MODE_CONTROL_POINTS:
		mainWin->urbanGeometry->controlPointsBuilder.end();
		mainWin->urbanGeometry->controlPoints = mainWin->urbanGeometry->controlPointsBuilder.polyline();

		mainWin->urbanGeometry->adaptToTerrain();
		
		break;
	}
}

void GLWidget3D::initializeGL() {

	//qglClearColor(QColor(113, 112, 117));
	qglClearColor(QColor(0, 0, 0));
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
			//exit(1);
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

		G::global()["3d_render_mode"]=0;
		// init hatch tex
		std::vector<QString> hatchFiles;
		for(int i=0;i<=6;i++){//5 hatch + perlin + water normals
			hatchFiles.push_back("../data/textures/LC/hatch/h"+QString::number(i)+"b.png");
		}
		for(int i=0;i<=0;i++){//1 win (3 channels)
			hatchFiles.push_back("../data/textures/LC/hatch/win"+QString::number(i)+"b.png");//win0b
		}
		vboRenderManager.loadArrayTexture("hatching_array",hatchFiles);


		///
		vboRenderManager.init();
		updateCamera();
		shadow.initShadow(vboRenderManager.program,this);
		glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);//SHADOW: Disable
		glUniform1i(glGetUniformLocation(vboRenderManager.program, "terrainMode"),0);//FLAT
		
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
	if(G::global().getInt("3d_render_mode")==0){
		//vboRenderManager.renderStaticGeometry(QString("sky"));

		glDisable(GL_CULL_FACE);
		mainWin->urbanGeometry->render(vboRenderManager);
		glEnable(GL_CULL_FACE);
		
		vboRenderManager.vboTerrain.render(vboRenderManager);
		if(mainWin->controlWidget->ui.terrain_2DShader->isChecked()==false)
			vboRenderManager.vboWater.render(vboRenderManager);

		// draw the selected vertex and edge
		if (vertexSelected) {
			RendererHelper::renderPoint(vboRenderManager, "selected_vertex", selectedVertex->pt, QColor(0, 0, 255), selectedVertex->pt3D.z() + 2.0f);
		}
		if (edgeSelected) {
			Polyline3D polyline(selectedEdge->polyline3D);
			for (int i = 0; i < polyline.size(); ++i) polyline[i].setZ(polyline[i].z() + 10.0f);
			RendererHelper::renderPolyline(vboRenderManager, "selected_edge_lines", "selected_edge_points", polyline, QColor(0, 0, 255));
		}

	}
	///////////////////////////////////
	// LC MODE
	if(G::global().getInt("3d_render_mode")==1||G::global().getInt("3d_render_mode")==2){//LC HATCH

		// NORMAL
		if(drawMode==0){
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 0);//SHADOW: Disable

			// RENDER SKY WATER

			vboRenderManager.renderStaticGeometry(QString("sky"));
			vboRenderManager.vboWater.render(vboRenderManager);

			

			if(shadowEnabled)
				glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 1);//SHADOW: Render Normal with Shadows

			vboRenderManager.vboTerrain.render(vboRenderManager);

			vboRenderManager.renderStaticGeometry(QString("3d_sidewalk"));
			vboRenderManager.renderStaticGeometry(QString("3d_roads"));

			
			vboRenderManager.renderStaticGeometry(QString("3d_roads_inter"));//
			vboRenderManager.renderStaticGeometry(QString("3d_roads_interCom"));//
			

			vboRenderManager.renderStaticGeometry(QString("3d_building"));
			vboRenderManager.renderStaticGeometry(QString("3d_building_fac"));

			if(mainWin->controlWidget->ui.render_3DtreesCheckBox->isChecked()){
				vboRenderManager.renderStaticGeometry(QString("3d_trees"));//hatch
				vboRenderManager.renderAllStreetElementName("tree");//LC
				vboRenderManager.renderAllStreetElementName("streetLamp");//LC
			}

		}
		// SHADOWS
		if(drawMode==1){
			glUniform1i(glGetUniformLocation(vboRenderManager.program,"shadowState"), 2);// SHADOW: From light

			vboRenderManager.vboTerrain.render(vboRenderManager);

			//vboRenderManager.renderStaticGeometry(QString("3d_sidewalk"));
			//vboRenderManager.renderStaticGeometry(QString("3d_roads"));
			vboRenderManager.renderStaticGeometry(QString("3d_building"));
			vboRenderManager.renderStaticGeometry(QString("3d_building_fac"));

			if(mainWin->controlWidget->ui.render_3DtreesCheckBox->isChecked()){
				vboRenderManager.renderStaticGeometry(QString("3d_trees"));//hatch
				vboRenderManager.renderAllStreetElementName("tree");//LC
				vboRenderManager.renderAllStreetElementName("streetLamp");//LC
			}
		}
	}

	///////////////////////////////////
	// 2D WITH BLOCKS MODE
	if(G::global().getInt("3d_render_mode")==3){

		glDisable(GL_CULL_FACE);
		mainWin->urbanGeometry->render(vboRenderManager);
		glEnable(GL_CULL_FACE);
		
		vboRenderManager.vboTerrain.render(vboRenderManager);

		vboRenderManager.renderStaticGeometry(QString("2d_blocks"));
		vboRenderManager.renderStaticGeometry(QString("2d_parks"));

		// draw the selected vertex and edge
		if (vertexSelected) {
			RendererHelper::renderPoint(vboRenderManager, "selected_vertex", selectedVertex->pt, QColor(0, 0, 255), selectedVertex->pt3D.z() + 2.0f);
		}
		if (edgeSelected) {
			Polyline3D polyline(selectedEdge->polyline3D);
			for (int i = 0; i < polyline.size(); ++i) polyline[i].setZ(polyline[i].z() + 10.0f);
			RendererHelper::renderPolyline(vboRenderManager, "selected_edge_lines", "selected_edge_points", polyline, QColor(0, 0, 255));
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
			mainWin->urbanGeometry->adaptToTerrain();
			edgeSelected = false;
			updateGL();
		}
		break;
	case Qt::Key_R:
		printf("Reseting camera pose\n");
		camera->resetCamera();
		break;
	case Qt::Key_W:
		camera3D.moveKey(0);updateCamera();updateGL();
		break;
	case Qt::Key_S:
		camera3D.moveKey(1);updateCamera();updateGL();
		break;
	case Qt::Key_D:
		camera3D.moveKey(2);updateCamera();updateGL();
		break;
	case Qt::Key_A:
		camera3D.moveKey(3);updateCamera();updateGL();
		break;
	case Qt::Key_Q:
		camera3D.moveKey(4);updateCamera();updateGL();
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
	if(G::global()["3d_render_mode"]==1)
		camera3D.updateCamMatrix();
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

	// light poss
	QVector3D light_dir=camera3D.light_dir.toVector3D();
	glUniform3f(glGetUniformLocation(vboRenderManager.program, "lightDir"),light_dir.x(),light_dir.y(),light_dir.z());
}//

void GLWidget3D::generate2DGeometry(){
	GraphUtil::cleanEdges(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);
	printf("generate2DGeometry\n");
	G::global()["num_place_types"]=0;
	G::global()["3d_render_mode"]=3;//2D geometry
	VBOPm::generate2DGeometry(vboRenderManager, mainWin->urbanGeometry->roads);

}//

void GLWidget3D::generate3DGeometry(bool justRoads){
	GraphUtil::cleanEdges(mainWin->urbanGeometry->roads);
	GraphUtil::clean(mainWin->urbanGeometry->roads);

	printf("generate3DGeometry\n");
	G::global()["3d_render_mode"]=1;//LC
	mainWin->controlWidget->ui.terrain_2DShader->setChecked(false);
	//camera=&camera3D;//change camera MODE!! 
	//camera=&flyCamera;

	//1. update roadgraph geometry
	if(justRoads){//just roads a bit higher
		G::global()["3d_road_deltaZ"]=10.0f;
		mainWin->controlWidget->ui.render_3DtreesCheckBox->setChecked(false);//not trees
	}else{
		G::global()["3d_road_deltaZ"]=1.0f;
	}
	VBORoadGraph::updateRoadGraph(vboRenderManager, mainWin->urbanGeometry->roads);
	//2. generate blocks, parcels and buildings and vegetation
	if(justRoads==false)
		VBOPm::generateGeometry(vboRenderManager, mainWin->urbanGeometry->roads);

	shadow.makeShadowMap(this);

	printf("<<generate3DGeometry\n");
}//