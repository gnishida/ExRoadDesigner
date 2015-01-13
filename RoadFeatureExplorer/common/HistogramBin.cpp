#include "HistogramBin.h"

bool HistogramBin::withinRange(float value) {
	if (value >= min_range && value < max_range) return true;
	else return false;
}

void HistogramBin::vote(float value) {
	values.push_back(value);
}

float HistogramBin::average() {
	if (values.size() == 0) return 0.0f;

	float total = 0.0f;
	for (int i = 0; i < values.size(); ++i) {
		total += values[i];
	}
	return total / (float)values.size();
}
