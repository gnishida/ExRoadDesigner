#include "BlockMeshGenerator.h"
#include <QDir>
#include "global.h"

bool BlockMeshGenerator::initialized = false;
std::vector<QString> BlockMeshGenerator::sideWalkFileNames;
std::vector<QVector3D> BlockMeshGenerator::sideWalkScale;
std::vector<QString> BlockMeshGenerator::grassFileNames;

/**
 * テクスチャ画像の読み込み
 */
void BlockMeshGenerator::init() {
	QString pathName="../data/textures/LC";
	// 3. sidewalk
	QDir directorySW(pathName+"/sidewalk/");
	QStringList nameFilter;
	nameFilter << "*.png" << "*.jpg" << "*.gif";
	QStringList list = directorySW.entryList( nameFilter, QDir::Files );
	for(int lE=0;lE<list.size();lE++){
		if(QFile::exists(pathName+"/sidewalk/"+list[lE])){
			sideWalkFileNames.push_back(pathName+"/sidewalk/"+list[lE]);
			QStringList scaleS=list[lE].split("_");
			if(scaleS.size()!=4)
				sideWalkScale.push_back(QVector3D(1.0f,1.0f,0));
			else{
				sideWalkScale.push_back(QVector3D(scaleS[1].toFloat(),scaleS[2].toFloat(),0));
			}
		}
	}
	grassFileNames.push_back("../data/textures/LC/grass/grass01.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass02.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass03.jpg");
	grassFileNames.push_back("../data/textures/LC/grass/grass04.jpg");
	printf("-->Initialized LC\n");

	initialized = true;
}

void BlockMeshGenerator::generateBlockMesh(VBORenderManager& rendManager, BlockSet& blocks) {
	if (!initialized) init();

	// 3Dモデルを生成する
	rendManager.removeStaticGeometry("3d_blocks");
	rendManager.removeStaticGeometry("3d_parks");

	const float deltaZ = 2.3f;

	for (int i = 0; i < blocks.size(); ++i) {
		if (!blocks[i].valid) continue;
		//blocks[i].adaptToTerrain(&rendManager);

		// 歩道の3Dモデルを生成
		{
			std::vector<QVector3D> polygon;
			for (int pi = 0; pi < blocks[i].sidewalkContour.contour.size(); ++pi) {
				//float z = rendManager.getTerrainHeight(blocks[i].sidewalkContour[pi].x(), blocks[i].sidewalkContour[pi].y()) + deltaZ;
				polygon.push_back(QVector3D(blocks[i].sidewalkContour[pi].x(), blocks[i].sidewalkContour[pi].y(), deltaZ));
			}

			int randSidewalk=2;//qrand()%grassFileNames.size();
			rendManager.addStaticGeometry2("3d_blocks", polygon, 0.0f, false, sideWalkFileNames[randSidewalk], GL_QUADS, 2|mode_AdaptTerrain, sideWalkScale[randSidewalk], QColor());

			// 側面
			std::vector<Vertex> vert;
			for (int sN = 0;sN < blocks[i].sidewalkContour.contour.size(); sN++) {
				int ind1 = sN;
				int ind2 = (sN+1) % blocks[i].sidewalkContour.contour.size();
				QVector3D dir = blocks[i].sidewalkContour[ind2] - blocks[i].sidewalkContour[ind1];
				float length = dir.length();
				dir /= length;
				//float z1 = rendManager.getTerrainHeight(blocks[i].sidewalkContour[ind1].x(), blocks[i].sidewalkContour[ind1].y());
				//float z2 = rendManager.getTerrainHeight(blocks[i].sidewalkContour[ind2].x(), blocks[i].sidewalkContour[ind2].y());

				QVector3D p1 = QVector3D(blocks[i].sidewalkContour[ind1].x(), blocks[i].sidewalkContour[ind1].y(), 0);
				QVector3D p2 = QVector3D(blocks[i].sidewalkContour[ind2].x(), blocks[i].sidewalkContour[ind2].y(), 0);
				QVector3D p3 = QVector3D(blocks[i].sidewalkContour[ind2].x(), blocks[i].sidewalkContour[ind2].y(), deltaZ);
				QVector3D p4 = QVector3D(blocks[i].sidewalkContour[ind1].x(), blocks[i].sidewalkContour[ind1].y(), deltaZ);
				QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
				vert.push_back(Vertex(p1, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p2, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p3, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p4, QColor(128, 128, 128), normal, QVector3D()));
			}
			rendManager.addStaticGeometry("3d_blocks", vert, "", GL_QUADS, 1|mode_Lighting|mode_AdaptTerrain);
		}

		// 公園の3Dモデルを生成
		if (blocks[i].isPark) {
			// PARK
			int randPark=qrand()%grassFileNames.size();
			rendManager.addStaticGeometry2("3d_parks", blocks[i].blockContour.contour, 0.0f, false, grassFileNames[randPark], GL_QUADS, 2|mode_AdaptTerrain, QVector3D(0.05f,0.05f,0.05f), QColor());

			// 側面
			std::vector<Vertex> vert;
			for (int sN = 0; sN < blocks[i].blockContour.contour.size(); sN++) {
				int ind1 = sN;
				int ind2 = (sN+1) % blocks[i].blockContour.contour.size();
				QVector3D dir = blocks[i].blockContour.contour[ind2] - blocks[i].blockContour.contour[ind1];
				float length = dir.length();
				dir /= length;
				//float z1 = rendManager.getTerrainHeight(blocks[i].blockContour[ind1].x(), blocks[i].blockContour[ind1].y());
				//float z2 = rendManager.getTerrainHeight(blocks[i].blockContour[ind2].x(), blocks[i].blockContour[ind2].y());

				//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
				QVector3D p1 = QVector3D(blocks[i].blockContour.contour[ind1].x(), blocks[i].blockContour.contour[ind1].y(), 0);
				QVector3D p2 = QVector3D(blocks[i].blockContour.contour[ind2].x(), blocks[i].blockContour.contour[ind2].y(), 0);
				QVector3D p3 = QVector3D(blocks[i].blockContour.contour[ind2].x(), blocks[i].blockContour.contour[ind2].y(), deltaZ);
				QVector3D p4 = QVector3D(blocks[i].blockContour.contour[ind1].x(), blocks[i].blockContour.contour[ind1].y(), deltaZ);
				QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
				vert.push_back(Vertex(p1, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p2, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p3, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p4, QColor(128, 128, 128), normal, QVector3D()));
			}
			rendManager.addStaticGeometry("3d_parks", vert, "", GL_QUADS, 1|mode_Lighting|mode_AdaptTerrain);
		}
	}
}

/**
 * Parcel情報から、その3Dモデルを生成する
 */
void BlockMeshGenerator::generateParcelMesh(VBORenderManager& rendManager, BlockSet& blocks) {
	rendManager.removeStaticGeometry("3d_parcels");

	const float deltaZ = 2.6f;

	for (int i = 0; i < blocks.size(); ++i) {
		Block::parcelGraphVertexIter vi, viEnd;
			
		int cnt = 0;
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi, ++cnt) {
			std::vector<Vertex> vert;
			QVector3D color;

			if (blocks[i].myParcels[*vi].parcelContour.isSelfIntersecting()) continue;

			// 上面のモデル
			int randPark=1;//qrand()%grassFileNames.size();
			rendManager.addStaticGeometry2("3d_parcels", blocks[i].myParcels[*vi].parcelContour.contour, deltaZ, false, grassFileNames[randPark], GL_QUADS, 2|mode_AdaptTerrain, QVector3D(0.05f,0.05f,0.05f), QColor());

			// 側面のモデル
			for(int sN=0;sN<blocks[i].myParcels[*vi].parcelContour.contour.size();sN++){
				int ind1 = sN;
				int ind2 = (sN+1) % blocks[i].myParcels[*vi].parcelContour.contour.size();
				QVector3D dir = blocks[i].myParcels[*vi].parcelContour.contour[ind2] - blocks[i].myParcels[*vi].parcelContour.contour[ind1];
				float length = dir.length();
				dir /= length;
				
				QVector3D p1(blocks[i].myParcels[*vi].parcelContour.contour[ind1].x(), blocks[i].myParcels[*vi].parcelContour.contour[ind1].y(), 0);
				QVector3D p2(blocks[i].myParcels[*vi].parcelContour.contour[ind2].x(), blocks[i].myParcels[*vi].parcelContour.contour[ind2].y(), 0);
				QVector3D p3(blocks[i].myParcels[*vi].parcelContour.contour[ind2].x(), blocks[i].myParcels[*vi].parcelContour.contour[ind2].y(), deltaZ);
				QVector3D p4(blocks[i].myParcels[*vi].parcelContour.contour[ind1].x(), blocks[i].myParcels[*vi].parcelContour.contour[ind1].y(), deltaZ);
				QVector3D normal = QVector3D::crossProduct(p2-p1,p4-p1).normalized();
				vert.push_back(Vertex(p1, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p4, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p3, QColor(128, 128, 128), normal, QVector3D()));
				vert.push_back(Vertex(p2, QColor(128, 128, 128), normal, QVector3D()));
			}
			rendManager.addStaticGeometry("3d_parcels", vert, "", GL_QUADS, 1|mode_Lighting|mode_AdaptTerrain);
		}
	}
}

void BlockMeshGenerator::generate2DParcelMesh(VBORenderManager& rendManager, BlockSet& blocks) {
	rendManager.removeStaticGeometry("3d_blocks");
	rendManager.removeStaticGeometry("3d_parks");

	const float deltaZ = 3.0f;

	QColor parkColor(0xca, 0xdf, 0xaa);
	for (int bN = 0; bN < blocks.size(); ++bN) {
		if (!blocks[bN].valid) continue;
		if (blocks[bN].isPark) {
			Loop3D parkC = blocks[bN].blockContour.contour;
			if (parkC.size() > 2) {
				parkC.push_back(parkC.front());
				rendManager.addStaticGeometry2("3d_parks", parkC, deltaZ, false, "", GL_QUADS, 1, QVector3D(), parkColor);
			}
		}/* else {
			Block::parcelGraphVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(blocks[bN].myParcels); vi != vend; ++vi) {
				if (blocks[bN].myParcels[*vi].isPark && blocks[bN].myParcels[*vi].parcelContour.contour.size() > 2) {
					rendManager.addStaticGeometry2("3d_parks", blocks[bN].myParcels[*vi].parcelContour.contour, deltaZ, false, "", GL_QUADS, 1, QVector3D(), parkColor);
				}
			}
		}*/
	}

	// Draw parcels as lines
	float parcelLine = G::global().getFloat("2d_parcelLine");
	if (parcelLine > 0) {
		glLineWidth(parcelLine);
		std::vector<Vertex> vert;
		for(int bN=0;bN<blocks.size();bN++){
			if (blocks[bN].isPark) continue;

			Block::parcelGraphVertexIter vi, viEnd;	
			for (boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi) {
				if (blocks[bN].myParcels[*vi].parcelContour.isSelfIntersecting()) continue;

				Polygon3D pol = blocks[bN].myParcels[*vi].parcelBuildableAreaContour;
				for (int i = 0; i < pol.contour.size(); ++i) {
					int next = (i+1) % pol.contour.size();
					vert.push_back(Vertex(pol.contour[i]+QVector3D(0,0,deltaZ), QColor(150, 150, 150), QVector3D(0,0,1), QVector3D()));
					vert.push_back(Vertex(pol.contour[next]+QVector3D(0,0,deltaZ), QColor(150, 150, 150), QVector3D(0,0,1), QVector3D()));
				}
			}
		}
		rendManager.addStaticGeometry("3d_blocks", vert, "", GL_LINES, 1);
	}
}
