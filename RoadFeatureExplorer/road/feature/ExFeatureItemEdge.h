#pragma once

#include "../../common/Polyline2D.h"

class ExFeatureItemEdge {
public:
	Polyline2D edge;
	int lanes;
	bool deadend;
	bool onBoundary;

public:
	ExFeatureItemEdge(const Polyline2D &edge, int lanes, bool deadend, bool onBoundary) : edge(edge), lanes(lanes), deadend(deadend), onBoundary(onBoundary) {}
	~ExFeatureItemEdge() {}

	void scale(float scaleX, float scaleY, const QVector2D &orig);
};

