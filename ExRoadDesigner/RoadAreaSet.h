#pragma once

#include "glew.h"

#include "RoadArea.h"

class RoadAreaSet {
public:
	RoadGraph roads;
	std::vector<RoadAreaPtr> areas;
	int selectedIndex;

public:
	RoadAreaSet() { selectedIndex = -1; }
	~RoadAreaSet() {}

	const size_t size() const;
	RoadAreaPtr operator[](int index);
	void add(RoadAreaPtr area);
	void clear();
	void remove(int index);
	bool contains(const QVector2D &pt) const;
	Polygon2D unionArea() const;

	void selectArea(const QVector2D &pt);
	void selectLastArea();
	RoadAreaPtr selectedArea();
	void deleteArea();

	void setZ(float z);

	void addRoads(int roadType, int lanes, bool oneWay, const Polyline2D &polyline);
	void mergeRoads();

	void load(QString filename);
	void save(QString filename);
};

