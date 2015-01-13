#include "BSpline.h"

Polyline2D BSpline::spline(const Polyline2D &controlPoints, int numSplit) {
	Polyline2D ret = controlPoints;
	Polyline2D polyline;

	if (controlPoints.size() <= 2 || numSplit < 2) return ret;

	for (int nS = 0; nS < numSplit; ++nS) {
		polyline = ret;
		ret.clear();

		ret.push_back(polyline[0]);
		for (int i = 1; i < polyline.size() - 1; ++i) {
			QVector2D p0 = polyline[i - 1];
			QVector2D p1 = polyline[i];
			QVector2D p2 = polyline[i + 1];

			ret.push_back((p0 + p1 * 2) / 3.0f);
			ret.push_back((p2 + p1 * 2) / 3.0f);
		}
		ret.push_back(polyline.back());
	}

	/*
	for (int nS = 0; nS < numSplit; ++nS) {
		ret.push_back(controlPoints[0]);
		for (int i = 1; i < controlPoints.size() - 1; ++i) {
			for (int j = 1; j < numSplit; ++j) {
				QVector2D p0 = (controlPoints[i - 1] * (numSplit - j) + controlPoints[i] * j) / (float)numSplit;
				QVector2D p1 = (controlPoints[i] * (numSplit - j) + controlPoints[i + 1] * j) / (float)numSplit;

				ret.push_back((p0 * (numSplit - j) + p1 * j) / (float)numSplit);
			}
		}
		ret.push_back(controlPoints.back());
	}
	*/

	return ret;
}
