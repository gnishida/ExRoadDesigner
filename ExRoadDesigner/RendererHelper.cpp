#include "RendererHelper.h"
#include "VBORenderManager.h"

void RendererHelper::renderPoint(VBORenderManager &renderManager, const QVector2D& pt, const QColor& color, float height) {
	renderManager.removeStaticGeometry("__temp__");
	renderManager.addPoint("__temp__", pt, color, height);
	renderManager.renderStaticGeometry("__temp__");
	renderManager.removeStaticGeometry("__temp__");
}

void RendererHelper::renderPolyline(VBORenderManager &renderManager, const Polyline3D& polyline, const QColor& color) {
	renderManager.removeStaticGeometry("__temp__");
	renderManager.addPolyline("__temp__", polyline, color);
	renderManager.renderStaticGeometry("__temp__");
	renderManager.removeStaticGeometry("__temp__");
}

void RendererHelper::renderPolyline(VBORenderManager &renderManager, const QString &linesN, const Polyline2D& polyline, const QColor& color, float heightOffset) {
	// clean former step
	renderManager.removeStaticGeometry(linesN);

	std::vector<Vertex> vertP;
	std::vector<Vertex> vertL;

	int num = polyline.size();
	if (num == 0) return;

	std::vector<Vertex> vert(num - 1);

	for (int i = 0; i < num; ++i) {
		Vertex v=Vertex(polyline[i].x(), polyline[i].y(), renderManager.getTerrainHeight(polyline[i].x(), polyline[i].y()) + heightOffset, color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertP.push_back(v);//add point
	}

	// add lines
	for (int i = 0; i < num - 1; i++) {
		Vertex v=Vertex(polyline[i].x(), polyline[i].y(), renderManager.getTerrainHeight(polyline[i].x(), polyline[i].y()) + heightOffset, color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		Vertex v2=Vertex(polyline[i+1].x(), polyline[i+1].y(), renderManager.getTerrainHeight(polyline[i+1].x(), polyline[i+1].y()) + heightOffset, color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertL.push_back(v);//add line
		vertL.push_back(v2);//add line
	}

	renderManager.addStaticGeometry(linesN,vertL,"",GL_LINES,1);//MODE=1 color
}

void RendererHelper::renderCircle(VBORenderManager &renderManager, const QString &linesN, const QVector2D& center, float radius, const QColor& color, float heightOffset) {
	// clean former step
	renderManager.removeStaticGeometry(linesN);

	std::vector<Vertex> circleP;

	for (int i = 0; i < 20; ++i) {
		float th = i * M_PI / 10.0f;
		float x = center.x() + radius * cosf(th);
		float y = center.y() + radius * sinf(th);
		Vertex v(x, y, renderManager.getTerrainHeight(x, y) + heightOffset, color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		circleP.push_back(v);
	}


	renderManager.addStaticGeometry(linesN, circleP, "", GL_LINE_LOOP, 1);
}