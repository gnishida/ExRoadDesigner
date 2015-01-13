#pragma once

#include "glew.h"

#include <QVector2D>
#include "common.h"
#include "Polygon2D.h"
#include "Polygon3D.h"
#include "Polyline2D.h"
#include "Polyline3D.h"
//#include "Terrain.h"
#include "VBORenderManager.h"

class PolygonBuilder {
private:
	bool _selecting;
	Polyline2D _polyline;
	Polyline3D _polyline3D;

public:
	PolygonBuilder();
	~PolygonBuilder() {}

	void start(const QVector2D& pt);
	void addPoint(const QVector2D& pt);
	void moveLastPoint(const QVector2D& pt);
	void end();
	void cancel();

	bool selected() const;
	bool selecting() const;
	Polyline2D polyline() const;
	Polygon2D polygon() const;

	Polygon3D polygon3D() const;
	const Polyline3D& polyline3D() const;

	void adaptToTerrain(VBORenderManager* vboRenderManager);
};

