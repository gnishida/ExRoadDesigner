#pragma once

#include "VBORenderManager.h"
#include "BlockSet.h"

class BlockMeshGenerator {
private:
	static std::vector<QString> sideWalkFileNames;
	static std::vector<QVector3D> sideWalkScale;
	static std::vector<QString> grassFileNames;
	static bool initialized;

public:
	BlockMeshGenerator() {}

	static void init();
	static void generateBlockMesh(VBORenderManager& rendManager, BlockSet& blocks);
	static void generate2DBlockMesh(VBORenderManager& rendManager, BlockSet& blocks);
};

