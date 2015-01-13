/************************************************************************************************
*		VBO Block Class
*		@author igarciad
************************************************************************************************/
#pragma once

#ifndef Q_MOC_RUN
#include <boost/graph/adjacency_list.hpp>
#endif

#include "VBOParcel.h"
#include <QVector3D>
#include "Polygon3D.h"

/**
* Block.
**/

class Block
{

public:
	/**
	* Constructor.
	**/
	Block();

	/**
	* Destructor.
	**/
	~Block();

	/**
	* Copy constructor.
	**/
	Block(const Block &ref)
	{					
		blockContour = ref.blockContour;
		blockContourRoadsWidths = ref.blockContourRoadsWidths;
		myNeighborhoodID = ref.myNeighborhoodID;
		myCityID = ref.myCityID;
		myParcels = ref.myParcels;
		bbox = ref.bbox;
		randSeed = ref.randSeed;
		myPlaceTypeIdx = ref.myPlaceTypeIdx;
		myColor = ref.myColor;
		isPark=ref.isPark;
	}

	/**
	* Assignment operator.
	**/
	inline Block &operator=(const Block &ref)
	{	
		blockContour = ref.blockContour;
		blockContourRoadsWidths = ref.blockContourRoadsWidths;
		myNeighborhoodID = ref.myNeighborhoodID;
		myCityID = ref.myCityID;
		myParcels = ref.myParcels;
		bbox = ref.bbox;
		randSeed = ref.randSeed;
		myPlaceTypeIdx = ref.myPlaceTypeIdx;
		myColor = ref.myColor;
		isPark=ref.isPark;
		return (*this);
	}

	/**
	* Clear
	**/
	void clear(void);

	void computeMyBBox3D(void);

	inline void setMyPlaceTypeIdx(int inIdx)
	{
		myPlaceTypeIdx = inIdx;
	}

	inline int getMyPlaceTypeIdx(void)
	{
		return myPlaceTypeIdx;
	}

	inline int getMyNeighborhoodID(void)
	{
		return myNeighborhoodID;
	}

	inline int getMyCityID(void)
	{						
		return myCityID;
	}


	/**
	* Compute parcel adjacency graph
	**/
	void computeParcelAdjacencyGraph(void);

	void buildableAreaMock(void);

	static void findParcelFrontAndBackEdges(Block &inBlock, Parcel &inParcel,
		std::vector<int> &frontEdges,
		std::vector<int> &rearEdges,
		std::vector<int> &sideEdges );

	/**
	* Contour of the block.
	**/
	Polygon3D blockContour;

	/**
	* Boundary road widths
	**/
	std::vector<float> blockContourRoadsWidths;

	/**
	* Adapt block to vboRenderManager
	**/
	//void adaptBlockToTerrain(MTC::geometry::ElevationGrid *elGrid);

	/**
	* BGL Graph of parcels into which block is subdivided.
	**/				 

	typedef boost::adjacency_list
		<boost::vecS, boost::vecS, boost::undirectedS, Parcel> parcelGraph;				

	typedef boost::graph_traits<parcelGraph>::vertex_descriptor parcelGraphVertexDesc;

	typedef boost::graph_traits<parcelGraph>::vertex_iterator parcelGraphVertexIter;

	typedef boost::graph_traits<parcelGraph>::edge_iterator parcelGraphEdgeIter;

	typedef boost::graph_traits<parcelGraph>::adjacency_iterator parcelGraphAdjIter;// Carlos

	bool splitBlockParcelsWithRoadSegment(std::vector<QVector3D> &roadSegmentGeometry,
		float roadSegmentWidth, BBox3D roadSegmentBBox3D, std::list<Parcel> &blockParcels);

	bool areParcelsAdjacent(parcelGraphVertexIter &p0, parcelGraphVertexIter &p1);

	parcelGraph myParcels;

	/**
	* Pointer to my place type
	**/			
	QVector3D myColor;

	BBox3D bbox;

	int randSeed;
	bool isPark;

private:
	int myPlaceTypeIdx;
	int myNeighborhoodID;
	int myCityID;
};

