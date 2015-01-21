#pragma once

#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#endif

#include "VBORenderManager.h"
#include "VBOParcel.h"
#include <QVector3D>
#include "Polygon3D.h"
#include "Polygon2D.h"

class Block {
public:
	/**
	* BGL Graph of parcels into which block is subdivided.
	**/
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, Parcel> parcelGraph;
	typedef boost::graph_traits<parcelGraph>::vertex_descriptor parcelGraphVertexDesc;
	typedef boost::graph_traits<parcelGraph>::vertex_iterator parcelGraphVertexIter;
	typedef boost::graph_traits<parcelGraph>::edge_iterator parcelGraphEdgeIter;
	typedef boost::graph_traits<parcelGraph>::adjacency_iterator parcelGraphAdjIter;// Carlos


public:
	parcelGraph myParcels;
	Polygon3D blockContour;
	Polygon3D sidewalkContour;
	std::vector<float> sidewalkContourRoadsWidths;
	bool isPark;
	bool valid;

public:
	Block() : isPark(false), valid(true) {}

	void clear();
	void buildableAreaMock();

	static void findParcelFrontAndBackEdges(Block &inBlock, Parcel &inParcel, std::vector<int> &frontEdges,	std::vector<int> &rearEdges, std::vector<int> &sideEdges);
	
	bool splitBlockParcelsWithRoadSegment(std::vector<QVector3D> &roadSegmentGeometry, float roadSegmentWidth, BBox3D roadSegmentBBox3D, std::list<Parcel> &blockParcels);
};

