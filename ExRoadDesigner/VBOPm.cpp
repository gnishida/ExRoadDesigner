/************************************************************************************************
 *		Procedural City Generation
 *		@author igarciad
 ************************************************************************************************/

#include "VBOPm.h"
#include "Polygon3D.h"

#include <qdir.h>
#include <QStringList>
#include <QTime>

#include "VBOPmBlocks.h"
#include "VBOPmParcels.h"
#include "VBOPmBuildings.h"

#include "VBOGeoBuilding.h"

#include "VBOVegetation.h"

///////////////////////////////////////////////////////////////
// INIT
///////////////////////////////////////////////////////////////
bool VBOPm::initialized=false;
PlaceTypesMainClass VBOPm::placeTypes;
// LC
bool VBOPm::initializedLC=false;
static std::vector<QString> sideWalkFileNames;
static std::vector<QVector3D> sideWalkScale;
static std::vector<QString> grassFileNames;

void VBOPm::init(){
	/////////////////////////////////////////
	// INIT PLACETYPE
	std::vector<PlaceType> *placeTypesPtr = 
		&(placeTypes.myPlaceTypes);

	placeTypesPtr->clear();
	PlaceType tmpPlaceType;
	//----- place type category ----
	tmpPlaceType["pt_category"]= 0;
	//----- roads -----
	tmpPlaceType["pt_radius"] = 600.0f;
	tmpPlaceType["pt_radius2"] = 600.0f;
	tmpPlaceType["pt_edges_curvature"] = 0;
	tmpPlaceType["pt_edges_irregularity"] =	0;
	tmpPlaceType["pt_edges_lengthU"] =	350.0f;
	tmpPlaceType["pt_edges_lengthV"] = 200.0f;
	tmpPlaceType["pt_edges_width"] =			20.0f;//!!!!!! UPDATE LC::misctools::Global::global()->arterial_edges_width;
	tmpPlaceType["pt_num_departing"] =	4;
	tmpPlaceType["pt_orientation"] = 0;
	tmpPlaceType["pt_loc_edges_curvature"] = 0;
	tmpPlaceType["pt_loc_edges_irregularity"] =	0;
	tmpPlaceType["pt_loc_edges_lengthU"] =		0.01f*50;
	tmpPlaceType["pt_loc_edges_lengthV"] =	0.01f*50;
	tmpPlaceType["pt_cur_edges_count"] = 0;
	//----- parcels -----
	tmpPlaceType["pt_parcel_area_mean"] = 5000.0f;//3600;
	tmpPlaceType["pt_parcel_area_deviation"] =	49;
	tmpPlaceType["pt_parcel_split_deviation"] =	0.19;
	tmpPlaceType["pt_park_percentage"] =0.0f;
	//----- buildings -----
	tmpPlaceType["pt_parcel_setback_front"] =15.0f;
	tmpPlaceType["pt_parcel_setback_sides"] = 2.0f;
	tmpPlaceType["pt_building_height_mean"] = 12;
	tmpPlaceType["pt_building_height_deviation"] =	90;
	tmpPlaceType["pt_building_max_frontage"] =0;
	tmpPlaceType["pt_parcel_setback_rear"] =0;
	tmpPlaceType["pt_building_max_depth"] =0;
	//-------------------
	tmpPlaceType["pt_pt"]= QVector3D(   0.0f,    0.0f, 0.0f);
	placeTypesPtr->push_back(tmpPlaceType);
	G::global()["num_place_types"]=1;
	printf("-->Initialized placetypes\n");
	initialized=true;
}//

void VBOPm::initLC(){
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
	initializedLC=true;
}


///////////////////////////////////////////////////////////////
// GENERATE 3D GEOMETRY
///////////////////////////////////////////////////////////////

bool VBOPm::generateGeometry(VBORenderManager& rendManager,RoadGraph &roadGraph){
	//////////////////////////////////////////////
	// INIT
	if (!initialized) {
		init();//init placetypes
	}
	if (!initializedLC) {
		initLC();//init LC textures
	}
	QTime timer;

	//////////////////////////////////////////////
	// 1. BLOCKS
	std::vector< Block > blocks;
	if(VBOPmBlocks::generateBlocks(placeTypes,roadGraph,blocks)==false){
		printf("ERROR: generateBlocks\n");
		return false;
	}
	printf(">>Num Blocks %d\n",blocks.size());
	
	//////////////////////////////////////////////
	// 2. BLOCKS Geometry
	//float offsetSidewalks = +3.5f; 
	//float offsetArterials =-3.5f;//-((LC::misctools::Global::global()->cuda_arterial_numLanes+1)*LC::misctools::Global::global()->roadLaneWidth);	
	rendManager.removeStaticGeometry("3d_sidewalk");
	
	/*{//test blocks
		for(int bN=0;bN<blocks.size();bN++){
			std::vector<Vertex> vert;
			QVector3D color((float)qrand()/RAND_MAX,(float)qrand()/RAND_MAX,(float)qrand()/RAND_MAX);
			qDebug()<<color;
			qDebug()<<blocks[bN].blockContour.contour.size();
			for(int sN=0;sN<blocks[bN].blockContour.contour.size();sN++){
				vert.push_back(Vertex(QVector3D(blocks[bN].blockContour.contour[sN].x(),blocks[bN].blockContour.contour[sN].y(),5+bN),
					color,QVector3D(),QVector3D()));
			}
			rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_POINTS,1|mode_AdaptTerrain);

		}
		return true;

	}*/

	for(int bN=0;bN<blocks.size();bN++){		
		if(blocks[bN].isPark==true){
			// PARK
			int randPark=qrand()%grassFileNames.size();
			rendManager.addStaticGeometry2("3d_sidewalk",blocks[bN].blockContour.contour,0.5f,false,grassFileNames[randPark],GL_QUADS,2|mode_AdaptTerrain,QVector3D(0.05f,0.05f,0.05f),QVector3D());

		}else{
			// NORMAL
			int randSidewalk=qrand()%sideWalkFileNames.size();

			rendManager.addStaticGeometry2("3d_sidewalk",blocks[bN].blockContour.contour,1.5f,false,sideWalkFileNames[randSidewalk],GL_QUADS,2|mode_AdaptTerrain,sideWalkScale[randSidewalk],QVector3D());
			//sides
			std::vector<Vertex> vert;
			for(int sN=0;sN<blocks[bN].blockContour.contour.size();sN++){
				int ind1=sN;
				int ind2=(sN+1)%blocks[bN].blockContour.contour.size();
				QVector3D dir=blocks[bN].blockContour.contour[ind2]-blocks[bN].blockContour.contour[ind1];
				float length=dir.length();
				dir/=length;
				//printf("z %f\n",blocks[bN].blockContour.contour[ind1].z());
				QVector3D p1=blocks[bN].blockContour.contour[ind1]+QVector3D(0,0, 1.0f);
				QVector3D p2=blocks[bN].blockContour.contour[ind2]+QVector3D(0,0, 1.0f);
				QVector3D p3=blocks[bN].blockContour.contour[ind2]+QVector3D(0,0, 1.5f);
				QVector3D p4=blocks[bN].blockContour.contour[ind1]+QVector3D(0,0, 1.5f);
				QVector3D normal=QVector3D::crossProduct(p2-p1,p4-p1).normalized();
				vert.push_back(Vertex(p1,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				vert.push_back(Vertex(p2,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				vert.push_back(Vertex(p3,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
				vert.push_back(Vertex(p4,QVector3D(0.5f,0.5f,0.5f),normal,QVector3D()));
			}
			rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_QUADS,1|mode_Lighting|mode_AdaptTerrain);
		}
	}

	//////////////////////////////////////////////
	// 3. PARCELS
	if(VBOPmParcels::generateParcels(placeTypes,blocks)==false){
		printf("ERROR: generateParcels\n");
		return false;
	}
	printf(">>Parcels were generated.\n");
	
	//////////////////////////////////////////////
	// 4. BUILDINGS
	if(VBOPmBuildings::generateBuildings(placeTypes,blocks)==false){
		printf("ERROR: generateBuildings\n");
		return false;
	}
	printf(">>Buildings contours were generated.\n");
	
	//////////////////////////////////////////////
	// 5. BUILDINGS Geometry
	rendManager.removeStaticGeometry("3d_building");
	rendManager.removeStaticGeometry("3d_building_fac");
		
	timer.start();
	int type = 1;
	Block::parcelGraphVertexIter vi, viEnd;
	for(int bN=0;bN<blocks.size();bN++){
		if(blocks[bN].isPark==true) continue;//skip those with parks
		for(boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi) {
			VBOGeoBuilding::generateBuilding(rendManager,blocks[bN].myParcels[*vi].myBuilding,type);
		}
	}
	printf("Building generation: %d ms\n",timer.elapsed());

	//////////////////////////////////////////////
	// 6. VEGETATION
	timer.restart();
	VBOVegetation::generateVegetation(rendManager,placeTypes,blocks);
	printf("Vegetation generation: %d ms\n",timer.elapsed());

	return true;
}

///////////////////////////////////////////////////////////////
// GENERATE 2D GEOMTRY
///////////////////////////////////////////////////////////////

bool VBOPm::generate2DGeometry(VBORenderManager& rendManager,RoadGraph &roadGraph){
	//////////////////////////////////////////////
	// INIT
	//if(initialized==false){
		init();//init placetypes
	//}
	rendManager.removeStaticGeometry("2d_blocks");
	rendManager.removeStaticGeometry("2d_parks");

	QTime timer;
	timer.start();
	//////////////////////////////////////////////
	// 1. Generate geometry
	// blocks
	std::vector< Block > blocks;
	if(VBOPmBlocks::generateBlocks(placeTypes,roadGraph,blocks)==false){
		printf("\nERROR: generateBlocks\n\n");
		return false;
	}
	printf(">>Num Blocks %d\n",blocks.size());
	
	//parcels
	if(VBOPmParcels::generateParcels(placeTypes,blocks)==false){
		printf("\nERROR: generateParcels\n\n");
		return false;
	}
	printf(">>Parcels were generated.\n");


	//buildings (for buildable area)
	if(VBOPmBuildings::generateBuildings(placeTypes,blocks)==false){
		printf("\nERROR: generateBuildings\n\n");
		return false;
	}
	printf(">>Buildings contours were generated.\n");

	/////////////////////////////
	// 2. Render: PARKS AND PARCEL LINES
	/*// park blocks (same code in VBOVegetation)
	qsrand(blocks.size());
	float park_percentage=G::global().getInt("2d_parkPer")*0.01f;//tmpPlaceType["pt_park_percentage"]
	int numBlockParks=park_percentage*blocks.size();
	QSet<int> blockWithPark;
	while(blockWithPark.size()<numBlockParks){
		blockWithPark.insert(qrand()%blocks.size());
	}

	// create green blocks
	QSetIterator<int> i(blockWithPark);
	
	while (i.hasNext()){
		int bN=i.next();*/
	QVector3D parkColor(0xca/255.0f,0xdf/255.0f,0xaa/255.0f);//cadfaa
	for(int bN=0;bN<blocks.size();bN++){
		if(blocks[bN].isPark==false) continue;//skip those without parks
			Loop3D parkC=blocks[bN].blockContour.contour;
			if(parkC.size()>2){
				parkC.push_back(parkC.front());
				rendManager.addStaticGeometry2("2d_parks",parkC,0.35f,false,"",GL_QUADS,1|mode_AdaptTerrain,QVector3D(),parkColor);
			}
			//rendManager.addStaticGeometry2("2d_parks",blocks[bN].blockContour.contour,0.2f,false,"",GL_QUADS,1|mode_AdaptTerrain,QVector3D(),parkColor);
		
	}

	// Draw parcels as lines
	float parcelLine=G::global().getFloat("2d_parcelLine");
	if(parcelLine>0){
		glLineWidth(parcelLine);
		std::vector<Vertex> vert;
		for(int bN=0;bN<blocks.size();bN++){
			if(blocks[bN].isPark==true) continue;//skip those with parks
			//printf("bN[%d]\n",bN);
			Block::parcelGraphVertexIter vi, viEnd;	
			for(boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels); vi != viEnd; ++vi){	
				//Polygon3D& pol=blocks[bN].myParcels[*vi].parcelContour;//.parcelBuildableAreaContour;
				Polygon3D& pol=blocks[bN].myParcels[*vi].parcelBuildableAreaContour;
				//printf("pol-> %d\n",pol.contour.size());
				for(int i=0;i<pol.contour.size();i++){
					int next=(i+1)%pol.contour.size();//close
					vert.push_back(Vertex(pol.contour[i]+QVector3D(0,0,0.3f),QVector3D(0.6f,0.6f,0.6f),QVector3D(0,0,1),QVector3D()));
					vert.push_back(Vertex(pol.contour[next]+QVector3D(0,0,0.3f),QVector3D(0.6f,0.6f,0.6f),QVector3D(0,0,1),QVector3D()));
				}
			}


		}
		rendManager.addStaticGeometry("2d_blocks",vert,"",GL_LINES,1|mode_AdaptTerrain);
		printf(">> generate2DGeometry lines %f vert %d in %d ms\n",parcelLine,vert.size(),timer.elapsed());
	}
	
	
	return true;
}//