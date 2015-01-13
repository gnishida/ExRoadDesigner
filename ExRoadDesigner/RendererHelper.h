#pragma once

#include "VBORenderManager.h"
#include "Polygon2D.h"
#include "Polygon3D.h"
#include "Polyline2D.h"
#include "Polyline3D.h"

class RendererHelper {
public:
	static void renderPoint(VBORenderManager &renderManager, const QString &pointsN, const QVector2D& pt, const QColor& color, float height);
	static void renderPolyline(VBORenderManager &renderManager, const QString &linesN, const QString &pointsN, const Polyline3D& polyline, const QColor& color);
	static void renderPolyline(VBORenderManager &renderManager, const QString &linesN, const QString &pointsN, const Polyline2D& polyline, const QColor& color, float height);
	static void renderCircle(VBORenderManager &renderManager, const QString &linesN, const QVector2D &center, float radius, const QColor &color, float height);
};

