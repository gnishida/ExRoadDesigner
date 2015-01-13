#pragma once

#include <vector>
#include <QVector2D>
#include <QDomNode>
#include "../../common/Polyline2D.h"
#include "ExFeatureItemEdge.h"

class ExFeatureItem {
public:
	int id;
	QVector2D pt;
	bool streetSeed;
	bool byExample;
	std::vector<ExFeatureItemEdge> edges;

public:
	ExFeatureItem() : id(-1), byExample(false) {}
	ExFeatureItem(int id) : id(id), byExample(false) {}
	~ExFeatureItem() {}

	void addEdge(const Polyline2D &polyline, int lanes, bool deadend, bool onBoundary);
	float getMinDistance(const Polyline2D &polyline) const;
	float getMinAngle(const Polyline2D &polyline) const;

	void rotate(float orientation, const QVector2D &orig);
	void scale(float scaleX, float scaleY, const QVector2D &orig);

	void load(QDomNode& node);
	void save(QDomDocument& doc, QDomNode& node);

	void imwrite(const QString filename) const;
};

