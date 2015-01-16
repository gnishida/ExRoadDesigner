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
	bool fixed;
	int type;
	int patchId;
	bool deadend;
	bool onBoundary;
	float rotationAngle;	// 反時計回りの回転角 [rad]
	bool connector;
	QString generationType;	// "example" / "pm" / "snapped"

	QHash<QString, QVariant> properties;

public:
	RoadVertex();
	RoadVertex(const QVector2D &pt);

	const QVector2D& getPt() const;
};

typedef boost::shared_ptr<RoadVertex> RoadVertexPtr;
