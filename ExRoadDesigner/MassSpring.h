#pragma once

#include "RoadGraph.h"
#include <QVector2D>

class MassSpring {
public:
	RoadGraph &roads;
	std::vector<std::pair<RoadVertexDesc, RoadVertexDesc> > springs;

public:
	MassSpring(RoadGraph &roads);
	~MassSpring() {}

	void move(RoadVertexDesc v_desc, const QVector2D &newPt);
};

