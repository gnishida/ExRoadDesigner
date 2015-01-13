#pragma once

#include "glew.h"

#include <QVector2D>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <boost/shared_ptr.hpp>
#include "Polygon2D.h"
#include "Histogram.h"
#include "RoadGraph.h"

class ExFeature {
public:
	Polygon2D area;
	RoadGraph avenues;
	RoadGraph reducedAvenues;
	RoadGraph streets;
	Polyline2D hintLine;

	float avgAvenueLength;
	float varAvenueLength;
	float avgStreetLength;
	float varStreetLength;
	float avgAvenueCurvature;
	float varAvenueCurvature;
	float avgStreetCurvature;
	float varStreetCurvature;

	bool avenueShapesDetected;
	bool streetShapesDetected;
	std::vector<RoadEdgeDescs> avenueShapes;
	std::vector<RoadEdgeDescs> streetShapes;

public:
	ExFeature() {}
	~ExFeature() {}

	void setArea(const Polygon2D &area);
	void setHintLine(const Polyline2D &polyline);
	RoadGraph& roads(int roadType);
	RoadGraph& reducedRoads(int roadType);

	//void rotate(float deg);
	//void scale(const Polygon2D &area);

	// for PM
	float length(int roadType) const;
	float curvature(int roadType) const;
	void computePMParameters();

	void load(QString filepath, bool reduce = true);
	void load(QDomNode& node);
	void loadAvenue(QDomNode& node);
	void loadStreet(QDomNode& node);
	void loadArea(QDomNode &node);
	void loadHintLine(QDomNode &node);

	void save(QString filepath);
	void save(QDomDocument& doc, QDomNode& node, const QString &filename);
	void saveAvenue(QDomDocument& doc, QDomNode& node);
	void saveStreet(QDomDocument& doc, QDomNode& node);
	void saveArea(QDomDocument &doc, QDomNode &parent);
	void saveHintLine(QDomDocument &doc, QDomNode &parent);

	void init();

	std::vector<RoadEdgeDescs> shapes(int roadType, float houghScale, float patchDistance);
	void detectAvenueShapes(float houghScale, float patchDistance);
	void detectStreetShapes(float houghScale, float patchDistance);
};

