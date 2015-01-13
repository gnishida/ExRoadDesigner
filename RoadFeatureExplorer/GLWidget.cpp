#include "GLWidget.h"
#include "MainWindow.h"
#include <road/GraphUtil.h>
#include <gl/GLU.h>
#include <vector>

float GLWidget::MIN_Z = 150.0f;
float GLWidget::MAX_Z = 11520.0f;
//float GLWidget::MAX_Z = 2880.0f;

GLWidget::GLWidget(MainWindow* mainWin) : QGLWidget(QGLFormat(QGL::SampleBuffers), (QWidget*)mainWin) {
	this->mainWin = mainWin;

	renderer = new RendererHelper();

	// set up the camera
	camera = new Camera();
	camera->setLookAt(0.0f, 0.0f, 0.0f);
	camera->setTranslation(0.0f, 0.0f, (MIN_Z + MAX_Z) / 2.0f);

	QString str;
	str.setNum(camera->dz);
	mainWin->ui.statusBar->showMessage(str);

	// initialize the width and others
	roads.setZ((MIN_Z + MAX_Z) / 2.0f);

	areaPointSelected = false;

	// initialize the key status
	shiftPressed = false;
	controlPressed = false;
	altPressed = false;
	keyXPressed = false;
}

GLWidget::~GLWidget() {
}

void GLWidget::drawScene() {
	// draw the road graph
	roads.generate2DMesh();
	renderer->render(roads.renderables);
	
	// define the height for other items
	float height = (float)((int)(camera->dz * 0.012f)) * 0.15f;

	// draw the selected area
	if (selectedAreaBuilder.selected()) {
		renderer->renderArea(selectedArea, QColor(0, 0, 255), GL_LINE_STIPPLE, height);
	} else if (selectedAreaBuilder.selecting()) {
		renderer->renderPolyline(selectedAreaBuilder.polyline(), QColor(0, 0, 255), GL_LINE_STIPPLE, height);
	}

	// draw a hint line
	if (hintLineBuilder.selected()) {
		renderer->renderPolyline(hintLine, QColor(255, 0, 0), GL_LINE_STIPPLE, height);
	} else if (hintLineBuilder.selecting()) {
		renderer->renderPolyline(hintLineBuilder.polyline(), QColor(255, 0, 0), GL_LINE_STIPPLE, height);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void GLWidget::keyPressEvent(QKeyEvent *e) {
	shiftPressed = false;
	controlPressed = false;
	altPressed = false;

	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed = true;
		break;
	case Qt::Key_Control:
		controlPressed = true;
		break;
	case Qt::Key_Alt:
		altPressed = true;
		break;
	case Qt::Key_X:
		keyXPressed = true;
		break;
	case Qt::Key_Escape:
		selectedAreaBuilder.cancel();
		
		updateGL();

		break;
	}
}

void GLWidget::keyReleaseEvent(QKeyEvent* e) {
	if (e->isAutoRepeat()) {
		e->ignore();
		return;
	}

	switch (e->key()) {
	case Qt::Key_Shift:
		shiftPressed = false;
		break;
	case Qt::Key_Control:
		controlPressed = false;
		break;
	case Qt::Key_Alt:
		altPressed = false;
		break;
	case Qt::Key_X:
		keyXPressed = false;
		break;
	}
}

void GLWidget::mousePressEvent(QMouseEvent *e) {
	if (Qt::ControlModifier == e->modifiers()) {
		controlPressed = true;
	} else {
		controlPressed = false;
	}

	this->setFocus();

	lastPos = e->pos();
	QVector2D pos;
	mouseTo2D(e->x(), e->y(), pos);

	if (e->buttons() & Qt::LeftButton) {
		switch (mainWin->mode) {
		case MainWindow::MODE_AREA_CREATE:
			if (!selectedAreaBuilder.selecting()) {
				selectedAreaBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (selectedAreaBuilder.selecting()) {
				selectedAreaBuilder.addPoint(pos);
			}
			break;
		case MainWindow::MODE_HINT_LINE:
			if (!hintLineBuilder.selecting()) {
				hintLineBuilder.start(pos);
				setMouseTracking(true);
			}
		
			if (hintLineBuilder.selecting()) {
				hintLineBuilder.addPoint(pos);
			}
			break;
		case MainWindow::MODE_AREA_EDIT:
			float min_dist = std::numeric_limits<float>::max();
			int min_index;
			for (int i = 0; i < selectedArea.size(); ++i) {
				float dist = (pos - selectedArea[i]).lengthSquared();
				if (dist < min_dist) {
					min_dist = dist;
					min_index = i;
				}
			}

			if (min_dist < 10000) {
				areaPointSelected = true;
				areaPointSelectedIndex = min_index;
			}
		}
	}

	updateGL();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e) {
	lastPos = e->pos();

	setCursor(Qt::ArrowCursor);
	updateGL();
}

void GLWidget::mouseMoveEvent(QMouseEvent *e) {
	float dx = (float)(e->x() - lastPos.x());
	float dy = (float)(e->y() - lastPos.y());
	lastPos = e->pos();

	QVector2D pos;
	mouseTo2D(e->x(), e->y(), pos);

	if (e->buttons() & Qt::MidButton) {   // Shift the camera
		setCursor(Qt::ClosedHandCursor);
		camera->changeXYZTranslation(-dx * camera->dz * 0.001f, dy * camera->dz * 0.001f, 0);
	} else if (e->buttons() & Qt::RightButton) { // Zoom the camera
		setCursor(Qt::SizeVerCursor);

		camera->changeXYZTranslation(0, 0, -dy * camera->dz * 0.02f);
		if (camera->dz < MIN_Z) camera->dz = MIN_Z;
		if (camera->dz > MAX_Z) camera->dz = MAX_Z;

		QString str;
		str.setNum(camera->dz);
		mainWin->ui.statusBar->showMessage(str);

		// tell the Z coordinate to the road graph so that road graph updates rendering related variables.
		roads.setZ(camera->dz);

		lastPos = e->pos();
	} else if (selectedAreaBuilder.selecting()) {	// Move the last point of the selected polygonal area
		selectedAreaBuilder.moveLastPoint(pos);
	} else if (hintLineBuilder.selecting()) {
		hintLineBuilder.moveLastPoint(pos);
	} else {
		switch (mainWin->mode) {
		case MainWindow::MODE_AREA_EDIT:
			if (controlPressed) {
				if (areaPointSelected) {
					selectedArea[areaPointSelectedIndex] = pos;
					if (areaPointSelectedIndex == 0) {
						selectedArea.back() = pos;
					}
					if (areaPointSelectedIndex == selectedArea.size() - 1) {
						selectedArea[0] = pos;
					}
				}
			}
			break;
		}
	}

	updateGL();
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *e) {
	setMouseTracking(false);

	switch (mainWin->mode) {
	case MainWindow::MODE_AREA_CREATE:
		selectedAreaBuilder.end();
		selectedArea = selectedAreaBuilder.polygon();
		mainWin->mode = MainWindow::MODE_AREA_EDIT;
		break;
	case MainWindow::MODE_HINT_LINE:
		hintLineBuilder.end();
		hintLine = hintLineBuilder.polyline();
		mainWin->mode = MainWindow::MODE_AREA_EDIT;
		break;
	}
}

void GLWidget::initializeGL() {
	qglClearColor(QColor(233, 229, 220));
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
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, materialEmission);
}

void GLWidget::resizeGL(int width, int height) {
	height = height ? height : 1;

	glViewport(0, 0, (GLint)width, (GLint)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (GLfloat)width/(GLfloat)height, 5.0f, 90000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	camera->applyCamTransform();
}

void GLWidget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	camera->applyCamTransform();	

	drawScene();	
}

/**
 * Convert the screen space coordinate (x, y) to the model space coordinate.
 */
void GLWidget::mouseTo2D(int x,int y, QVector2D &result) {
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
