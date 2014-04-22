#include "GLWidget.h"
#include "MyMainWindow.h"
#include "GraphUtil.h"
#include "CircleArea.h"
#include <gl/GLU.h>
#include <qvector3d.h>
#include <qfile.h>
#include <qset.h>
#include <qmap.h>
#include <vector>
#include "Util.h"

#include <QMouseEvent>

#include "VBORoadGraph.h"
#include "VBOPm.h"

GLWidget::GLWidget(MyMainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	roads = new RoadGraph();
	sketch = new Sketch();
	ex_roads = NULL;

	G::global()["MIN_Z"]=150.0f;
	G::global()["MAX_Z"]=2880.0f;

	// set up the cameras
	camera2D.resetCamera();
	camera3D.resetCamera();
	camera = &camera2D;

	/*camera = new Camera();
	camera->setLookAt(0.0f, 0.0f, 0.0f);
	camera->setTranslation(0.0f, 0.0f, MAX_Z);*/
	// initialize the width and others
	roads->setZ(G::global().getFloat("MAX_Z"));
}

GLWidget::~GLWidget() {
}

/**
 * Start a new road from scratch.
 */
void GLWidget::newRoad() {
	roads->clear();

	updateGL();
}

/**
 * Open a road from the specified file.
 */
void GLWidget::openRoad(QString filename) {
	roads->clear();

	GraphUtil::loadRoads(roads, filename, 7);

	updateGL();
}

/**
 * Make a road graph from the sketch.
 */
void GLWidget::makeRoadsFromSketch() {
	RoadGraph* temp = sketch->makeRoads();
	sketch->clear();

	GraphUtil::mergeRoads(roads, temp);
	delete temp;

	updateGL();
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
	this->setFocus();

	lastPos = event->pos();

	if (event->buttons() & Qt::LeftButton) {
		QVector2D pos;
		mouseTo2D(event->x(), event->y(), &pos);
		mainWin->ui.statusBar->showMessage(QString("clicked (%1, %2)").arg(pos.x()).arg(pos.y()));
		//////////////////////
		// MODE 2D
		if(G::global()["rend_mode"]==0){
			if (mainWin->mode == MyMainWindow::MODE_VIEW) {
				// MODE_VIEW
			} else if (mainWin->mode == MyMainWindow::MODE_SKETCH) {
				// MODE_SKETCH
				RoadVertexDesc v1_desc;
				if (!GraphUtil::getVertex(sketch, pos, 10.0f, v1_desc)) {
					// If there is a vertex close to the point, don't add new vertex. Otherwise, add a new vertex.
					RoadVertex* v1 = new RoadVertex(pos);
					v1_desc = boost::add_vertex(sketch->graph);
					sketch->graph[v1_desc] = v1;
				}
				// add the 2nd vertex of a line
				RoadVertex* v2 = new RoadVertex(pos);
				RoadVertexDesc v2_desc = boost::add_vertex(sketch->graph);
				sketch->graph[v2_desc] = v2;

				sketch->curVertex = v2_desc;

				// add an edge
				if (camera->dz <= G::global().getFloat("MAX_Z") / 2) {
					sketch->curEdge = GraphUtil::addEdge(sketch, v1_desc, v2_desc, 1, 1, false);
				} else {
					sketch->curEdge = GraphUtil::addEdge(sketch, v1_desc, v2_desc, 2, 2, false);
				}
			}

			updateGL();
		}
	}
}//

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
	if (mainWin->mode == MyMainWindow::MODE_SKETCH) {
		QVector2D pos;
		mouseTo2D(event->x(), event->y(), &pos);

		RoadVertexDesc v2_desc;
		if (GraphUtil::getVertex(sketch, pos, 10.0f, sketch->curVertex, v2_desc)) {
			// If there is a vertex close to the point, snap the point to the nearest vertex
			GraphUtil::snapVertex(sketch, sketch->curVertex, v2_desc);
		}
	}

	event->ignore();

	setCursor(Qt::ArrowCursor);
	updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event) {
	float dx = (float)(event->x() - lastPos.x());
	float dy = (float)(event->y() - lastPos.y());
	float camElevation = camera->getCamElevation();

	lastPos = event->pos();
	//////////////////////
	// MODE 2D
	if(G::global()["rend_mode"]==0){
		if (event->buttons() & Qt::LeftButton) {	
			if (mainWin->mode == MyMainWindow::MODE_VIEW) { // Shift the camera
				camera->changeXYZTranslation(-dx * camera->dz * 0.001f, dy * camera->dz * 0.001f, 0);
				updateCamera();

			} else if (mainWin->mode == MyMainWindow::MODE_SKETCH) { // Draw a sketch line
				QVector2D pos;
				mouseTo2D(event->x(), event->y(), &pos);

				GraphUtil::moveVertex(sketch, sketch->curVertex, pos);

			}
		} else if(event->buttons() & Qt::MidButton) {   // Shift the camera
			camera->changeXYZTranslation(-dx * camera->dz * 0.001f, dy * camera->dz * 0.001f, 0);
			updateCamera();
		} else if (event->buttons() & Qt::RightButton) { // Zoom the camera
			setCursor(Qt::SizeVerCursor);

			camera->changeXYZTranslation(0, 0, -dy * camera->dz * 0.02f);
			if (camera->dz < G::global().getFloat("MIN_Z")) camera->dz = G::global().getFloat("MIN_Z");
			if (camera->dz > G::global().getFloat("MAX_Z")) camera->dz = G::global().getFloat("MAX_Z");
			updateCamera();

			roads->setZ(camera->dz);

			lastPos = event->pos();

			mainWin->ui.statusBar->showMessage(QString("Z: %1").arg(camera->dz));
		}
	}
	//////////////////////
	// MODE 3D
	if(G::global()["rend_mode"]==1){
		camera3D.motion(dx,dy,false);//if M pressed--> light Otherwise-->Move camera
		updateCamera();
	}
	updateGL();
}

void GLWidget::keyPressEvent( QKeyEvent *e ){

	//printf("keypressed\n");
	switch( e->key() ){
	case Qt::Key_Escape:
		this->parentWidget()->close();
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
	case Qt::Key_Z:
		break;
	case Qt::Key_H:
		printf("repaint\n");
		updateGL();
		break;
	/*case Qt::Key_6:
		printf("Save camera 1\n");
		myCam->printCamera();
		myCam->saveCameraPose(1);
		break;
	case Qt::Key_7:
		printf("Save camera 2\n");
		myCam->saveCameraPose(2);
		break;	
	case Qt::Key_8:
		printf("Save camera 3\n");
		myCam->saveCameraPose(3);
		break;	
	case Qt::Key_9:
		printf("Save camera 4\n");
		myCam->saveCameraPose(4);
		break;
	case Qt::Key_0:
		printf("Save camera 5\n");
		myCam->saveCameraPose(5);
		break;
	case Qt::Key_1:
		printf("Load Camera1\n");
		myCam->loadCameraPose(1);
		break;
	case Qt::Key_2:
		printf("Load Camera2\n");
		myCam->loadCameraPose(2);
		break;
	case Qt::Key_3:
		printf("Load Camera3\n");
		myCam->loadCameraPose(3);
		break;
	case Qt::Key_4:
		printf("Load Camera4\n");
		myCam->loadCameraPose(4);
		break;
	case Qt::Key_5:
		printf("Load Camera5\n");
		myCam->loadCameraPose(5);
		break;*/
	default:
		;

	}
}//

void GLWidget::initializeGL() {
	/*qglClearColor(QColor(233, 229, 220));
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
	qglClearColor(QColor(233, 229, 220));
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

void GLWidget::resizeGL(int width, int height) {

	updateCamera();
}

void GLWidget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);
	
	// NOTE: camera transformation is not necessary here since the updateCamera updates the uniforms each time they are changed
	
	drawScene();	
}

float waterMove=0;
bool waterDir=true;

void GLWidget::drawScene() {
	///////////////////////////////////
	// 2D MODE
	if(G::global()["rend_mode"]==0){
		// draw the road graph
		roads->generateMesh(vboRenderManager,"roadGraph");
		vboRenderManager.renderStaticGeometry("roadGraph");

		// define the height for other items
		float height = (float)((int)(camera->dz * 0.012f)) * 0.1f * 1.5f;

		// draw the sketch
		sketch->generateSketchMesh(vboRenderManager,"sketch_lines","sketch_points",height);
		vboRenderManager.renderStaticGeometry("sketch_points");
		vboRenderManager.renderStaticGeometry("sketch_lines");

		// draw the reference road graph
		if (ex_roads != NULL) {
			RoadGraph* ref_roads = GraphUtil::copyRoads(ex_roads->roads);
			CircleArea circle(ex_roads->center, ex_roads->radius);
			GraphUtil::extractRoads(ref_roads, circle, true);
			ref_roads->setZ(camera->dz);
			ref_roads->generateMesh(vboRenderManager,"ref_roadGraph");
			vboRenderManager.renderStaticGeometry("ref_roadGraph");

			delete ref_roads;

			// draw the bounding box
			//BBox box = GraphUtil::getAABoundingBox(ref_roads);
		}
	}
	///////////////////////////////////
	// 3D MODE
	if(G::global()["rend_mode"]==1){

		glUniform1f(glGetUniformLocation(vboRenderManager.program, "waterMove"),waterMove);

		//waterDir=qrand()%2;
		if(waterDir)waterMove+=0.05;
		if(!waterDir)waterMove-=0.05;
		if(waterMove>1){waterMove=0;waterDir=!waterDir;}
		if(waterMove<0){waterMove=0;waterDir=!waterDir;}
		printf("wM %f\n",waterMove);
		vboRenderManager.renderStaticGeometry(QString("3d_roads"));
		vboRenderManager.renderStaticGeometry(QString("3d_sidewalk"));
		vboRenderManager.renderStaticGeometry(QString("3d_building"));
		vboRenderManager.renderStaticGeometry(QString("3d_building_fac"));

		vboRenderManager.renderStaticGeometry(QString("3d_trees"));
	
	}
	//renderer->render(roads->renderables);
	/*
	// define the height for other items
	float height = (float)((int)(camera->dz * 0.012f)) * 0.1f * 1.5f;

	// draw the sketch
	sketch->generateSketchMesh(height);
	renderer->render(sketch->renderables);

	// draw the reference road graph
	if (ex_roads != NULL) {
		RoadGraph* ref_roads = GraphUtil::copyRoads(ex_roads->roads);
		CircleArea circle(ex_roads->center, ex_roads->radius);
		GraphUtil::extractRoads(ref_roads, circle, true);
		ref_roads->setZ(camera->dz);
		ref_roads->generateMesh();
		renderer->render(ref_roads->renderables);

		delete ref_roads;

		// draw the bounding box
		//BBox box = GraphUtil::getAABoundingBox(ref_roads);
	}*/
}

/**
 * Convert the screen space coordinate (x, y) to the model space coordinate.
 */
void GLWidget::mouseTo2D(int x,int y, QVector2D *result) {
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
	result->setX(x2d);
	result->setY(y2d);
	//printf("Mouse %d %d\n",x,y);
}

// this method should be called after any camera transformation (perspective or modelview)
// it will update viewport, perspective, view matrix, and update the uniforms
void GLWidget::updateCamera(){
	// update matrices
	int height = this->height() ? this->height() : 1;
	glViewport(0, 0, (GLint)this->width(), (GLint)this->height());
	camera->updatePerspective(this->width(),height);
	camera->updateCamMatrix();
	if(G::global()["rend_mode"]==1)
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
	QVector3D light_position=-camera3D.light_dir.toVector3D();
	glUniform3f(glGetUniformLocation(vboRenderManager.program, "lightPosition"),light_position.x(),light_position.y(),light_position.z());
}//


void GLWidget::generate3DGeometry(){
	printf("generate3DGeometry\n");
	//1. update roadgraph
	VBORoadGraph::updateRoadGraph(vboRenderManager,*roads);
	//2. generate parcels and buildings
	VBOPm::generateGeometry(vboRenderManager,*roads);

	printf("<<generate3DGeometry\n");
}//