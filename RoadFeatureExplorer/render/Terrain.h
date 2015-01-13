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

#include <render/GeometryObject.h>
#include "TerrainCell.h"
#include <render/TextureManager.h>
#include <common/ElevationColorTable.h>

namespace mylib {

class Terrain : public GeometryObject {
public:
	int width;
	int depth;
	int numCols;
	int numRows;
	int cellLength;
	std::vector<TerrainCell> cells;

	mylib::ElevationColorTable colorTable;

public:
	Terrain(int width, int depth, int cellLength);
	~Terrain();

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

	void _generateMeshVertices(mylib::TextureManager* textureManager);
};

} // namespace mylib