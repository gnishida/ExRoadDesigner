#pragma once

#include "glew.h"

#include "TerrainCell.h"
#include "ElevationColorTable.h"
#include "VBORenderManager.h"

class Terrain {// : public GeometryObject {
public:
	int width;
	int depth;
	int numCols;
	int numRows;
	int cellLength;
	std::vector<TerrainCell> cells;

	ElevationColorTable colorTable;
	bool modified;

public:
	Terrain(int width, int depth, int cellLength);
	~Terrain();

	void setModified() { modified = true; }
	int getNumCols() { return numCols; }
	int getNumRows() { return numRows; }
	int getCellLength() { return cellLength; }
	TerrainCell& getCell(int idxX, int idxY);

	void init();
	float getValue(int x, int y);
	void setValue(int x, int y, float elevation);
	void addValue(int x, int y, float change);
	//void recalcCornersElevation();
	float getSlope(int x, int y);
	float getSlopeAtCell(int idxX, int idxY);

	void generateMesh(VBORenderManager &renderManager, const QString &meshName);
};
