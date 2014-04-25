#include "RendererHelper.h"

void RendererHelper::renderPoint(VBORenderManager &renderManager, const QString &pointsN, const QVector2D& pt, const QColor& color, float height) {
	// clean former step
	renderManager.removeStaticGeometry(pointsN);

	std::vector<Vertex> vertP;

	Vertex v(pt.x(), pt.y(), height, color.redF(), color.greenF(), color.blueF(), 0, 0, 1, 0, 0, 0);//pos color normal tex
	vertP.push_back(v);

	renderManager.addStaticGeometry(pointsN, vertP, "", GL_POINTS, 1);

	renderManager.renderStaticGeometry(pointsN);
}

void RendererHelper::renderPolyline(VBORenderManager &renderManager, const QString &linesN, const QString &pointsN, const Polyline3D& polyline, const QColor& color) {
	// clean former step
	renderManager.removeStaticGeometry(linesN);
	renderManager.removeStaticGeometry(pointsN);

	std::vector<Vertex> vertP;
	std::vector<Vertex> vertL;

	int num = polyline.size();
	if (num == 0) return;

	std::vector<Vertex> vert(num - 1);

	for (int i = 0; i < num; ++i) {
		Vertex v=Vertex(polyline[i].x(), polyline[i].y(), polyline[i].z(), color.redF(), color.greenF(), color.blueF(), 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertP.push_back(v);//add point
	}

	// add lines
	for (int i = 0; i < num - 1; i++) {
		Vertex v=Vertex(polyline[i].x(), polyline[i].y(), polyline[i].z(), color.redF(), color.greenF(), color.blueF(), 0, 0, 1, 0, 0, 0);//pos color normal tex
		Vertex v2=Vertex(polyline[i+1].x(), polyline[i+1].y(), polyline[i+1].z(), color.redF(), color.greenF(), color.blueF(), 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertL.push_back(v);//add line
		vertL.push_back(v2);//add line
	}

	renderManager.addStaticGeometry(linesN,vertL,"",GL_LINES,1);//MODE=1 color
	renderManager.addStaticGeometry(pointsN,vertP,"",GL_POINTS,1);//MODE=1 colors

	renderManager.renderStaticGeometry(pointsN);
	renderManager.renderStaticGeometry(linesN);
}

