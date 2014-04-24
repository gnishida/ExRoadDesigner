#pragma once

#include "VBORenderManager.h"
#include "Polygon2D.h"
#include "Polygon3D.h"
#include "Polyline2D.h"
#include "Polyline3D.h"

class RendererHelper {
public:
	/*static void renderArea(const Polygon3D& area, const QColor& color, GLenum lineType);
	static void renderArea(const Polygon2D& area, const QColor& color, GLenum lineType, float height);
	*/
	static void renderPoint(VBORenderManager &renderManager, const QString &pointsN, const QVector2D& pt, const QColor& color, float height);
	static void renderPolyline(VBORenderManager &renderManager, const QString &linesN, const QString &pointsN, const Polyline3D& polyline, const QColor& color);
	//static void renderPolyline(const Polyline2D& polyline, const QColor& color, GLenum lineType, float height);

	//static void renderConcave(Polygon2D& polygon, const QColor& color, float height);
};

