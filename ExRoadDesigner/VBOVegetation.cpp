#include "VBOVegetation.h"

#include "VBORenderManager.h"
#include "BBox.h"


	VBOVegetation::VBOVegetation(void){
	}


	VBOVegetation::~VBOVegetation(void){
	}

	ModelSpec addTree(QVector3D pos){
		ModelSpec tree;

		tree.transMatrix.setToIdentity();
		tree.transMatrix.translate(pos);
		tree.transMatrix.rotate(pos.x()*pos.y(),0.0f,0.0f,1.0f);//random
		tree.transMatrix.scale(1.0f+(((0.5f*qrand())/RAND_MAX))-0.5f);
		tree.colors.resize(2);
		//trunk
		tree.colors[1]=QVector3D(0.27f,0.22f,0.22f);
		tree.colors[1]=tree.colors[1]+QVector3D(1.0,1.0,1.0f)*((0.2f*qrand())/RAND_MAX-0.1f);
		//leaves
		int treeColor=qrand()%100;
		int desvLeaf=qrand()%40;
		if(treeColor>=0&&treeColor<=46)
			tree.colors[0]=QVector3D(115-desvLeaf,169-desvLeaf,102-desvLeaf);//green to dark 
		if(treeColor>=47&&treeColor<=92)
			tree.colors[0]=QVector3D(69-desvLeaf/2.0f,109-desvLeaf/2.0f,72-desvLeaf/2.0f);//green to dark 
		if(treeColor>=93&&treeColor<=94)
			tree.colors[0]=QVector3D(155-desvLeaf/3.0f,124-desvLeaf/3.0f,24-desvLeaf/3.0f);//yellow to dark 

		if(treeColor>=95&&treeColor<=96)
			tree.colors[0]=QVector3D(96-desvLeaf/4.0f,25-desvLeaf/4.0f,33-desvLeaf/4.0f);//red to dark 

		if(treeColor>=97&&treeColor<=100)
			tree.colors[0]=QVector3D(97-desvLeaf/2.0f,69-desvLeaf/2.0f,58-desvLeaf/2.0f);//grey to dark 
		tree.colors[0]/=255.0f;
		tree.type=0;

		return tree;
	}

	ModelSpec addStreetLap( QVector3D pos,QVector3D contourDir){


		ModelSpec stEl;

		stEl.transMatrix.setToIdentity();
		stEl.transMatrix.translate(pos);
		QVector3D perP=QVector3D::crossProduct(contourDir,QVector3D(0,0,1.0f));

		float rotAngle=atan2(perP.y(),perP.x())*57.2957795f;//rad to degrees (angle to rotate will be the tan since we compare respect 1,0,0)
		stEl.transMatrix.rotate(rotAngle,0.0f,0.0f,1.0f);

		stEl.colors.resize(1);
		//body xolor
		stEl.colors[0]=QVector3D(0.35f,0.35f,0.35f);
		stEl.type=1;

		return stEl;
	}


	bool VBOVegetation::generateVegetation(VBORenderManager& rendManager,
			PlaceTypesMainClass &placeTypesIn,std::vector< Block > &blocks){

		QTime tim;
		tim.start();

		std::cout << "Generating vegetation...";

		//for(int i=0; i<blocks.size(); ++i){
			//blocks[i].streetElementInfo.clear();
		//}
		rendManager.removeAllStreetElementName("streetLamp");
		rendManager.removeAllStreetElementName("tree");

		float treesPerSqMeter = 0.002f; //used for trees in parcels
		float distanceBetweenTrees = 31.0f;//23 N 15.0f; //used for trees along streets
		float minTreeHeight = 10.0f;
		float maxTreeHeight = 20.0f;
		float minTreeRadToHeightRatio = 0.3f;
		float maxTreeRadToHeightRatio = 0.5f;

		float distanceBetweenStreetLamps = 43.0f;//30 from google but too close

		int numTreesInParcel;

		
		float rad, height;
		float parcelBBoxArea;
		QVector3D pos;


		float xmin, xmax, ymin, ymax;
		float xpos, ypos, zpos;

		BBox3D parcelBBox;

		//generate trees in blocks
		for(int bN=0;bN<blocks.size();bN++){
		if(blocks[bN].isPark==false) continue;//skip those without parks

			blocks[bN].computeMyBBox3D();
			parcelBBox = blocks[bN].bbox;

			xmin = parcelBBox.minPt.x();
			xmax = parcelBBox.maxPt.x();
			ymin = parcelBBox.minPt.y();
			ymax = parcelBBox.maxPt.y();

			parcelBBoxArea = (xmax - xmin)*(ymax - ymin);

			numTreesInParcel = (int)(parcelBBoxArea*treesPerSqMeter);

			for(int i=0; i<numTreesInParcel; ++i){		
				pos.setX( (float(qrand())/RAND_MAX)*(xmax-xmin)+xmin );
				pos.setY( (float(qrand())/RAND_MAX)*(ymax-ymin)+ymin );
				pos.setZ( 0.0f );
				if(blocks[bN].blockContour.isPointWithinLoop(pos)){
					rendManager.addStreetElementModel("tree",addTree(pos));
				}
				//if(blocks[i].blockContour.contour.size()>0)
				//	printf("pos %f %f --> %f %f \n",pos.x(),pos.y(),blocks[i].blockContour.contour[0].x(),blocks[i].blockContour.contour[0].y());
			}
		}
		


		//generate trees along streets
		float blockSetback;
		QVector3D ptThis, ptNext;
		QVector3D segmentVector;
		float segmentLength;
		int numTreesAlongSegment;
		std::vector <QVector3D> *contourPtr;
		for(int i=0; i<blocks.size(); ++i){

			srand(blocks.at(i).randSeed);


			blockSetback = placeTypesIn.myPlaceTypes.at(0).getFloat("pt_parcel_setback_front");


			blockSetback = 5.0f;

			if(blockSetback >= 4.0f){ //add trees along road only if there's enough space

				contourPtr = &(blocks.at(i).blockContour.contour);

				for(int j=0; j<contourPtr->size(); ++j){
					ptThis = contourPtr->at(j);
					ptNext = contourPtr->at((j+1)%contourPtr->size());
					segmentVector = ptNext - ptThis;
					segmentLength = segmentVector.length();
					segmentVector/=segmentLength;

					QVector3D perpV=QVector3D::crossProduct(segmentVector,QVector3D(0,0,1));
					ptThis=ptThis-perpV*5.5f;

					// Trees
					float distFromSegmentStart = 0.0f;
					while(true){					
						distFromSegmentStart += distanceBetweenTrees*(0.8f+(0.4f*qrand()/RAND_MAX));
						if(distFromSegmentStart > segmentLength){
							break;
						}
						pos = ptThis + segmentVector*distFromSegmentStart;
						pos.setZ(1.0f);//pavement at 1.5f
						rendManager.addStreetElementModel("tree",addTree(pos));

					}
					// StreetLamps
					int numStreetLamps=ceil((segmentLength-1.0f)/distanceBetweenStreetLamps);//-1.0 to leave space at the beginning
					if(numStreetLamps<2)numStreetLamps=2;
					float distanceBetweenPosts=(segmentLength-1.0f)/(numStreetLamps-1);
					for(int i=0;i<numStreetLamps-1;i++){//not in the end (avoid two in the corner)
						pos = ptThis + segmentVector*(0.5f+i*distanceBetweenPosts);

						//blocks[i].streetElementInfo.push_back(addStreetLap(pos,segmentVector));
						rendManager.addStreetElementModel("streetLamp",addStreetLap(pos,segmentVector));
					}

				}
			}
		}

		std::cout << "\t" << tim.elapsed() << " ms\n";
		return true;
	}//
