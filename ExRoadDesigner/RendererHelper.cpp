#include "RendererHelper.h"

/**
 * 与えられたポリゴンに基づいて、閉じた領域を描画する。
 * ただし、ポリゴンデータ自体は、閉じていなくて良い。
 */
/*
void RendererHelper::renderArea(const Polygon3D& area, const QColor& color, GLenum lineType) {
	if (area.size() == 0) return;

	std::vector<mylib::RenderablePtr> renderables;
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(lineType, 3.0f)));
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(GL_POINTS, 10.0f)));

	mylib::Vertex v;

	v.color[0] = color.redF();
	v.color[1] = color.greenF();
	v.color[2] = color.blueF();
	v.color[3] = color.alphaF();
	v.normal[0] = 0.0f;
	v.normal[1] = 0.0f;
	v.normal[2] = 1.0f;

	for (int i = 0; i < area.size(); i++) {
		v.location[0] = area[i].x();
		v.location[1] = area[i].y();
		v.location[2] = area[i].z();
		renderables[0]->vertices.push_back(v);
		renderables[1]->vertices.push_back(v);
	}

	v.location[0] = area[0].x();
	v.location[1] = area[0].y();
	v.location[2] = area[0].z();
	renderables[0]->vertices.push_back(v);

	render(renderables);
}
*/

/**
 * 与えられたポリゴンに基づいて、閉じた領域を描画する。
 * ただし、ポリゴンデータ自体は、閉じていなくて良い。
 */
/*
void RendererHelper::renderArea(const Polygon2D& area, const QColor& color, GLenum lineType, float height) {
	std::vector<mylib::RenderablePtr> renderables;
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(lineType, 3.0f)));
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(GL_POINTS, 10.0f)));

	mylib::Vertex v;

	v.color[0] = color.redF();
	v.color[1] = color.greenF();
	v.color[2] = color.blueF();
	v.color[3] = color.alphaF();
	v.normal[0] = 0.0f;
	v.normal[1] = 0.0f;
	v.normal[2] = 1.0f;

	for (int i = 0; i < area.size(); i++) {
		v.location[0] = area[i].x();
		v.location[1] = area[i].y();
		v.location[2] = height;
		renderables[0]->vertices.push_back(v);
		renderables[1]->vertices.push_back(v);
	}

	v.location[0] = area[0].x();
	v.location[1] = area[0].y();
	v.location[2] = height;
	renderables[0]->vertices.push_back(v);

	render(renderables);
}
*/

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

/*
void RendererHelper::renderPolyline(const Polyline2D& polyline, const QColor& color, GLenum lineType, float height) {
	std::vector<mylib::RenderablePtr> renderables;
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(lineType, 3.0f)));
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(GL_POINTS, 10.0f)));
	
	mylib::Vertex v;
	v.color[0] = color.redF();
	v.color[1] = color.greenF();
	v.color[2] = color.blueF();
	v.color[3] = color.alphaF();
	v.normal[0] = 0.0f;
	v.normal[1] = 0.0f;
	v.normal[2] = 1.0f;

	// add lines
	for (int i = 0; i < polyline.size(); i++) {
		v.location[0] = polyline[i].x();
		v.location[1] = polyline[i].y();
		v.location[2] = height;

		renderables[0]->vertices.push_back(v);
		renderables[1]->vertices.push_back(v);
	}

	render(renderables);
}

void RendererHelper::renderConcave(Polygon2D& polygon, const QColor& color, float height) {
	std::vector<mylib::RenderablePtr> renderables;
	renderables.push_back(mylib::RenderablePtr(mylib::RenderablePtr(new mylib::Renderable(GL_TRIANGLES))));

	if (polygon.size() < 3) return;

	mylib::Vertex v;
	v.color[0] = color.redF();
	v.color[1] = color.greenF();
	v.color[2] = color.blueF();
	v.color[3] = color.alphaF();
	v.normal[0] = 0.0f;
	v.normal[1] = 0.0f;
	v.normal[2] = 1.0f;

	std::vector<Polygon2D> trapezoids = polygon.tessellate();

	for (int i = 0; i < trapezoids.size(); ++i) {
		if (trapezoids[i].size() < 3) continue;

		for (int j = 1; j < trapezoids[i].size() - 1; ++j) {
			v.location[0] = trapezoids[i][0].x();
			v.location[1] = trapezoids[i][0].y();
			v.location[2] = height;
			renderables[0]->vertices.push_back(v);

			v.location[0] = trapezoids[i][j].x();
			v.location[1] = trapezoids[i][j].y();
			v.location[2] = height;
			renderables[0]->vertices.push_back(v);

			v.location[0] = trapezoids[i][j+1].x();
			v.location[1] = trapezoids[i][j+1].y();
			v.location[2] = height;
			renderables[0]->vertices.push_back(v);
		}
	}

	render(renderables);
}
*/