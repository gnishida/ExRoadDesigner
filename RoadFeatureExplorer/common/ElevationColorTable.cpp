#include "ElevationColorTable.h"

namespace mylib {

ElevationColorTable::ElevationColorTable() {
	/*
	addRGBPoint(-100, 0, 0, 0);
	addRGBPoint(-20, 30, 30, 105);
	addRGBPoint(-1E-6, 255, 255, 255);
	addRGBPoint(0, 166, 160, 131);
	addRGBPoint(200, 205, 200, 182);
	addRGBPoint(3000, 255, 255, 255);
	*/

	addRGBPoint(-100, 0, 0, 0);
	addRGBPoint(-20, 30, 30, 105);
	addRGBPoint(-1E-6, 255, 255, 255);
	addRGBPoint(0, 212, 210, 195);
	addRGBPoint(200, 235, 232, 222);
	addRGBPoint(3000, 255, 255, 255);

	/*
	addRGBPoint(-100, 0, 0, 0);
	addRGBPoint(-20, 30, 30, 105);
	addRGBPoint(-1E-6, 255, 255, 255);
	addRGBPoint(0, 233, 229, 220);
	addRGBPoint(200, 240, 236, 227);
	addRGBPoint(3000, 255, 255, 255);
	*/
}

QColor ElevationColorTable::getColor(double value) {
	return getRGB(value);
}

} // namespace mylib