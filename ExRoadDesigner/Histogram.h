#pragma once

#include <QMap>
#include <vector>

class Histogram {
public:
	int start;
	int end;
	int step;
	std::vector<int> bins;
	//std::map<std::pair<int, int>, int> bins;

public:
	Histogram(int start, int end, int step);
	~Histogram() {}

	void clear();
	void add(int value);
	int size();
};

