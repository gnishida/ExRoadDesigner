#pragma once

#include "ColorTable.h"

class ElevationColorTable : public ColorTable {
public:
	ElevationColorTable();
	~ElevationColorTable() {}

	QColor getColor(double value);
};

