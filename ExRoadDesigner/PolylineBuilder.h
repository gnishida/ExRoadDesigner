#pragma once

#include "glew.h"

#include <QVector2D>
#include "common.h"
#include "Polygon2D.h"
#include "Polyline2D.h"
#include "Polyline3D.h"
#include "Terrain.h"

class PolylineBuilder {
private:
	bool _selecting;
	Polyline2D _polyline;
	Polyline3D _polyline3D;

public:
	PolylineBuilder();
	~PolylineBuilder() {}

	void start(const QVector2D& pt);
	void addPoint(const QVector2D& pt);
	void moveLastPoint(const QVector2D& pt);
	void end();
	void cancel();

	bool selected() const;
	bool selecting() const;
	Polyline2D polyline() const;

	const Polyline3D& polyline3D() const;

	void adaptToTerrain(Terrain* terrain);
};

