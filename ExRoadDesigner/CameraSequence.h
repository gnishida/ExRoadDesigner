#pragma once

#include "glew.h"

#include "common.h"
#include <QString>
#include <vector>
#include "FlyThroughCamera.h"

class CameraSequence {
public:
	std::vector<FlyThroughCamera> cameras;

public:
	CameraSequence() {}
	~CameraSequence() {}

	void load(const QString &filename, float step = 5.0f);
};

