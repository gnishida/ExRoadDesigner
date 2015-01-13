#include "MorphGrid.h"

void MorphGrid::addGrid(const Polygon2D &srcPolygon, const Polygon2D &dstPolygon) {
	for (int i = 1; i < srcPolygon.size() - 1; ++i) {
		Polygon2D srcP;
		srcP.push_back(srcPolygon[0]);
		srcP.push_back(srcPolygon[i]);
		srcP.push_back(srcPolygon[i + 1]);
		srcP.push_back(srcPolygon[0]);
		srcGrid.push_back(srcP);

		Polygon2D dstP;
		dstP.push_back(dstPolygon[0]);
		dstP.push_back(dstPolygon[i]);
		dstP.push_back(dstPolygon[i + 1]);
		dstP.push_back(dstPolygon[0]);
		dstGrid.push_back(dstP);
	}
}

QVector2D MorphGrid::transform(const QVector2D &pt) {
	for (int i = 0; i < srcGrid.size(); ++i) {
		if (srcGrid[i].contains(pt)) {
			float denom = (srcGrid[i][2] - srcGrid[i][0]).x() * (srcGrid[i][1] - srcGrid[i][0]).y() - (srcGrid[i][2] - srcGrid[i][0]).y() * (srcGrid[i][1] - srcGrid[i][0]).x();

			float s = -((pt - srcGrid[i][0]).x() * (srcGrid[i][2] - srcGrid[i][0]).y() - (pt - srcGrid[i][0]).y() * (srcGrid[i][2] - srcGrid[i][0]).x()) / denom;
			float t = ((pt - srcGrid[i][0]).x() * (srcGrid[i][1] - srcGrid[i][0]).y() - (pt - srcGrid[i][0]).y() * (srcGrid[i][1] - srcGrid[i][0]).x()) / denom;

			return dstGrid[i][0] + (dstGrid[i][1] - dstGrid[i][0]) * s + (dstGrid[i][2] - dstGrid[i][0]) * t;
		}
	}

	return pt;
}
