/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#pragma once

#include <QVector3D>

namespace mylib {

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

} // namespace mylib