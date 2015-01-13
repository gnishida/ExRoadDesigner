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

#include "TerrainCell.h"
#include "Terrain.h"
#include <common/Util.h>
#include <common/common.h>

namespace mylib {

TerrainCell::TerrainCell(Terrain* terrain, int idxX, int idxY, float x, float y, int length) {
	this->terrain = terrain;
	this->idxX = idxX;
	this->idxY = idxY;
	this->x = x;
	this->y = y;
	this->length = length;
	elevation = 0;
	elevationSW = elevationSE = elevationNW = elevationNE = elevation;
	cornersElevationModified = false;
}

TerrainCell::~TerrainCell() {
}

float TerrainCell::getElevationSW() {
	if (cornersElevationModified) recalcCornersElevation();

	return elevationSW;
}

float TerrainCell::getElevationSE() {
	if (cornersElevationModified) recalcCornersElevation();

	return elevationSE;
}

float TerrainCell::getElevationNW() {
	if (cornersElevationModified) recalcCornersElevation();

	return elevationNW;
}

float TerrainCell::getElevationNE() {
	if (cornersElevationModified) recalcCornersElevation();

	return elevationNE;
}

float TerrainCell::getElevation(float x, float y) {
	if (cornersElevationModified) recalcCornersElevation();

	QVector3D pSW(this->x, this->y, this->elevationSW);
	QVector3D pSE(this->x + length, this->y, this->elevationSE);
	QVector3D pNE(this->x + length, this->y + length, this->elevationNE);
	QVector3D pNW(this->x, this->y + length, this->elevationNW);

	QVector2D p(x, y);

	if (SQR(this->x + length - x) + SQR(this->y - y) < SQR(this->x - x) + SQR(this->y + length - y)) {
		return Util::barycentricInterpolation(pSW, pSE, pNE, p);
	} else {
		return Util::barycentricInterpolation(pSW, pNE, pNW, p);
	}
}

void TerrainCell::addElevation(int increase) {
	elevation += increase;
}

QVector3D TerrainCell::getNormal() {
	if (cornersElevationModified) recalcCornersElevation();

	QVector3D v0(length, 0, elevationSE - elevationSW);
	QVector3D v1(0, length, elevationNW - elevationSW);

	v0.normalize();
	v1.normalize();
	return QVector3D::crossProduct(v0, v1);
}

QVector3D TerrainCell::getNormalSW() {
	QVector3D n1 = getNormal();
	QVector3D n2 = terrain->getCell(idxX-1, idxY).getNormal();
	QVector3D n3 = terrain->getCell(idxX-1, idxY-1).getNormal();
	QVector3D n4 = terrain->getCell(idxX, idxY-1).getNormal();

	return (n1 + n2 + n3 + n4) / 4;
}

QVector3D TerrainCell::getNormalSE() {
	QVector3D n1 = getNormal();
	QVector3D n2 = terrain->getCell(idxX+1, idxY).getNormal();
	QVector3D n3 = terrain->getCell(idxX+1, idxY-1).getNormal();
	QVector3D n4 = terrain->getCell(idxX, idxY-1).getNormal();

	return (n1 + n2 + n3 + n4) / 4;
}

QVector3D TerrainCell::getNormalNW() {
	QVector3D n1 = getNormal();
	QVector3D n2 = terrain->getCell(idxX-1, idxY).getNormal();
	QVector3D n3 = terrain->getCell(idxX-1, idxY+1).getNormal();
	QVector3D n4 = terrain->getCell(idxX, idxY+1).getNormal();

	return (n1 + n2 + n3 + n4) / 4;
}

QVector3D TerrainCell::getNormalNE() {
	QVector3D n1 = getNormal();
	QVector3D n2 = terrain->getCell(idxX+1, idxY).getNormal();
	QVector3D n3 = terrain->getCell(idxX+1, idxY+1).getNormal();
	QVector3D n4 = terrain->getCell(idxX, idxY+1).getNormal();

	return (n1 + n2 + n3 + n4) / 4;
}

void TerrainCell::recalcCornersElevation() {
	if (!cornersElevationModified) return;

	elevationSW = (terrain->getCell(idxX, idxY).getElevation() + terrain->getCell(idxX-1, idxY).getElevation() + terrain->getCell(idxX, idxY-1).getElevation() + terrain->getCell(idxX-1, idxY-1).getElevation()) * 0.25;
	elevationSE = (terrain->getCell(idxX, idxY).getElevation() + terrain->getCell(idxX+1, idxY).getElevation() + terrain->getCell(idxX, idxY-1).getElevation() + terrain->getCell(idxX+1, idxY-1).getElevation()) * 0.25;
	elevationNW = (terrain->getCell(idxX, idxY).getElevation() + terrain->getCell(idxX-1, idxY).getElevation() + terrain->getCell(idxX, idxY+1).getElevation() + terrain->getCell(idxX-1, idxY+1).getElevation()) * 0.25;
	elevationNE = (terrain->getCell(idxX, idxY).getElevation() + terrain->getCell(idxX+1, idxY).getElevation() + terrain->getCell(idxX, idxY+1).getElevation() + terrain->getCell(idxX+1, idxY+1).getElevation()) * 0.25;

	cornersElevationModified = false;
}

float TerrainCell::getSlope() {
	if (cornersElevationModified) recalcCornersElevation();

	return fabs(elevationSW - elevation) + fabs(elevationSE - elevation) + fabs(elevationNW - elevation) + fabs(elevationNE - elevation);
}

} // namespace mylib