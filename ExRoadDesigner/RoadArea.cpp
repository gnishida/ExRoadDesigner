#include "RoadArea.h"
#include "global.h"

RoadArea::RoadArea() {
	controlPointSelected = false;
}

RoadArea::RoadArea(const Polygon2D &area) {
	this->area = area;
	controlPointSelected = false;
}

void RoadArea::clear() {
	roads.clear();
}

/**
 * load an area from XML
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

void RoadArea::adaptToTerrain(VBORenderManager* vboRenderManager) {
	area3D.clear();
	hintLine3D.clear();
	controlPoints3D.clear();

	for (int i = 0; i < area.size(); ++i) {
		float z = 0.0f;
		if (!G::getBool("shader2D")) {
			z = vboRenderManager->getTerrainHeight(area[i].x(), area[i].y());
		}
		area3D.push_back(QVector3D(area[i].x(), area[i].y(), z + 30));
	}

	for (int i = 0; i < hintLine.size(); ++i) {
		float z = 0.0f;
		if (!G::getBool("shader2D")) {
			z = vboRenderManager->getTerrainHeight(hintLine[i].x(), hintLine[i].y());
		}
		hintLine3D.push_back(QVector3D(hintLine[i].x(), hintLine[i].y(), z + 30));
	}

	for (int i = 0; i < controlPoints.size(); ++i) {
		float z = 0.0f;
		if (!G::getBool("shader2D")) {
			z = vboRenderManager->getTerrainHeight(controlPoints[i].x(), controlPoints[i].y());
		}
		controlPoints3D.push_back(QVector3D(controlPoints[i].x(), controlPoints[i].y(), z + 30));
	}

}

void RoadArea::selectAreaPoint(const QVector2D &pt) {
	float min_dist = std::numeric_limits<float>::max();
	int min_index;
	for (int i = 0; i < area.size(); ++i) {
		float dist = (pt - area[i]).lengthSquared();
		if (dist < min_dist) {
			min_dist = dist;
			min_index = i;
		}
	}

	if (min_dist < 10000) {
		areaPointSelected = true;
		areaPointSelectedIndex = min_index;
	}
}

void RoadArea::updateAreaPoint(const QVector2D &pt) {
	if (areaPointSelected && areaPointSelectedIndex >= 0 && areaPointSelectedIndex < area.size()) {
		area[areaPointSelectedIndex] = pt;
		if (areaPointSelectedIndex == 0) {
			area[area.size() - 1] = pt;
		}
		if (areaPointSelectedIndex == area.size() - 1) {
			area[0] = pt;
		}
	}
}
