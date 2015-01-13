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
	if(initialized==false){
		init();//init placetypes
	}
	if(initializedLC==false){//&&G::global().getInt("3d_render_mode")==0){
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
		
		//////////////////
		// COLORFULL: Render LC --> 3d_render_mode==1
		if(G::global().getInt("3d_render_mode")==1){
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
		//////////////////
		// HATCHING: Render hash --> 3d_render_mode==2
		if(G::global().getInt("3d_render_mode")==2){
			//printf("hatch\n");
			float blackWidth=0.3f;
			float randC=(0.3f*qrand()/RAND_MAX)-0.15f;
			QVector3D colorW(0.5+randC,0.5+randC,0.5+randC);
			QVector3D colorB(0.0,0.0,0.0);
			Polygon3D blockRed;
			blocks[bN].blockContour.computeInset(1.0f,blockRed.contour,false);
	
			rendManager.addStaticGeometry2("3d_sidewalk",blockRed.contour,1.5f,false,"hatching_array",GL_QUADS,10|mode_TexArray|mode_AdaptTerrain,QVector3D(1,1,1),colorW);
			rendManager.addStaticGeometry2("3d_sidewalk",blocks[bN].blockContour.contour,1.5f-blackWidth/2.0f,false,"hatching_array",GL_QUADS,10|mode_TexArray|mode_AdaptTerrain,QVector3D(1,1,1),colorB);

			//sides
			std::vector<Vertex> vert;
			float h0 []={1.00f,1.15f,1.35f};
			float h1 []={1.15f,1.35f,1.50f};
			QVector3D cA[]={colorB,colorW,colorB};
			for(int i=0;i<3;i++){
				for(int sN=0;sN<blocks[bN].blockContour.contour.size();sN++){
					int ind1=sN;
					int ind2=(sN+1)%blocks[bN].blockContour.contour.size();
					QVector3D dir=blocks[bN].blockContour.contour[ind2]-blocks[bN].blockContour.contour[ind1];
					float length=dir.length();
					dir/=length;
					QVector3D p1=blocks[bN].blockContour.contour[ind1]+QVector3D(0,0, h0[i]);
					QVector3D p2=blocks[bN].blockContour.contour[ind2]+QVector3D(0,0, h0[i]);
					QVector3D p3=blocks[bN].blockContour.contour[ind2]+QVector3D(0,0, h1[i]);
					QVector3D p4=blocks[bN].blockContour.contour[ind1]+QVector3D(0,0, h1[i]);
					QVector3D normal=QVector3D::crossProduct(p2-p1,p4-p1).normalized();
					vert.push_back(Vertex(p1,cA[i],normal,QVector3D()));
					vert.push_back(Vertex(p2,cA[i],normal,QVector3D()));
					vert.push_back(Vertex(p3,cA[i],normal,QVector3D()));
					vert.push_back(Vertex(p4,cA[i],normal,QVector3D()));
				}
			}
			rendManager.addStaticGeometry("3d_sidewalk",vert,"hatching_array",GL_QUADS,10|mode_TexArray|mode_Lighting|mode_AdaptTerrain);
		}
	}
	//return true;//!!!!!!!!!!!!!
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
	int type=0;
	// COLORFULL: Render LC --> 3d_render_mode==1
	if(G::global().getInt("3d_render_mode")==1)
		type=1;

	// HATCHING: Render hash --> 3d_render_mode==2
	if(G::global().getInt("3d_render_mode")==2)
		type=3;	

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
	// COLORFULL: Render LC --> 3d_render_mode==1
	if(G::global().getInt("3d_render_mode")==1){
		VBOVegetation::generateVegetation(rendManager,placeTypes,blocks);
	}

	// HATCHING: Render hash --> 3d_render_mode==2
	if(G::global().getInt("3d_render_mode")==2){
		// 6.1 create one tree
		std::vector<Vertex> vert;
		// OAK
		{
			const int numSides=8;
			const int numHeigh=8;
			QVector3D colorTrunk(0.01,0.4,0.4);
			const float trunkRad=1.6f;
			const float trunkHei=2.5f;

			const float deltaY=25.0f/numHeigh;
			float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
			float angle;
			float accY=0;
			float accY1=0;
			QVector3D p0,p1,p2,p3;
			for(int y=0;y<numHeigh+1;y++){
				float accX1=0;
				float accX2=0;
				float radi1,radi2;
				if(y==0){
					radi1=trunkRad*1.2f;
					radi2=trunkRad*0.9f;
				}else{
					float x=accY-trunkHei;
					radi1=std::max(0.001f,-0.0004406435f*x*x*x*x + 0.0242577163f*x*x*x - 0.4816392816f*x*x + 3.6703204091f*x + 2.0f);//0.0003f*pow(x,4) + 0.019f*pow(x,3) - 0.3842f*x*x + 2.9968f* x + 3.286f; //0.001-->normal
					x=accY+deltaY-trunkHei;
					radi2=std::max(0.001f,-0.0004406435f*x*x*x*x + 0.0242577163f*x*x*x - 0.4816392816f*x*x + 3.6703204091f*x + 2.0f);//0.0003f*pow(x,4) + 0.019f*pow(x,3) - 0.3842f*x*x + 2.9968f* x + 3.286f;
				}
				//printf("rad %f %f\n",radi1,radi2);
				float yLen;
				for(int i=0;i<numSides;i++){
					angle=deltaAngle*i;
					if(i==0){//to calculate x vert once per height
						p0=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),accY );
						p3=QVector3D( radi2 * cos( angle ), radi2 * sin( angle ),accY+deltaY );
					}
					angle=deltaAngle*((i+1)%numSides);
					p1=QVector3D ( radi1 * cos( angle ), radi1 * sin( angle ),accY );
					p2=QVector3D( radi2 * cos( angle ), radi2 * sin( angle ),accY+deltaY );

					float xLen1=(p1-p0).length();
					float xLen2=(p2-p3).length();	
					yLen=(p3-p0).length();

					vert.push_back(Vertex(p0,colorTrunk,QVector3D(p0.x(),p0.y(),0).normalized(),QVector3D(accX1,accY1,0)));
					vert.push_back(Vertex(p1,colorTrunk,QVector3D(p1.x(),p1.y(),0).normalized(),QVector3D(accX1+xLen1,accY1,0)));
					vert.push_back(Vertex(p2,colorTrunk,QVector3D(p2.x(),p2.y(),0).normalized(),QVector3D(accX2+xLen2,accY1+yLen,0)));
					vert.push_back(Vertex(p3,colorTrunk,QVector3D(p3.x(),p3.y(),0).normalized(),QVector3D(accX2,accY1+yLen,0)));

					p0=p1;
					p3=p2;
					accX1+=xLen1;
					accX2+=xLen2;
				}
				if(y==0)
					accY+=trunkHei;
				else
					accY+=deltaY;
				accY1+=yLen;
			}
		}
		// 6.1 create other tree
		std::vector<Vertex> vertFir;
		//FIR
		{
			int numSides=5;
			QVector3D colorTrunk(0.01,0.4,0.4);
			//for each heigh difference (deltaYA), we set the radi1 (bottom) and radi2 (top)
			float deltaYA[]={0.06f,0.109f,0.238f,0.226f,0.372f};
			int numHeigh=5;
			float rad1A[]={0.19f,0.12f,0.45f,0.39f,0.32f};//{0.217f,0.125f,0.45f,0.39f,0.32f};
			float rad2A[]={0.12f,0.12f,0.31f,0.23f,0.001f};//{0.125f,0.125f,0.31f,0.23f,0.00f};
			float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;

			float angle,deltaY;
			float accY=0;
			float accY1=0;
			QVector3D p0,p1,p2,p3;
			for(int y=0;y<numHeigh;y++){
				float accX1=0;
				float accX2=0;
				float radi1,radi2;

				radi1=rad1A[y];
				radi2=rad2A[y];
				deltaY=deltaYA[y];
				//printf("rad %f radi2 %f accY %f +d %f\n",radi1,radi2,accY,accY+deltaY);
				float yLen;
				for(int i=0;i<numSides;i++){
					angle=deltaAngle*i;
					if(i==0){//to calculate x vertFir once per height
						p0=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),accY );
						p3=QVector3D( radi2 * cos( angle ), radi2 * sin( angle ),accY+deltaY );//0.1 for overlap
					}
					angle=deltaAngle*((i+1)%numSides);
					p1=QVector3D ( radi1 * cos( angle ), radi1 * sin( angle ),accY );
					p2=QVector3D( radi2 * cos( angle ), radi2 * sin( angle ),accY+deltaY );

					float xLen1=(p1-p0).length();
					float xLen2=(p2-p3).length();	
					yLen=(p3-p0).length();

					vertFir.push_back(Vertex(p0,colorTrunk,QVector3D(p0.x(),p0.y(),0).normalized(),QVector3D(accX1,accY1,0)));
					vertFir.push_back(Vertex(p1,colorTrunk,QVector3D(p1.x(),p1.y(),0).normalized(),QVector3D(accX1+xLen1,accY1,0)));
					vertFir.push_back(Vertex(p2,colorTrunk,QVector3D(p2.x(),p2.y(),0).normalized(),QVector3D(accX2+xLen2,accY1+yLen,0)));
					vertFir.push_back(Vertex(p3,colorTrunk,QVector3D(p3.x(),p3.y(),0).normalized(),QVector3D(accX2,accY1+yLen,0)));

					p0=p1;
					p3=p2;
					accX1+=xLen1;
					accX2+=xLen2;
				}
				accY+=deltaY;
				accY1+=yLen;
			}

		}

		////////////////////////////////////////
		// 6.2 create tree positions
		int numTotalVert=0;
		float distanceBetweenTrees = 25.0f; //used for trees along streets
		std::vector<QVector3D> treePos;
		std::vector<QVector2D> treeScale;
		std::vector<int> treeType;

		QVector3D ptThis, ptNext,pos;
		QVector3D segmentVector;
		float segmentLength,height,rad;
		for (int i = 0; i < blocks.size(); ++i) {
			srand(blocks[i].randSeed);
			Loop3D blockContourInset;
			blocks[i].blockContour.computeInset(3.0f,blockContourInset);
			for (int j = 0; j < blockContourInset.size(); ++j) {
				ptThis = blockContourInset[j];
				ptNext = blockContourInset[(j+1)%blockContourInset.size()];
				segmentVector = ptNext - ptThis;
				segmentLength = segmentVector.length();
				segmentVector.normalize();

				float distFromSegmentStart = 0.0f;

				while (true) {
					distFromSegmentStart += distanceBetweenTrees * (0.4f*qrand()/RAND_MAX +0.8f);//Util::genRand(0.80f, 1.20f);
					if (distFromSegmentStart > segmentLength) 
						break;
					pos = ptThis + segmentVector*distFromSegmentStart;
					treePos.push_back(pos);

					int type=qrand()%2;//1
					treeType.push_back(type);

					if(type==0){//OAK
						// 15 avg tree * rand /our tree heigh
						height = 15.0f*((0.4f*qrand())/RAND_MAX +0.8f)/26.0f;//Util::genRand(0.80f, 1.20f);
						// height * rand rad to heigh ration / our tree rad
						rad    = 26.0f*height * (0.2f*qrand()/RAND_MAX +0.3f)/10.0f;//Util::genRand(0.30f, 0.50f);
						numTotalVert+=vert.size();
					}
					if(type==1){//FIR
						// 15 avg tree * rand /our tree heigh
						height = 15.0f*((0.4f*qrand())/RAND_MAX +0.8f);//Util::genRand(0.80f, 1.20f);
						// height * rand rad to heigh ration / our tree rad
						rad  = height * (0.2f*qrand()/RAND_MAX +0.85f);//Util::genRand(0.65f, 0.85f);
						numTotalVert+=vertFir.size();
					}
					treeScale.push_back(QVector2D(rad,height));		
				}
			}
		}
		// 6.3 create vertices
		std::vector<Vertex> vertTrees;

		//treePos.push_back(QVector3D());
		//treePos.push_back(QVector3D(50,50,0));
		vertTrees.resize(numTotalVert);
		QVector3D trans,vet;QVector2D tex;
		int vertNum=0;
		for(int tN=0;tN<treePos.size();tN++){
			trans=treePos[tN]+QVector3D(0,0,1.5f);
			QVector3D scale3D(treeScale[tN].x(),treeScale[tN].x(),treeScale[tN].y());
			QVector2D scale2D(treeScale[tN].x(),treeScale[tN].y());
			std::vector<Vertex>* vertA;
			if(treeType[tN]==0)
				vertA=&vert;
			if(treeType[tN]==1)
				vertA=&vertFir;
			for(int vN=0;vN<vertA->size();vN++){
				vet=QVector3D((*vertA)[vN][0],(*vertA)[vN][1],(*vertA)[vN][2]);
				tex=QVector2D((*vertA)[vN][9],(*vertA)[vN][10]);
				vet=vet*scale3D+trans;
				tex=tex*scale2D;
				vertTrees[vertNum]=(*vertA)[vN];
				vertTrees[vertNum][0]=vet.x();
				vertTrees[vertNum][1]=vet.y();
				vertTrees[vertNum][2]=vet.z();
				vertTrees[vertNum][9]=tex.x();
				vertTrees[vertNum][10]=tex.y();
				vertNum++;
			}

		}

		rendManager.addStaticGeometry("3d_trees",vertTrees,"hatching_array",GL_QUADS,10|mode_TexArray|mode_Lighting|mode_AdaptTerrain);
		printf("VEG generation in %d ms: numTree %d\n",timer.elapsed(),treePos.size());
	}
	return true;
}//

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