#pragma once

#include "glew.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <QString>

class VBORenderManager;

class VBOTerrain {
private:
	VBORenderManager* rendManager;
	GLuint texId;

	GLuint elementbuffer;
	GLuint vbo;
	GLuint indicesCount;
	GLuint grassText;

	bool initialized;			// flag to check if initialization is done
	int _resolution;			// the number of side of the grid - 1

public:
	cv::Mat layerData;			// the grid that stores the terrain elevation data

public:
	VBOTerrain();

	int resolution() { return _resolution; };
	void init(VBORenderManager* rendManager, int resolution);

	void render(bool drawEditingCircle);
	void updateGaussian(float u, float v, float height, float rad_ratio);
	void excavate(float u, float v, float height, float rad_ratio);
	float getTerrainHeight(float u, float v);
	void loadTerrain(const QString& fileName);
	void saveTerrain(const QString& fileName);

private:
	void updateTexture();
};