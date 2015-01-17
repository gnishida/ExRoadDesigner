#pragma once


#include "LC_Layer.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class VBORenderManager;

class VBOTerrain {
public:
	VBOTerrain();

	void init(VBORenderManager& rendManager);

	void render(VBORenderManager& rendManager);
	void updateGaussian(float coordX,float coordY,float change,float rad_ratio);
	void updateTerrainNewValue(float coordX,float coordY,float newValue,float rad);
	void excavate(float x, float y, float height, float rad_ratio);
	void smoothTerrain();
	float getTerrainHeight(float xM,float yM,bool actual=false);
	void loadTerrain(QString& fileName);
	void saveTerrain(QString& fileName);

	// edit
	Layer terrainLayer;
	bool initialized;
	int resolutionX;
	int resolutionY;
};