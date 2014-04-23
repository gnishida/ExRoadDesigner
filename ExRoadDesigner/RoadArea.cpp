#include "RoadArea.h"

RoadArea::RoadArea() {
}

RoadArea::RoadArea(const Polygon2D &area) {
	this->area = area;
}

void RoadArea::clear() {
	roads.clear();
}

/**
 * 与えられたfeatureノード配下のXML情報に基づいて、グリッド特徴量を設定する。
 */
void RoadArea::load(QDomNode& node) {
	roads.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "area") {
			loadArea(child);
		} else if (child.toElement().tagName() == "hintLine") {
			loadHintLine(child);
		}

		child = child.nextSibling();
	}
}

void RoadArea::loadArea(QDomNode &node) {
	area.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			QVector2D pt(child.toElement().attribute("x").toFloat(), child.toElement().attribute("y").toFloat());
			area.push_back(pt);
		}

		child = child.nextSibling();
	}
}

void RoadArea::loadHintLine(QDomNode &node) {
	hintLine.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			QVector2D pt(child.toElement().attribute("x").toFloat(), child.toElement().attribute("y").toFloat());
			hintLine.push_back(pt);
		}

		child = child.nextSibling();
	}
}

void RoadArea::save(QDomDocument& doc, QDomNode& parent) {
	QDomElement node_area = doc.createElement("area");
	parent.appendChild(node_area);

	QDomElement node_area2 = doc.createElement("area");
	node_area.appendChild(node_area2);
	for (int i = 0; i < area.size(); ++i) {
		QDomElement node_point = doc.createElement("point");
		node_point.setAttribute("x", area[i].x());
		node_point.setAttribute("y", area[i].y());
		node_area2.appendChild(node_point);
	}

	QDomElement node_hintLine = doc.createElement("hintLine");
	node_area.appendChild(node_hintLine);
	for (int i = 0; i < hintLine.size(); ++i) {
		QDomElement node_point = doc.createElement("point");
		node_point.setAttribute("x", hintLine[i].x());
		node_point.setAttribute("y", hintLine[i].y());
		node_hintLine.appendChild(node_point);
	}
}

void RoadArea::adaptToTerrain() {//mylib::Terrain* terrain) {
	area3D.clear();
	hintLine3D.clear();

	for (int i = 0; i < area.size(); ++i) {
		//float z = terrain->getValue(area[i].x(), area[i].y());
		float z = 0.0f;
		area3D.push_back(QVector3D(area[i].x(), area[i].y(), z + 30));
	}

	for (int i = 0; i < hintLine.size(); ++i) {
		//float z = terrain->getValue(hintLine[i].x(), hintLine[i].y());
		float z = 0.0f;
		hintLine3D.push_back(QVector3D(hintLine[i].x(), hintLine[i].y(), z + 30));
	}
}
