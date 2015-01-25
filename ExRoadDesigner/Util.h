#pragma once

#include "glew.h"

#include <QGenericMatrix>
#include "common.h"
#include "Polyline2D.h"

class Util {
	static const float MTC_FLOAT_TOL;

protected:
	Util();

public:
	// 3D
	static QVector3D calculateNormal(const QVector3D& p0, const QVector3D& p1, const QVector3D& p2);


	static float pointSegmentDistanceXY(const QVector3D &a, const QVector3D &b, const QVector3D &c, bool segmentOnly = true);
	static QVector2D projLatLonToMeter(const QVector2D &latLon, const QVector2D &centerLatLon);
	static QVector2D projLatLonToMeter(double longitude, double latitude, const QVector2D &centerLatLon);

	static bool segmentSegmentIntersectXY(const QVector2D& a, const QVector2D& b, const QVector2D& c, const QVector2D& d, float *tab, float *tcd, bool segmentOnly, QVector2D &intPoint);
	static bool segmentSegmentIntersectXY3D(const QVector3D& a, const QVector3D& b, const QVector3D& c, const QVector3D& d, float *tab, float *tcd, bool segmentOnly, QVector3D &intPoint);
	static float pointSegmentDistanceXY(const QVector2D& a, const QVector2D& b, const QVector2D& c, QVector2D& closestPtInAB);

	static bool leftTurn(const QVector2D& a, const QVector2D& b, const QVector2D& c);
	static bool leftTurn(const QVector2D& v1, const QVector2D& v2);

	// angle computatiopn
	static float deg2rad(float deg);
	static float rad2deg(float rad);
	static float normalizeAngle(float angle);
	static float diffAngle(const QVector2D& dir1, const QVector2D& dir2, bool absolute = true);
	static float diffAngle(const QVector3D& dir1, const QVector3D& dir2, bool absolute = true);
	static float diffAngle(float angle1, float angle2, bool absolute = true);
	static bool withinAngle(float angle, float angle1, float angle2);
	static float angleThreePoints(const QVector3D& pa, const QVector3D& pb, const QVector3D& pc);
	static QVector2D rotate(const QVector2D &pt, float rad);
	static QVector2D rotate(const QVector2D &pt, float rad, const QVector2D &orig);

	// coordinate conversion
	static void cartesian2polar(const QVector2D &pt, float &radius, float &theta);
	static QVector2D transform(const QVector2D &pt, const QVector2D &sourcePt, float rad, const QVector2D &targetPt);

	// curvature
	static float curvature(const Polyline2D &polyline);

	// random
	static float genRand();
	static float genRand(float a, float b);
	static float genRandNormal(float mean, float variance);
	static int sampleFromCdf(std::vector<float> &cdf);
	static int sampleFromPdf(std::vector<float> &pdf);

	// Barycentric interpolation
	static float barycentricInterpolation(const QVector3D& p0, const QVector3D& p1, const QVector3D& p2, const QVector2D& p);

	// projection
	static QVector2D projectTo2D(const QVector3D &pt);

	static bool getIrregularBisector(const QVector3D& p0, const QVector3D& p1, const QVector3D& p2, float d01, float d12, QVector3D& intPt);
	static bool getIrregularBisector(const QVector2D& p0, const QVector2D& p1, const QVector2D& p2, float d01, float d12, QVector2D& intPt);
};
