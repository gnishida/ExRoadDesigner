﻿#pragma once

#include "common.h"

class BBox {
public:
	QVector2D minPt, maxPt;

public:
	BBox() {}
	~BBox() {}

	void addPoint(const QVector2D& pt);
	bool contains(const QVector2D& pt) const;
	QVector2D midPt() const;
	float dx() const;
	float dy() const;
	float area() const;

	bool intersects(const QVector2D& a, const QVector2D& b, QVector2D& intPt) const;
};

/**
 * BBoxをBoostのboxの代替として使用
 */
BOOST_GEOMETRY_REGISTER_BOX(BBox, QVector2D, minPt, maxPt)

