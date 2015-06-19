#pragma once

#include "glew.h"

#include <vector>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include "BBox.h"
#include "Polygon2D.h"
#include "RoadGraph.h"

class GraphUtil {
protected:
	GraphUtil() {}

public:
	// Vertex related functions
	static int getNumVertices(RoadGraph& roads, bool onlyValidVertex = true);
	static int getNumVertices(RoadGraph& roads, const QVector2D& pos, float radius);
	static int getNumConnectedVertices(RoadGraph& roads, RoadVertexDesc start, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph& roads, int index, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph& roads, const QVector2D& pt, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph& roads, const QVector2D& pt, RoadVertexDesc ignore, bool onlyValidVertex = true);
	static RoadVertexDesc getVertex(RoadGraph& roads, const QVector2D& pt, float angle, float angle_threshold, bool onlyValidVertex = true);
	static bool getVertex(RoadGraph& roads, RoadVertexDesc srcDesc, float distance_threshold, float angle, float angle_threshold, RoadVertexDesc& nearest_desc, bool onlyValidVertex = true);
	static bool getVertex(RoadGraph& roads, const QVector2D& pos, float threshold, RoadVertexDesc& desc, bool onlyValidVertex = true);
	static bool getVertex(RoadGraph& roads, RoadVertexDesc v, float threshold, RoadVertexDesc& desc, bool onlyValidVertex = true);
	static bool getVertex(RoadGraph& roads, const QVector2D& pos, float threshold, RoadVertexDesc ignore, RoadVertexDesc& desc, bool onlyValidVertex = true);
	static bool getVertex2(RoadGraph& roads, const QVector2D& pos, float threshold, RoadVertexDesc ignore, RoadVertexDesc& desc, bool onlyValidVertex = true);
	static bool getVertexInArea(RoadGraph& roads, const QVector2D& pos, const BBox &area, RoadVertexDesc& desc);
	static RoadVertexDesc addVertex(RoadGraph& roads, RoadVertexPtr v);
	static void moveVertex(RoadGraph& roads, RoadVertexDesc v, const QVector2D& pt);
	static int getDegree(RoadGraph& roads, RoadVertexDesc v, bool onlyValidEdge = true);
	static std::vector<RoadVertexDesc> getVertices(RoadGraph* roads, bool onlyValidVertex = true);
	static void removeIsolatedVertices(RoadGraph& roads, bool onlyValidVertex = true);
	static void snapVertex(RoadGraph& roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static RoadVertexDesc getCentralVertex(RoadGraph& roads);
	static float getDensity(RoadGraph& roads, const QVector2D& pos, float radius);
	static bool hasRedundantEdge(RoadGraph& roads, RoadVertexDesc desc, const Polyline2D &polyline, float threshold);
	static bool tshape(RoadGraph &roads, RoadVertexDesc v, RoadEdgeDesc edge);
	static void setVertexType(RoadGraph &roads);

	// Edge related functions
	static RoadEdgeDesc getEdge(RoadGraph& roads, int index, bool onlyValidEdge = true);
	static float getTotalEdgeLength(RoadGraph& roads, RoadVertexDesc v);
	static int getNumEdges(RoadGraph& roads, bool onlyValidEdge = true);
	static int getNumEdges(RoadGraph& roads, RoadVertexDesc v, bool onlyValidEdge = true);
	static int getNumEdges(RoadGraph& roads, RoadVertexDesc v, int roadType, bool onlyValidEdge = true);
	static RoadEdgeDesc addEdge(RoadGraph& roads, RoadVertexDesc src, RoadVertexDesc tgt, unsigned int type, unsigned int lanes, bool oneWay = false, bool link = false, bool roundabout = false);
	static RoadEdgeDesc addEdge(RoadGraph& roads, RoadVertexDesc src, RoadVertexDesc tgt, RoadEdgePtr edge);
	static RoadEdgeDesc addEdge(RoadGraph& roads, const Polyline2D &polyline, unsigned int type, unsigned int lanes, bool oneWay = false, bool link = false, bool roundabout = false);
	static bool hasEdge(RoadGraph& roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static bool hasSimilarEdge(RoadGraph& roads, RoadVertexDesc desc1, RoadVertexDesc desc2, const Polyline2D &polyline);
	static RoadEdgeDesc getEdge(RoadGraph& roads, RoadVertexDesc src, RoadVertexDesc tgt, bool onlyValidEdge = true);
	static void getOrderedPolyLine(RoadGraph& roads, RoadEdgeDesc e, std::vector<QVector2D>& polyline);
	static Polyline2D orderPolyLine(RoadGraph& roads, RoadEdgeDesc e, RoadVertexDesc src);
	static Polyline2D orderPolyLine(RoadGraph& roads, RoadEdgeDesc e, RoadVertexDesc src, float angle);
	static Polyline3D orderPolyLine3D(RoadGraph& roads, RoadEdgeDesc e, RoadVertexDesc src, float angle);
	static void moveEdge(RoadGraph& roads, RoadEdgeDesc e, QVector2D& src_pos, QVector2D& tgt_pos);
	static void movePolyline(RoadGraph& roads, Polyline2D &polyline, const QVector2D& src_pos, const QVector2D& tgt_pos);
	static bool isSimilarPolyline(const Polyline2D &polyline1, const Polyline2D &polyline2);
	static bool removeDeadEnd(RoadGraph& roads);
	static void removeIsolatedEdges(RoadGraph& roads, bool onlyValidEdge = true);
	static void removeLoop(RoadGraph& roads);
	static RoadVertexDesc splitEdge(RoadGraph &roads, RoadEdgeDesc edge_desc, const QVector2D& pt);
	static RoadVertexDesc splitEdge(RoadGraph &roads, RoadEdgeDesc edge_desc, const QVector2D& pt, RoadEdgeDesc &edge1, RoadEdgeDesc &edge2);
	static bool hasCloseEdge(RoadGraph* roads, RoadVertexDesc v1, RoadVertexDesc v2, float angle_threshold = 0.3f);
	static bool isIntersect(RoadGraph &smallRoads, RoadGraph &largeRoads);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline, QVector2D &intPoint);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline, RoadVertexDesc srcDesc, QVector2D &intPoint);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline, RoadEdgeDesc ignoreEdge);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline1, const Polyline2D &polyline2);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline1, const Polyline2D &polyline2, QVector2D &intPoint);
	static bool isIntersect(RoadGraph &roads, const Polyline2D &polyline, RoadVertexDesc srcDesc, RoadEdgeDesc &nearestEdgeDesc, QVector2D &intPoint);
	static std::vector<QVector2D> simplifyPolyLine(std::vector<QVector2D>& polyline, float threshold);
	static void removeShortEdges(RoadGraph& roads, float threshold);
	static void removeLinkEdges(RoadGraph& roads);
	static Polyline2D finerEdge(RoadGraph& roads, RoadEdgeDesc e, float step = 1.0f);
	static Polyline2D finerEdge(Polyline2D &polyline, float step = 1.0f);
	static float distance(RoadGraph& roads, const QVector2D& pt, RoadEdgeDesc e, QVector2D &closestPt);
	static void cleanEdges(RoadGraph &roads);
	static void cleanEdge(RoadEdgePtr &edge);
	static void cleanPolyline(Polyline2D &polyline);
	static bool isStraightEdge(RoadGraph &roads, RoadVertexDesc v, RoadEdgeDesc e);
	static bool isPotentiallyStraightEdge(RoadGraph &roads, RoadVertexDesc v, RoadEdgeDesc e);
	static RoadVertexDesc cutoffEdge(RoadGraph &roads, RoadEdgeDesc edge, RoadVertexDesc v_desc, const QVector2D &pt);
	static Polyline2D getAdjoiningPolyline(RoadGraph& roads, RoadVertexDesc v_desc);
	static Polyline2D getAdjoiningPolyline(RoadGraph& roads, RoadVertexDesc v_desc, RoadVertexDesc& root_desc);

	// File I/O
	static void loadRoads(RoadGraph& roads, const QString& filename, int roadType = 0);
	static void saveRoads(RoadGraph& roads, const QString& filename);

	// The entire graph related functions
	static void copyRoads(RoadGraph& srcRoads, RoadGraph& dstRoads);
	static void mergeRoads(RoadGraph& roads1, RoadGraph& roads2);
	static BBox getAABoundingBox(RoadGraph& roads, bool checkPolyline = false);
	static BBox getBoudingBox(RoadGraph& roads, float theta1, float theta2, float theta_step = 0.087f);
	static void extractRoads(RoadGraph& roads, int roadType = 0);
	static void extractRoads(RoadGraph& roads, Polygon2D& area, bool strict, int roadType = 0);
	static void extractRoads2(RoadGraph& roads, const Polygon2D& area, int roadType = 0);
	static void trim(RoadGraph& roads, const Polygon2D& area);
	static void subtractRoads(RoadGraph& roads, Polygon2D& area, bool strict);
	static void subtractRoads2(RoadGraph& roads, Polygon2D& area);
	static void perturb(RoadGraph &roads, const Polygon2D &area, float factor);
	static void removeSelfIntersectingRoads(RoadGraph &roads);
	static void normalizeLoop(RoadGraph &roads);

	// Connectivity related functions
	static std::vector<RoadVertexDesc> getNeighbors(RoadGraph& roads, RoadVertexDesc v, bool onlyValidVertex = true);
	static bool isNeighbor(RoadGraph& roads, RoadVertexDesc v1, RoadVertexDesc v2);
	static bool isConnected(RoadGraph& roads, RoadVertexDesc desc1, RoadVertexDesc desc2, bool onlyValidEdge = true);
	static RoadVertexDesc findConnectedNearestNeighbor(RoadGraph* roads, const QVector2D &pt, RoadVertexDesc v);
	static bool getEdge(RoadGraph& roads, const QVector2D &pt, float threshold, RoadEdgeDesc& e, bool onlyValidEdge = true);
	static bool getEdge(RoadGraph& roads, const QVector2D &pt, float threshold, RoadVertexDesc srcDesc, RoadEdgeDesc& e, QVector2D &closestPt, bool onlyValidEdge = true);
	static bool getEdge(RoadGraph& roads, RoadVertexDesc v, float threshold, RoadEdgeDesc& e, QVector2D &closestPt, bool onlyValidEdge = true);
	static RoadEdgeDesc getEdge(RoadGraph& roads, RoadVertexDesc v, QVector2D& closestPt, bool onlyValidEdge = true);

	// The road graph modification functions
	static void clean(RoadGraph& roads);
	static void reduce(RoadGraph& roads);
	static bool reduce(RoadGraph& roads, RoadVertexDesc desc);
	static void simplify(RoadGraph& roads, float dist_threshold);
	static void simplify2(RoadGraph& srcRoad, float dist_threshold);
	static void simplify3(RoadGraph& srcRoad, float dist_threshold);
	static void normalize(RoadGraph& roads);
	static void normalize(RoadGraph& roads, float step_size);
	static void singlify(RoadGraph& roads);
	static void planarify(RoadGraph& roads);
	static bool planarifyOne(RoadGraph& roads);
	static void skeltonize(RoadGraph* roads);
	static void rotate(RoadGraph& roads, float theta, const QVector2D& rotationCenter = QVector2D(0, 0));
	static void translate(RoadGraph& roads, const QVector2D& offset);
	static void scale(RoadGraph& roads, const BBox& bbox1, const BBox& bbox2);
	static void snapDeadendEdges(RoadGraph& roads, float threshold);
	static void snapDeadendEdges2(RoadGraph& roads, int degree, float threshold);
	static void removeShortDeadend(RoadGraph& roads, float threshold);
	static void realize(RoadGraph &roads);
	static BBox bbox(RoadGraph &roads);
	static Polygon2D hull(RoadGraph &roads);

	// statistic
	static float getTotalEdgeLength(RoadGraph &roads);
	static void computeStatistics(RoadGraph &roads, float &avgEdgeLength, float &varEdgeLength, float &avgEdgeCurvature, float &varEdgeCurvature);
	static void computeStatistics(RoadGraph &roads, const QVector2D &pt, float dist, float &avgEdgeLength, float &varEdgeLength, float &avgEdgeCurvature, float &varEdgeCurvature);
	static float computeRateOfPatches(RoadGraph& roads);

	static void buildEmbedding(RoadGraph &roads, std::vector<std::vector<RoadEdgeDesc> > &embedding);

	// Others
	static bool isRoadTypeMatched(int type, int ref_type);

	// OpenCV
	static void convertToMat(RoadGraph& roads, cv::Mat_<uchar>& mat, const cv::Size& size, int width = 3, bool flip = true);
	static void drawRoadSegmentOnMat(RoadGraph& roads, RoadEdgeDesc e, cv::Mat& mat, int width = 3, int brightness = 255);

	// Simple Road Generation
	static void generateRegularGrid(RoadGraph &roads, float size, float avenueInterval, float streetInterval);
	static void generateCurvyGrid(RoadGraph &roads, float size, float avenueInterval, float streetInterval);
};

