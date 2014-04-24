#pragma once

#include "glew.h"

#include <QVector3D>

class Terrain;

class TerrainCell {
	friend TerrainCell;

private:
	Terrain* terrain;
	int idxX;
	int idxY;
	float x;
	float y;
	int length;
	float elevation;
	float elevationSW;
	float elevationSE;
	float elevationNW;
	float elevationNE;
	bool cornersElevationModified;

public:
	TerrainCell(Terrain* terrain, int idxX, int idxY, float x, float y, int length);
	~TerrainCell();

	/** getter for x */
	float getX() { return x; }

	/** getter for y */
	float getY() { return y; }

	/** getter for length */
	int getLength() { return length; }

	/** getter for elevation */
	float getElevation() { return elevation; }

	/** setter for elevation */
	void setElevation(float elevation) { this->elevation = elevation; }

	/** getter for elevationSW; */
	float getElevationSW();

	/** getter for elevationSE; */
	float getElevationSE();

	/** getter for elevationWW; */
	float getElevationNW();

	/** getter for elevationNE; */
	float getElevationNE();

	bool getCornersElevationModified() { return cornersElevationModified; }
	void setCornersElevationModified() { cornersElevationModified = true; }

	float getElevation(float x, float y);
	void addElevation(int increase);

	QVector3D getNormal();
	QVector3D getNormalSW();
	QVector3D getNormalSE();
	QVector3D getNormalNW();
	QVector3D getNormalNE();

	void recalcCornersElevation();
	float getSlope();
};
