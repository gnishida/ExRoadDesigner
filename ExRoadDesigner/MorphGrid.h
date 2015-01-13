#pragma once

#include <vector>
#include "Polygon2D.h"

class MorphGrid {
public:
	std::vector<Polygon2D> srcGrid;
	std::vector<Polygon2D> dstGrid;

public:
	MorphGrid() {}
	~MorphGrid() {}

	void addGrid(const Polygon2D &srcPolygon, const Polygon2D &dstPolygon);

	QVector2D transform(const QVector2D &pt);
};

