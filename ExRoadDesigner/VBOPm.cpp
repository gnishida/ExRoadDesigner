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

///////////////////////////////////////////////////////////////
// INIT
///////////////////////////////////////////////////////////////
bool VBOPm::initialized=false;

PlaceTypesMainClass VBOPm::placeTypes;

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
	tmpPlaceType["pt_parcel_area_mean"] = 3600;
	tmpPlaceType["pt_parcel_area_deviation"] =	49;
	tmpPlaceType["pt_parcel_split_deviation"] =	0.19;
	tmpPlaceType["pt_park_percentage"] =0.2f;
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

///////////////////////////////////////////////////////////////
// GENERATE
///////////////////////////////////////////////////////////////

bool VBOPm::generateGeometry(VBORenderManager& rendManager,RoadGraph &roadGraph){
	//////////////////////////////////////////////
	// INIT
	if(initialized==false){
		init();//init placetypes
	}
	QTime timer;
	//////////////////////////////////////////////
	// 1. BLOCKS
	std::vector< Block > blocks;
	if(VBOPmBlocks::generateBlocks(placeTypes,roadGraph,blocks)==false){
		printf("ERROR: generateBlocks\n");
		//return false;
	}
	
	printf(">>Num Blocks %d\n",blocks.size());
	
	//////////////////////////////////////////////
	// 2. BLOCKS Geometry
	//float offsetSidewalks = +3.5f; 
	//float offsetArterials =-3.5f;//-((LC::misctools::Global::global()->cuda_arterial_numLanes+1)*LC::misctools::Global::global()->roadLaneWidth);	
	rendManager.removeStaticGeometry("3d_sidewalk");

	for(int bN=0;bN<blocks.size();bN++){
		int randSidewalk=qrand()%VBOPmBlocks::sideWalkFileNames.size();
		//////////////////
		// Render with color --> 3d_render_mode==0
		if(G::global().getInt("3d_render_mode")==0){
			rendManager.addStaticGeometry2("3d_sidewalk",blocks[bN].blockContour.contour,1.5f,false,VBOPmBlocks::sideWalkFileNames[randSidewalk],GL_QUADS,2,VBOPmBlocks::sideWalkScale[randSidewalk],QVector3D());
			//sides
			std::vector<Vertex> vert;
			for(int sN=0;sN<blocks[bN].blockContour.contour.size();sN++){
				int ind1=sN;
				int ind2=(sN+1)%blocks[bN].blockContour.contour.size();
				QVector3D dir=blocks[bN].blockContour.contour[ind2]-blocks[bN].blockContour.contour[ind1];
				float length=dir.length();
				dir/=length;
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
			rendManager.addStaticGeometry("3d_sidewalk",vert,"",GL_QUADS,1|mode_Lighting);
		}
		//////////////////
		// Render hash --> 3d_render_mode==1
		if(G::global().getInt("3d_render_mode")==1){
			//printf("hatch\n");
			float blackWidth=0.3f;
			float randC=(0.3f*qrand()/RAND_MAX)-0.15f;
			QVector3D colorW(0.5+randC,0.5+randC,0.5+randC);
			QVector3D colorB(0.0,0.0,0.0);
			Polygon3D blockRed;
			blocks[bN].blockContour.computeInset(1.0f,blockRed.contour,false);
	
			rendManager.addStaticGeometry2("3d_sidewalk",blockRed.contour,1.5f,false,"hatching_array",GL_QUADS,10|mode_TexArray,QVector3D(1,1,1),colorW);
			rendManager.addStaticGeometry2("3d_sidewalk",blocks[bN].blockContour.contour,1.5f-blackWidth/2.0f,false,"hatching_array",GL_QUADS,10|mode_TexArray,QVector3D(1,1,1),colorB);

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
			rendManager.addStaticGeometry("3d_sidewalk",vert,"hatching_array",GL_QUADS,10|mode_TexArray|mode_Lighting);
		}
	}
	
	//////////////////////////////////////////////
	// 3. PARCELS
	if(VBOPmParcels::generateParcels(placeTypes,blocks)==false){
		printf("ERROR: generateParcels\n");
		//return false;
	}
	//////////////////////////////////////////////
	// 4. BUILDINGS
	if(VBOPmBuildings::generateBuildings(placeTypes,blocks)==false){
		printf("ERROR: generateBuildings\n");
		//return false;
	}
	
	//////////////////////////////////////////////
	// 5. BUILDINGS Geometry
	rendManager.removeStaticGeometry("3d_building");
	rendManager.removeStaticGeometry("3d_building_fac");

	
	timer.start();
	Block::parcelGraphVertexIter vi, viEnd;
	for(int bN=0;bN<blocks.size();bN++){
		//blocks[bN]
		for(boost::tie(vi, viEnd) = boost::vertices(blocks[bN].myParcels);
			vi != viEnd; ++vi)
		{
			//blocks[bN].myParcels[*vi].parcelContour
			VBOGeoBuilding::generateBuilding(rendManager,blocks[bN].myParcels[*vi].myBuilding);//.buildingFootprint,blocks[bN].myParcels[*vi].parcelBuildingAttributes.att_stories);
		}
	}
	printf("Building generation: %d ms\n",timer.elapsed());
	//////////////////////////////////////////////
	// 6. VEGETATION
	timer.restart();
	// 6.1 create one tree
	
	std::vector<Vertex> vert;
	// OAK
	{
		int numSides=8;
		int numHeigh=8;
		QVector3D colorTrunk(0.01,0.4,0.4);
		float trunkRad=1.6f;
		float trunkHei=2.5f;

		float deltaY=25.0f/numHeigh;
		float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
		float angle;
		float accY=0;
		float accY1=0;
		QVector3D p0,p1,p2,p3;
		for(int y=0;y<numHeigh+1;y++){
			float accX1=0;
			float accX2=0;
			float rad1,rad2;
			if(y==0){
				rad1=trunkRad*1.2f;
				rad2=trunkRad*0.9f;
			}else{
				float x=accY-trunkHei;
				rad1=std::max(0.0f,-0.0004406435f*x*x*x*x + 0.0242577163f*x*x*x - 0.4816392816f*x*x + 3.6703204091f*x + 2.0f);//0.0003f*pow(x,4) + 0.019f*pow(x,3) - 0.3842f*x*x + 2.9968f* x + 3.286f;
				x=accY+deltaY-trunkHei;
				rad2=std::max(0.0f,-0.0004406435f*x*x*x*x + 0.0242577163f*x*x*x - 0.4816392816f*x*x + 3.6703204091f*x + 2.0f);//0.0003f*pow(x,4) + 0.019f*pow(x,3) - 0.3842f*x*x + 2.9968f* x + 3.286f;
			}
			float yLen;
			for(int i=0;i<numSides;i++){
				angle=deltaAngle*i;
				if(i==0){//to calculate x vert once per height
					p0=QVector3D( rad1 * cos( angle ), rad1 * sin( angle ),accY );
					p3=QVector3D( rad2 * cos( angle ), rad2 * sin( angle ),accY+deltaY );
				}
				angle=deltaAngle*((i+1)%numSides);
				p1=QVector3D ( rad1 * cos( angle ), rad1 * sin( angle ),accY );
				p2=QVector3D( rad2 * cos( angle ), rad2 * sin( angle ),accY+deltaY );

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
		//for each heigh difference (deltaYA), we set the rad1 (bottom) and rad2 (top)
		float deltaYA[]={0.06f,0.109f,0.238f,0.226f,0.372f};
		int numHeigh=5;
		float rad1A[]={0.19f,0.12f,0.45f,0.39f,0.32f};//{0.217f,0.125f,0.45f,0.39f,0.32f};
		float rad2A[]={0.12f,0.12f,0.31f,0.23f,0.00f};//{0.125f,0.125f,0.31f,0.23f,0.00f};
		float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;

		float angle,deltaY;
		float accY=0;
		float accY1=0;
		QVector3D p0,p1,p2,p3;
		for(int y=0;y<numHeigh;y++){
			float accX1=0;
			float accX2=0;
			float rad1,rad2;

			rad1=rad1A[y];
			rad2=rad2A[y];
			deltaY=deltaYA[y];
			//printf("rad %f rad2 %f accY %f +d %f\n",rad1,rad2,accY,accY+deltaY);
			float yLen;
			for(int i=0;i<numSides;i++){
				angle=deltaAngle*i;
				if(i==0){//to calculate x vertFir once per height
					p0=QVector3D( rad1 * cos( angle ), rad1 * sin( angle ),accY );
					p3=QVector3D( rad2 * cos( angle ), rad2 * sin( angle ),accY+deltaY );//0.1 for overlap
				}
				angle=deltaAngle*((i+1)%numSides);
				p1=QVector3D ( rad1 * cos( angle ), rad1 * sin( angle ),accY );
				p2=QVector3D( rad2 * cos( angle ), rad2 * sin( angle ),accY+deltaY );

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
	
	rendManager.addStaticGeometry("3d_trees",vertTrees,"hatching_array",GL_QUADS,10|mode_TexArray|mode_Lighting);
	printf("VEG generation in %d ms: numTree %d\n",timer.elapsed(),treePos.size());
	return true;
}//

