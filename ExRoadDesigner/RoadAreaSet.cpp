#include "RoadAreaSet.h"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <QTextStream>
#include "GraphUtil.h"

const size_t RoadAreaSet::size() const {
	return areas.size();
}

RoadAreaPtr RoadAreaSet::operator[](int index) {
	return areas[index];
}

void RoadAreaSet::add(RoadAreaPtr area) {
	areas.push_back(area);
}

void RoadAreaSet::clear() {
	roads.clear();
	areas.clear();
}

void RoadAreaSet::remove(int index) {
	areas.erase(areas.begin() + index);
}

bool RoadAreaSet::contains(const QVector2D &pt) const {
	for (int i = 0; i < areas.size(); ++i) {
		if (areas[i]->area.contains(pt)) return true;
	}

	return false;
}

Polygon2D RoadAreaSet::unionArea() const {
	Polygon2D ret;
	if (areas.size() == 0) return ret;
	
	std::vector<boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > > output;
	for (int i = 0; i < areas.size(); ++i) {
		boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > temp;

		if (output.size() > 0) {
			for (int j = 0; j < output[0].outer().size(); ++j) {
				temp.outer().push_back(output[0].outer()[j]);
			}
		}

		output.clear();
		
		//Polygon2D temp;
		boost::geometry::model::polygon<boost::geometry::model::d2::point_xy<double> > b = areas[i]->area.convertToBoostPolygon();
		boost::geometry::union_(temp, b, output);
	}

	for (int i = 0; i < output[0].outer().size(); ++i) {
		ret.push_back(QVector2D(output[0].outer()[i].x(), output[0].outer()[i].y()));
	}

	return ret;
}

void RoadAreaSet::selectArea(const QVector2D &pt) {
	selectedIndex = -1;
	for (int i = 0; i < areas.size(); ++i) {
		if (areas[i]->area.contains(pt)) {
			selectedIndex = i;
			break;
		}
	}
}

void RoadAreaSet::selectLastArea() {
	selectedIndex = areas.size() - 1;
}

RoadAreaPtr RoadAreaSet::selectedArea() {
	return areas[selectedIndex];
}

void RoadAreaSet::deleteArea() {
	if (selectedIndex < 0 || selectedIndex >= areas.size()) return;

	areas[selectedIndex]->clear();
	areas.erase(areas.begin() + selectedIndex);

	selectedIndex = -1;
}

void RoadAreaSet::addRoads(int roadType, int lanes, bool oneWay, const Polyline2D &polyline) {
	GraphUtil::addEdge(roads, polyline, roadType, lanes, oneWay);
	GraphUtil::planarify(roads);
	GraphUtil::clean(roads);
}

void RoadAreaSet::mergeRoads() {
	for (int i = 0; i < areas.size(); ++i) {
		GraphUtil::mergeRoads(roads, areas[i]->roads);
		areas[i]->roads.clear();
	}
}

/**
 * load all the areas from XML.
 */
void RoadAreaSet::load(QString filename) {
	QFile file(filename);

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();
	while (!node.isNull()) {
		if (node.toElement().tagName() == "area") {
			RoadAreaPtr area = RoadAreaPtr(new RoadArea());
			area->load(node);
			areas.push_back(area);
		}

		node = node.nextSibling();
	}
}

void RoadAreaSet::save(QString filepath) {
	QDomDocument doc;

	QDomElement root = doc.createElement("areas");
	doc.appendChild(root);

	for (int i = 0; i < areas.size(); ++i) {
		areas[i]->save(doc, root);
	}

	// write the dom to the file
	QFile file(filepath);
	file.open(QIODevice::WriteOnly);

	QTextStream out(&file);
	doc.save(out, 4);
}

