#include "ConvexHull.h"

void ConvexHull::addPoint(const QVector2D& point) {
	boost::geometry::append(all_points, make<point_xy<float> >(point.x(), point.y()));
}

void ConvexHull::convexHull(Polygon2D& hull) {
	hull.clear();

	boost::geometry::model::polygon<point_xy<float> > hull2;
    boost::geometry::convex_hull(all_points, hull2);

	for (boost::geometry::model::polygon<point_xy<float> >::ring_type::iterator it = hull2.outer().begin(); it != hull2.outer().end(); ++it) {
		hull.push_back(QVector2D(it->x(), it->y()));
	}
}

Polygon2D ConvexHull::convexHull() {
	Polygon2D polygon;

	boost::geometry::model::polygon<point_xy<float> > hull2;
    boost::geometry::convex_hull(all_points, hull2);

	for (boost::geometry::model::polygon<point_xy<float> >::ring_type::iterator it = hull2.outer().begin(); it != hull2.outer().end(); ++it) {
		polygon.push_back(QVector2D(it->x(), it->y()));
	}

	return polygon;
}