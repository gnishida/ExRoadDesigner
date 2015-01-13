#pragma once

#include <vector>
#include <QVector2D>
#include <QVector3D>
#include <QHash>
#include <QVariant>
#include <boost/shared_ptr.hpp>

class RoadVertex {
public:
	QVector2D pt;
	QVector3D pt3D;
	bool valid;
	bool seed;
	bool fixed;
	int type;

	bool onBoundary;

	QHash<QString, QVariant> properties;

public:
	RoadVertex();
	RoadVertex(const QVector2D &pt);

	const QVector2D& getPt() const;
};

typedef boost::shared_ptr<RoadVertex> RoadVertexPtr;
