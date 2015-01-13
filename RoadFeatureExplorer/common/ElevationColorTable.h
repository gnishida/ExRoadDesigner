#pragma once

#include "ColorTable.h"

namespace mylib {

class ElevationColorTable : public ColorTable {
public:
	ElevationColorTable();
	~ElevationColorTable() {}

	QColor getColor(double value);
};

} // namespace mylib