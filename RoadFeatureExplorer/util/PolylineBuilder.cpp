#include "PolylineBuilder.h"

PolylineBuilder::PolylineBuilder() {
	_selecting = false;
}

void PolylineBuilder::start(const QVector2D& pt) {
	_polyline.clear();
	_polyline.push_back(pt);

	_selecting = true;
}

/**
 * Add pt to this polyline.
 */
void PolylineBuilder::addPoint(const QVector2D& pt) {
	_polyline.push_back(pt);
}

void PolylineBuilder::moveLastPoint(const QVector2D& pt) {
	if (_polyline.size() == 0) return;

	_polyline[_polyline.size() - 1] = pt;
}

void PolylineBuilder::end() {
	_selecting = false;

	if (_polyline.size() >= 2 && (_polyline.nextLast() - _polyline.last()).lengthSquared() < 900.0f) {
		_polyline.pop_back();
	}
}

void PolylineBuilder::cancel() {
	_polyline.clear();
	_selecting = false;
}

bool PolylineBuilder::selected() const {
	return !_selecting && _polyline.size() >= 1;
}

bool PolylineBuilder::selecting() const {
	return _selecting;
}

Polyline2D PolylineBuilder::polyline() const {
	return _polyline;
}

const Polyline3D& PolylineBuilder::polyline3D() const {
	return _polyline3D;
}

void PolylineBuilder::adaptToTerrain(mylib::Terrain* terrain) {
	_polyline3D.clear();

	for (int i = 0; i < _polyline.size(); ++i) {
		float z = terrain->getValue(_polyline[i].x(), _polyline[i].y());
		_polyline3D.push_back(QVector3D(_polyline[i].x(), _polyline[i].y(), z + 30));
	}
}
