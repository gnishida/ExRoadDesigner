#include "ColorTable.h"

ColorTableEntry::ColorTableEntry(double value, int r, int g, int b) {
	this->value = value;
	color = QColor(r, g, b);
}

float ColorTableEntry::getValue() {
	return value;
}

QColor ColorTableEntry::getColor() {
	return color;
}

ColorTable::ColorTable() {
	scale = ColorTable::SCALE_LINEAR;
}

void ColorTable::setScale(int scale) {
	this->scale = scale;
}

void ColorTable::addRGBPoint(double value, int r, int g, int b) {
	ColorTableEntry e(value, r, g, b);

	for (int i = 0; i < entries.size(); ++i) {
		if (value < entries[i].getValue()) {
			std::vector<ColorTableEntry>::iterator it = entries.begin() + i;
			entries.insert(it, e);
			return;
		}
	}

	entries.push_back(e);
}

QColor ColorTable::getRGB(double value) {
	ColorTableEntry prev(0, 0, 0, 0);

	for (int i = 0; i < entries.size(); ++i) {
		if (value < entries[i].getValue()) {
			if (scale == ColorTable::SCALE_LINEAR) {
				double t = (value - prev.getValue()) / (entries[i].getValue() - prev.getValue());
				int r = (entries[i].getColor().red() - prev.getColor().red()) * t + prev.getColor().red();
				int g = (entries[i].getColor().green() - prev.getColor().green()) * t + prev.getColor().green();
				int b = (entries[i].getColor().blue() - prev.getColor().blue()) * t + prev.getColor().blue();
				return QColor(r, g, b);
			} else {
				double t = log(value - prev.getValue()) / log(entries[i].getValue() - prev.getValue());
				if (t < 0) t = 0;
				int r = (entries[i].getColor().red() - prev.getColor().red()) * t + prev.getColor().red();
				int g = (entries[i].getColor().green() - prev.getColor().green()) * t + prev.getColor().green();
				int b = (entries[i].getColor().blue() - prev.getColor().blue()) * t + prev.getColor().blue();
				return QColor(r, g, b);
			}
		} else {
			prev = entries[i];
		}
	}

	return prev.getColor();
}

