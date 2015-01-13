#pragma once

#include "glew.h"

#include "qvector4d.h"
#include "qmatrix4x4.h"
#include "global.h"
#include "Camera.h"
#include <QFile>

class Camera {
public:

	float fovy;

	QMatrix4x4 mvMatrix;
	QMatrix4x4 mvpMatrix;
	QMatrix4x4 pMatrix;
	QMatrix3x3 normalMatrix;

public:
	//virtual QVector4D getCamPos() = 0;

	virtual void updatePerspective(int width,int height) = 0;
	virtual void updateCamMatrix() = 0;
	virtual float getCamElevation() = 0;
	virtual void setRotation(float x, float y, float z) = 0;
	virtual void setXRotation(float angle) = 0;

	virtual void setYRotation(float angle) = 0;
	virtual void setZRotation(float angle) = 0;

	virtual void changeXRotation(float angle) = 0;

	virtual void changeYRotation(float angle) = 0;

	virtual void changeZRotation(float angle) = 0;
	virtual void setTranslation(float x, float y, float z) = 0;
	virtual void changeXYZTranslation(float x, float y, float z) = 0;

	//virtual void setLookAt(float x, float y, float z) = 0;
	virtual void resetCamera() = 0;

	virtual void saveCameraPose(const QString &filepath) = 0;

	virtual void loadCameraPose(const QString &filepath) = 0;
};

