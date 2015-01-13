#pragma once

#include "common.h"
#include "Polygon2D.h"

using boost::geometry::model::d2::point_xy;
using boost::geometry::make;

class ConvexHull {
private:
	boost::geometry::model::multi_point<point_xy<float> > all_points;

public:
	ConvexHull() {}
	~ConvexHull() {}

	void addPoint(const QVector2D& p);
	void convexHull(Polygon2D& hull);
	Polygon2D convexHull();
};

