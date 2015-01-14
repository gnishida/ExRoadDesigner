#include "RoadVertex.h"

RoadVertex::RoadVertex() {
	this->pt = QVector2D(0.0f, 0.0f);
	this->valid = true;
	this->onBoundary = false;
	this->fixed = false;
	this->patchId = -1;
	this->deadend = false;
	connector = false;
}

RoadVertex::RoadVertex(const QVector2D &pt) {
	this->pt.setX(pt.x());
	this->pt.setY(pt.y());
	this->valid = true;
	this->onBoundary = false;
	this->fixed = false;
	this->patchId = -1;
	this->deadend = false;
	this->connector = false;
}

const QVector2D& RoadVertex::getPt() const {
	return pt;
}

