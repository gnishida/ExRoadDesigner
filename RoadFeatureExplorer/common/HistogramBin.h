#pragma once

#include <vector>

class HistogramBin {
public:
	float min_range;
	float max_range;

	std::vector<float> values;

public:
	HistogramBin(float min_range, float max_range) : min_range(min_range), max_range(max_range) {}
	~HistogramBin() {}

	bool withinRange(float value);
	void vote(float value);
	float average();
};

