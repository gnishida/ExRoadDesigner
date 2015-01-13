#include "Histogram.h"
#include <exception>

Histogram::Histogram(int start, int end, int step) {
	this->start = start;
	this->end = end;
	this->step = step;

	int size = (end - start) / step;
	bins.resize(size);
	for (int i = 0; i < bins.size(); ++i) {
		bins[i] = 0;
	}
}

void Histogram::clear() {
	bins.clear();
}

void Histogram::add(int value) {
	if (value < start) return;
	if (value >= end) return;

	int index = (value - start) / step;
	bins[index]++;
}

int Histogram::size() {
	return bins.size();
}
