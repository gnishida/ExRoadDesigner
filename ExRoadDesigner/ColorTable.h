#pragma once

#include "glew.h"

#include <vector>
#include <QColor>

class ColorTableEntry {
protected:
	float value;
	QColor color;

public:
	ColorTableEntry(double value, int r, int g, int b);
	~ColorTableEntry() {}
	float getValue();
	QColor getColor();
};

class ColorTable {
public:
	static enum {SCALE_LINEAR = 0, SCALE_LOG10};

protected:
	std::vector<ColorTableEntry> entries;
	int scale;

public:
	ColorTable();
	~ColorTable() {}

	void setScale(int scale);
	void addRGBPoint(double value, int r, int g, int b);
	virtual QColor getColor(double value) = 0;

protected:
	QColor getRGB(double value);
};
