#include "Terrain.h"
//#include "../render/RenderableQuadList.h"

Terrain::Terrain(int width, int depth, int cellLength) {
	this->width = width;
	this->depth = depth;
	this->cellLength = cellLength;
	modified = false;

	init();
}

Terrain::~Terrain() {
}

TerrainCell& Terrain::getCell(int idxX, int idxY) {
	if (idxX < 0) idxX = 0;
	if (idxX >= numCols) idxX = numCols - 1;
	if (idxY < 0) idxY = 0; 
	if (idxY >= numRows) idxY = numRows - 1;

	return cells[idxY * numCols + idxX];
}

/**
 * Initialize the terrain with the elevation 0m.
 */
void Terrain::init() {
	this->numCols = width / cellLength;
	this->numRows = depth / cellLength;

	this->cells.clear();
	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			TerrainCell cell = TerrainCell(this, j, i, j * cellLength - width/2.0f, i * cellLength - depth/2.0f, cellLength);
			this->cells.push_back(cell);
		}
	}

	this->setModified();
}

float Terrain::getValue(int x, int y) {
	int idxX = ((float)x + width/2.0f) / cellLength;
	int idxY = ((float)y + depth/2.0f) / cellLength;

	return getCell(idxX, idxY).getElevation(x, y);
}

/**
 * Set the elevation at the specified point.
 * Notice that this function does not recalculate the corners' elevation.
 * The caller is responsible for calling recalcCornersElevation method() if necessary.
 *
 * @param x		X coordinate
 * @param y		Y coordinate
 * @param elevation	new elevation
 */
void Terrain::setValue(int x, int y, float elevation) {
	int idxX = ((float)x + width/2.0f) / cellLength;
	int idxY = ((float)y + depth/2.0f) / cellLength;

	getCell(idxX, idxY).setElevation(elevation);

	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			getCell(idxX + i, idxY + j).setCornersElevationModified();
		}
	}

	setModified();
}

void Terrain::addValue(int x, int y, float change) {
	int idxX = ((float)x + width/2.0f) / cellLength;
	int idxY = ((float)y + depth/2.0f) / cellLength;

	getCell(idxX, idxY).addElevation(change);

	//recalcCornersElevation();
	for (int i = -1; i <= 1; ++i) {
		for (int j = -1; j <= 1; ++j) {
			getCell(idxX + i, idxY + j).setCornersElevationModified();
		}
	}

	setModified();
}

/*void Terrain::recalcCornersElevation() {
	for (int i = 0; i < numRows; ++i) {
		for (int j = 0; j < numCols; ++j) {
			cells[i * numCols + j].recalcCornersElevation();
		}
	}
	setModified();
}*/

float Terrain::getSlope(int x, int y) {
	int idxX = ((float)x + width/2.0f) / cellLength;
	int idxY = ((float)y + depth/2.0f) / cellLength;

	return getSlopeAtCell(idxX, idxY);
}

float Terrain::getSlopeAtCell(int idxX, int idxY) {
	return getCell(idxX, idxY).getSlope();
}

void Terrain::generateMesh(VBORenderManager &renderManager, const QString &meshName) {
	if (!modified) return;

	renderManager.removeStaticGeometry(meshName);

	std::vector<Vertex> vert(4*numRows*numCols);

	for (int i = 0; i < numRows; ++i) {
		int y = i * cellLength - depth/2.0f;
		for (int j = 0; j < numCols; ++j) {
			int x = j * cellLength - width/2.0f;

			float value1 = getValue(x, y);
			float value2 = getValue(x + cellLength, y);
			float value3 = getValue(x + cellLength, y + cellLength);
			float value4 = getValue(x, y + cellLength);

			QColor color1 = colorTable.getColor(value1);
			QColor color2 = colorTable.getColor(value2);
			QColor color3 = colorTable.getColor(value3);
			QColor color4 = colorTable.getColor(value4);

			QVector3D n1 = getCell(j, i).getNormalSW();
			QVector3D n2 = getCell(j, i).getNormalSE();
			QVector3D n3 = getCell(j, i).getNormalNE();
			QVector3D n4 = getCell(j, i).getNormalNW();

			vert[(i*numCols+j)*4+0]=Vertex(x, y, getCell(j, i).getElevationSW(), color1.redF(), color1.greenF(), color1.blueF(), n1.x(), n1.y(), n1.z(), 0, 0, 0);// pos color normal texture
			vert[(i*numCols+j)*4+1]=Vertex(x + cellLength, y, getCell(j, i).getElevationSE(), color2.redF(), color2.greenF(), color2.blueF(), n2.x(), n2.y(), n2.z(), 0, 0, 0);// pos color normal texture
			vert[(i*numCols+j)*4+2]=Vertex(x + cellLength, y + cellLength, getCell(j, i).getElevationNE(), color3.redF(), color3.greenF(), color3.blueF(), n3.x(), n3.y(), n3.z(), 0, 0, 0);// pos color normal texture
			vert[(i*numCols+j)*4+3]=Vertex(x, y + cellLength, getCell(j, i).getElevationNW(), color4.redF(), color4.greenF(), color4.blueF(), n4.x(), n4.y(), n4.z(), 0, 0, 0);// pos color normal texture

			//renderable->addQuad(QVector3D(x, y, getCell(j, i).getElevationSW()), QVector3D(x + cellLength, y, getCell(j, i).getElevationSE()), QVector3D(x + cellLength, y + cellLength, getCell(j, i).getElevationNE()), QVector3D(x, y + cellLength, getCell(j, i).getElevationNW()), getCell(j, i).getNormalSW(), getCell(j, i).getNormalSE(), getCell(j, i).getNormalNE(), getCell(j, i).getNormalNW(), color1, color2, color3, color4);
		}
	}

	renderManager.addStaticGeometry(meshName, vert, "", GL_QUADS, 1);//MODE=1 color

	modified = false;
}
