#pragma once

#include "glew.h"

#include <QDomNode>
#include <boost/shared_ptr.hpp>
#include "Polygon2D.h"
#include "Polyline3D.h"
//#include "Terrain.h"
#include "VBORenderManager.h"
#include "RoadGraph.h"

class RoadArea {
public:
	Polygon2D area;
	Polyline3D area3D;
	Polyline2D hintLine;
	Polyline3D hintLine3D;
	Polyline2D controlPoints;
	Polyline3D controlPoints3D;
	RoadGraph roads;

	bool areaPointSelected;
	int areaPointSelectedIndex;

	RoadGraph origRoads;
	Polyline2D origControlPoints;

	bool controlPointSelected;
	int controlPointSelectedIndex;

public:
	RoadArea();
	RoadArea(const Polygon2D &area);
	~RoadArea() {}

	void clear();

	void load(QDomNode& node);
	void loadArea(QDomNode &node);
	void loadHintLine(QDomNode &node);

	void save(QDomDocument& doc, QDomNode& node);

	void adaptToTerrain(VBORenderManager* vboRenderManager);

	void selectAreaPoint(const QVector2D &pt);
	void updateAreaPoint(const QVector2D &pt);
};

typedef boost::shared_ptr<RoadArea> RoadAreaPtr;