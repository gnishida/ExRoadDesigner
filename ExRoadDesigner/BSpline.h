#pragma once

#include "Polyline2D.h"

class BSpline {
public:
	BSpline() {}
	~BSpline() {}

	static Polyline2D spline(const Polyline2D &controlPoints, int numSplit = 2);
};

