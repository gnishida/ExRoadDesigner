/************************************************************************************************
*
*		@desc Class containing the road graph
*		@author igarciad
*
************************************************************************************************/

#include "VBORoadGraph.h"

#include "VBORenderManager.h"
#include "polygon3D.h"
#include "global.h"
#include "Util.h"
#include "GraphUtil.h"


void VBORoadGraph::clearVBORoadGraph(VBORenderManager& rendManager){
	rendManager.removeStaticGeometry("3d_roads");
}//

bool compare2ndPartTuple (const std::pair<std::pair<QVector3D,QVector2D>,float> &i, const std::pair<std::pair<QVector3D,QVector2D>,float> &j) {
	return (i.second<j.second);
}


void VBORoadGraph::updateRoadGraph(VBORenderManager& rendManager,RoadGraph &roadGraph){
	const int renderRoadType=3;
	float deltaZ=G::global().getFloat("3d_road_deltaZ");//avoid road intersect with terrain
	// 0 No polylines No intersection
	// 1 Polylines Circle Intersection --> GOOD
	// 2 Polylines Circle+Poly Intersection
	// 3 Polylines Circle+Complex --> GOOD

	//////////////////////////////////////////
	// TYPE=0 No polylines No intersection
	if(renderRoadType==0){

		clearVBORoadGraph(rendManager);
		clearVBORoadGraph(rendManager);
		RoadEdgeIter ei, eiEnd;
		QVector3D p0,p1;
		int numEdges=0;

		std::vector<Vertex> vertROAD;
		std::vector<Vertex> vertINT;
		QVector3D a0,a1,a2,a3;
		float width=3.5f;
		BBox3D roadBB;
		for(boost::tie(ei, eiEnd) = boost::edges(roadGraph.graph);
			ei != eiEnd; ++ei)
		{
			if (!roadGraph.graph[*ei]->valid) continue;
			numEdges++;

			RoadEdgePtr edge = roadGraph.graph[*ei];

			p0 = edge->polyline.front();//roadGraph.graph[boost::source(*ei, roadGraph.graph)].pt;
			p1 = edge->polyline.back();//roadGraph.graph[boost::target(*ei, roadGraph.graph)].pt;
			p0.setZ(1.0f);
			p1.setZ(1.0f);

			roadBB.addPoint(p0);

			QVector3D dir=(p1-p0);//.normalized();
			float length=dir.length();
			dir/=length;
			QVector3D per=(QVector3D::crossProduct(dir,QVector3D(0,0,1.0f)).normalized());

			int numSeg=0;
			const float intersectionClearance=3.5f;

			/// no room for one line
			if(length<2.0f*intersectionClearance){//no real line
				a0=QVector3D(p0.x()+per.x()*width, p0.y()+per.y()*width, p0.z());
				a1=QVector3D(p1.x()+per.x()*width, p1.y()+per.y()*width, p0.z());

				a2=QVector3D(p1.x()-per.x()*width, p1.y()-per.y()*width, p0.z());
				a3=QVector3D(p0.x()-per.x()*width, p0.y()-per.y()*width, p0.z());

				vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
				vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
				vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
				vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));
			}else{
				QVector3D p0_,p1_;
				// START
				p0_=p0;
				p1_=p0+dir*intersectionClearance;
				a0=QVector3D(p0_.x()+per.x()*width, p0_.y()+per.y()*width, p0_.z());
				a1=QVector3D(p1_.x()+per.x()*width, p1_.y()+per.y()*width, p0_.z());

				a2=QVector3D(p1_.x()-per.x()*width, p1_.y()-per.y()*width, p0_.z());
				a3=QVector3D(p0_.x()-per.x()*width, p0_.y()-per.y()*width, p0_.z());

				vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
				vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
				vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
				vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));

				// MIDDLE

				float middLenght=length-2.0f*intersectionClearance;
				float const maxSegmentLeng=5.0f;
				float segmentLeng;
				if(middLenght>2*maxSegmentLeng){
					int numSegments=ceil(length/5.0f);
					float lengthMoved=0;
					float dW=(2*width);
					for(int nS=0;nS<numSegments;nS++){
						segmentLeng=std::min(maxSegmentLeng,middLenght);
						a0=a1;
						a3=a2;
						a1=a1+dir*(segmentLeng);
						a2=a2+dir*(segmentLeng);
						vertROAD.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,lengthMoved/dW,0.0f)));
						vertROAD.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,(segmentLeng+lengthMoved)/dW,0)));
						vertROAD.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,(segmentLeng+lengthMoved)/dW,0)));
						vertROAD.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,lengthMoved/dW,0)));
						lengthMoved+=segmentLeng;
						middLenght-=segmentLeng;
					}

				}else{
					// JUST ONE MIDDLE SEGMENT
					a0=a1;
					a3=a2;
					a1=a1+dir*(middLenght);
					a2=a2+dir*(middLenght);

					vertROAD.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,0.0f,0)));
					vertROAD.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,middLenght/(2*width),0)));
					vertROAD.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,middLenght/(2*width),0)));
					vertROAD.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,0.0f,0)));
				}
				// END
				a0=a1;
				a3=a2;

				a1=a1+dir*intersectionClearance;
				a2=a2+dir*intersectionClearance;

				vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
				vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
				vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
				vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));

			}
		}




		// add all geometry
		if(G::global()["3d_render_mode"]==1){//LC
			rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1.png",GL_QUADS, 2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD,"../data/textures/street_0.png",GL_QUADS,2|mode_AdaptTerrain);
		}
		if(G::global()["3d_render_mode"]==2){//hatch
			rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1b.png",GL_QUADS, 2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD,"../data/textures/street_0b.png",GL_QUADS,2|mode_AdaptTerrain);
			/*// add bbox
			std::vector<QVector3D> vertBB;
			vertBB.push_back(QVector3D(roadBB.minPt.x(),roadBB.minPt.y(),0));
			vertBB.push_back(QVector3D(roadBB.maxPt.x(),roadBB.minPt.y(),0));
			vertBB.push_back(QVector3D(roadBB.maxPt.x(),roadBB.maxPt.y(),0));
			vertBB.push_back(QVector3D(roadBB.minPt.x(),roadBB.maxPt.y(),0));
			rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"hatching_array",GL_QUADS,11|mode_TexArray|mode_Lighting|mode_AdaptTerrain,QVector3D(1.0f/(5.0f),1.0f/(5.0f),1.0f),QVector3D(0,0,0));
			*/
		}
	}

	//////////////////////////////////////////
	// TYPE=1/2/3 POLYLINES 
	if(renderRoadType==1||renderRoadType==2||renderRoadType==3){

		clearVBORoadGraph(rendManager);

		float const maxSegmentLeng=5.0f;//5.0f

		RoadEdgeIter ei, eiEnd;
		QVector3D p0,p1;
		int numEdges=0;

		std::vector<Vertex> vertROAD[2];
		std::vector<Vertex> intersectCirclesV;
		QVector3D a0,a1,a2,a3;
		QVector3D per,dir,lastDir;
		float length;
		//BBox3D roadBB;
		for(boost::tie(ei, eiEnd) = boost::edges(roadGraph.graph);
			ei != eiEnd; ++ei)
		{
			if (!roadGraph.graph[*ei]->valid) continue;
			numEdges++;

			RoadEdgePtr edge = roadGraph.graph[*ei];
			float hWidth=1.1f*roadGraph.graph[*ei]->getWidth()/2.0f;//magic 1.1f !!!! (make roads go below buildings
			//printf("roadGraph.graph[*ei]->type %d\n",roadGraph.graph[*ei]->type);
			int type;
			switch (roadGraph.graph[*ei]->type) {
			case RoadEdge::TYPE_HIGHWAY:
				type=1;//should have its texture!!! TODO
				break;
			case RoadEdge::TYPE_BOULEVARD:
			case RoadEdge::TYPE_AVENUE:
				type=1;
				break;
			case RoadEdge::TYPE_STREET:
				type=0;
				break;
			default:
				type=0;
				break;
			}

			//float lengthMoved=0;//road texture dX
			
			float lengthMovedL=0;//road texture dX
			float lengthMovedR=0;//road texture dX

			for(int pL=0;pL<edge->polyline3D.size()-1;pL++){//note -1
				bool bigAngle=false;
				p0 = edge->polyline3D[pL];
				p1 = edge->polyline3D[pL+1];
				p0.setZ(deltaZ);
				p1.setZ(deltaZ);

				lastDir=dir;//save to compare

				dir=(p1-p0);//.normalized();
				length=dir.length();
				dir/=length;
				
				//per=(QVector3D::crossProduct(dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?

				per = QVector3D(-dir.y(), dir.x(), 0.0f).normalized();
				if (pL == 0) {
					a0 = p0 - per * hWidth;
					a3 = p0 + per * hWidth;
				}
				a1 = p1 - per * hWidth;
				a2 = p1 + per * hWidth;

				if (pL < edge->polyline3D.size() - 2) {
					QVector3D p2 = edge->polyline3D[pL + 2];
					p2.setZ(deltaZ);
					
					Util::getIrregularBisector(p0, p1, p2, hWidth, hWidth, a2);
					Util::getIrregularBisector(p0, p1, p2, -hWidth, -hWidth, a1);
					a1.setZ(deltaZ);
					a2.setZ(deltaZ);
				}
				
				float middLenghtR=length;
				float middLenghtL=length;
				float segmentLengR, segmentLengL;
				int numSegments=ceil(length/5.0f);

				float dW=7.5f;//tex size in m

				QVector3D b0, b3;
				QVector3D b1 = a0;
				QVector3D b2 = a3;
				QVector3D vecR = a1 - a0;
				QVector3D vecL = a2 - a3;

				for(int nS=0;nS<numSegments;nS++){
					segmentLengR=std::min(maxSegmentLeng,middLenghtR);
					segmentLengL=std::min(maxSegmentLeng,middLenghtL);

					b0 = b1;
					b3 = b2;
					if (nS < numSegments - 1) {
						b1 += dir * segmentLengR;
						b2 += dir * segmentLengL;
					} else {
						b1 = a1;
						b2 = a2;
					}
					//printf("a %f %f b %f %f %f %f\n",a2.z(),a1.z(),b0.z(),b1.z(),b2.z(),b3.z());
					vertROAD[type].push_back(Vertex(b0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1,lengthMovedR / dW,0)));
					vertROAD[type].push_back(Vertex(b1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1,(lengthMovedR + segmentLengR) / dW,0)));
					vertROAD[type].push_back(Vertex(b2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,(lengthMovedL + segmentLengL) / dW,0)));
					vertROAD[type].push_back(Vertex(b3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,lengthMovedL / dW,0)));

					lengthMovedR+=segmentLengR;
					lengthMovedL+=segmentLengL;
					middLenghtR-=segmentLengR;
					middLenghtL-=segmentLengL;
				}				

				a3 = a2;
				a0 = a1;



				/*
				if(pL>0){//first not check if bigAngle
					if(acos(QVector3D::dotProduct(lastDir,dir))>(45.0f*0.0174532925f)){//angle bigger than 45 degrees
						bigAngle=true;
						//force restart segment
						{//add circle below
							QVector3D center=p0;
							p0.setZ(0.9f);
							float radi1 = edge->getWidth() /2.0f;
							const float numSides=20;
							const float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
							float angle=0.0f;
							QVector3D nP,oP;
							oP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );//init point
							for(int i=0;i<numSides+1;i++){
								angle=deltaAngle*i;
								nP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );

								intersectCirclesV.push_back(Vertex(center,center/7.5f));
								intersectCirclesV.push_back(Vertex(center+oP,(center+oP)/7.5f));
								intersectCirclesV.push_back(Vertex(center+nP,(center+nP)/7.5f));
								oP=nP;
							}

						}//circle below
					}
				}

				// 

				float middLenght=length;
				float segmentLeng;
				int numSegments=ceil(length/5.0f);

				float dW=7.5f;//tex size in m
				for(int nS=0;nS<numSegments;nS++){
					segmentLeng=std::min(maxSegmentLeng,middLenght);
					if((nS==0&&pL==0)||bigAngle){
						a1=QVector3D(p0.x()+per.x()*hWidth, p0.y()+per.y()*hWidth, 1.0f);
						a2=QVector3D(p0.x()-per.x()*hWidth, p0.y()-per.y()*hWidth, 1.0f);
						bigAngle=false;//if was true, just once
					}
					a0=a1;
					a3=a2;

					a1=a1+dir*(segmentLeng);
					a2=a2+dir*(segmentLeng);




					vertROAD[type].push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,lengthMoved/dW,0.0f)));
					vertROAD[type].push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,(segmentLeng+lengthMoved)/dW,0)));
					vertROAD[type].push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,(segmentLeng+lengthMoved)/dW,0)));
					vertROAD[type].push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,lengthMoved/dW,0)));

					lengthMoved+=segmentLeng;
					middLenght-=segmentLeng;
				}
				*/
			}
		}



		// add all geometry
		if(G::global()["3d_render_mode"]==1){//LC
			//rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1.png",GL_QUADS, 2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD[0],"../data/textures/roads/road_2lines.jpg",GL_QUADS,2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD[1],"../data/textures/roads/road_4lines.jpg",GL_QUADS,2|mode_AdaptTerrain);
			if(intersectCirclesV.size()>0)
				rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2|mode_AdaptTerrain);
		}
		if(G::global()["3d_render_mode"]==2){//hatch
			//rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1b.png",GL_QUADS, 2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD[0],"../data/textures/street_0b.png",GL_QUADS,2|mode_AdaptTerrain);
			rendManager.addStaticGeometry("3d_roads",vertROAD[1],"../data/textures/street_0b.png",GL_QUADS,2|mode_AdaptTerrain);
		}
	}

	//////////////////////////////////////////
	// TYPE=1 Circle Intersection

	if(renderRoadType==1){
		// INTERSECTIONS
		std::vector<Vertex> intersectCirclesV;
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roadGraph.graph); vi != vend; ++vi) {
			if (!roadGraph.graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_r = 0;
			int max_roadType = 0;
			float offset = 0.3f;
			RoadOutEdgeIter oei, oeend;
			int numO=0;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, roadGraph.graph); oei != oeend; ++oei) {
				if (!roadGraph.graph[*oei]->valid) continue;

				float r = roadGraph.graph[*oei]->getWidth();
				if (r > max_r) {
					max_r = r;
				}

				if (roadGraph.graph[*oei]->type > max_roadType) {
					max_roadType = roadGraph.graph[*oei]->type;
				}
				numO++;
			}
			QVector3D center=roadGraph.graph[*vi]->pt3D;
			if(numO<=2)
				center.setZ(deltaZ-0.1f);//below
			else
				center.setZ(deltaZ+0.1f);//above

			float radi1 = max_r /2.0f;
			if(numO==2)radi1*=1.10f;
			if(numO>3)radi1*=1.2f;

			const float numSides=20;
			const float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
			float angle=0.0f;
			QVector3D nP,oP;
			oP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );//init point
			for(int i=0;i<numSides+1;i++){
				angle=deltaAngle*i;
				nP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );

				intersectCirclesV.push_back(Vertex(center,center/7.5f));
				intersectCirclesV.push_back(Vertex(center+oP,(center+oP)/7.5f));
				intersectCirclesV.push_back(Vertex(center+nP,(center+nP)/7.5f));
				oP=nP;
			}


		}//all vertex
		rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2|mode_AdaptTerrain);

	}

	//////////////////////////////////////////
	// TYPE=2  Circle+Poly Intersection

	if(renderRoadType==2){
		// 2. INTERSECTIONS
		std::vector<Vertex> intersectCirclesV;

		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roadGraph.graph); vi != vend; ++vi) {
			if (!roadGraph.graph[*vi]->valid) continue;

			int outDegree=boost::out_degree(*vi,roadGraph.graph);
			////////////////////////
			// 2.1 JUST TWO OR LESS--> CIRCLE BELOW
			if(outDegree<=2){
				// get the largest width of the outing edges
				float max_r = 0;
				int max_roadType = 0;
				float offset = 0.3f;
				RoadOutEdgeIter oei, oeend;
				for (boost::tie(oei, oeend) = boost::out_edges(*vi, roadGraph.graph); oei != oeend; ++oei) {
					if (!roadGraph.graph[*oei]->valid) continue;

					float r = roadGraph.graph[*oei]->getWidth();
					if (r > max_r) {
						max_r = r;
					}

					if (roadGraph.graph[*oei]->type > max_roadType) {
						max_roadType = roadGraph.graph[*oei]->type;
					}
				}
				QVector3D center=roadGraph.graph[*vi]->pt3D;
				center.setZ(deltaZ-0.1f);//below

				float radi1 = max_r /2.0f;
				if(outDegree==2)radi1*=1.10f;

				const float numSides=20;
				const float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
				float angle=0.0f;
				QVector3D nP,oP;
				oP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );//init point
				for(int i=0;i<numSides+1;i++){
					angle=deltaAngle*i;
					nP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );

					intersectCirclesV.push_back(Vertex(center,center/7.5f));
					intersectCirclesV.push_back(Vertex(center+oP,(center+oP)/7.5f));
					intersectCirclesV.push_back(Vertex(center+nP,(center+nP)/7.5f));
					oP=nP;
				}

			}else{
				////////////////////////
				// 2.2 TWO OR MORE--> COMPLEX INTERSECTION


				////////////
				// 2.2.1 For each vertex find edges and short by angle
				QVector2D referenceVector(0,1);
				QVector2D p0,p1;
				std::vector<std::pair<std::pair<QVector3D,QVector2D>,float>> edgeAngleOut;
				int numOutEdges=0;
				RoadOutEdgeIter Oei, Oei_end;
				float angleRef=atan2(referenceVector.y(),referenceVector.x());
				for(boost::tie(Oei, Oei_end) = boost::out_edges(*vi,roadGraph.graph); Oei != Oei_end; ++Oei){
					// find first segment 
					RoadEdgePtr edge = roadGraph.graph[*Oei];
					p0=edge->polyline[0];

					if(p0==roadGraph.graph[*vi]->pt){
						p1=edge->polyline[1];
					}else{
						p0=edge->polyline.back();
						p1=edge->polyline[edge->polyline.size()-2];
					}

					QVector2D edgeDir=(p1-p0).normalized();// NOTE p1-p0
					p1=p0+edgeDir*30.0f;//expand edge to make sure it is long enough

					float angle=angleRef-atan2(edgeDir.y(),edgeDir.x());
					float width=edge->getWidth();
					edgeAngleOut.push_back(std::make_pair(std::make_pair(QVector3D(p0.x(),p0.y(),width),p1),angle));//z as width
					numOutEdges++;
				}
				if(edgeAngleOut.size()>0){
					std::sort( edgeAngleOut.begin(), edgeAngleOut.end(), compare2ndPartTuple);
				}
				// 2.2.2 Create intersection geometry of the given edges
				QVector3D ed1p0,ed1p1,ed2p0,ed2p1;
				QVector3D ed1Per,ed2Per;
				float ed1W,ed2W;
				std::vector<QVector3D> interPoints;
				for(int eN=0;eN<edgeAngleOut.size();eN++){
					// a) ED1: this edge
					ed1W=edgeAngleOut[eN].first.first.z();//use z as width
					ed1p0=edgeAngleOut[eN].first.first;
					ed1p0.setZ(deltaZ);
					ed1p1=edgeAngleOut[eN].first.second.toVector3D();
					// compute right side
					QVector3D ed1Dir=(ed1p0-ed1p1).normalized();//ends in 0
					ed1Per=(QVector3D::crossProduct(ed1Dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed1p0+=ed1Per*ed1W/2.0f;
					ed1p1+=ed1Per*ed1W/2.0f;

					// b) ED2: next edge
					int nextEdge=(eN+1)%edgeAngleOut.size();
					ed2W=edgeAngleOut[nextEdge].first.first.z();//use z as width
					ed2p0=edgeAngleOut[nextEdge].first.first;
					ed2p0.setZ(0);
					ed2p1=edgeAngleOut[nextEdge].first.second.toVector3D();
					// compute left side
					QVector3D ed2Dir=(ed2p0-ed2p1).normalized();//ends in 0
					ed2Per=(QVector3D::crossProduct(ed2Dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed2p0-=ed2Per*ed2W/2.0f;
					ed2p1-=ed2Per*ed2W/2.0f;

					// c) computer intersection
					float tab,tcd;
					QVector3D intPoint;
					printf("ED1 %f %f --> %f %f  ED2 %f %f --> %f %f\n",ed1p0.x(),ed1p0.y(),ed1p1.x(),ed1p1.y(), ed2p0.x(),ed2p0.y(),ed2p1.x(),ed2p1.y());
					if(Util::segmentSegmentIntersectXY3D(ed1p0,ed1p1,ed2p0,ed2p1,&tab,&tcd, false,intPoint)==false&&false){
						printf("ERROR: Parallel!!!\n");
					}else{
						printf("Int %f %f\n",intPoint.x(),intPoint.y());
						printf("ADD: No Parallel!!!\n");
						interPoints.push_back(intPoint);
					}
				}
				if(interPoints.size()>2)
					//rendManager.addStaticGeometry2("3d_roads_interCom",interPoints,1.1f,false,"",GL_QUADS,1|mode_AdaptTerrain,QVector3D(1,1,9),QVector3D());
					rendManager.addStaticGeometry2("3d_roads_interCom",interPoints,1.1f,false,"../data/textures/roads/road_0lines.jpg",GL_QUADS,2|mode_AdaptTerrain,QVector3D(1.0f/7.5f,1.0f/7.5f,1),QVector3D());
				//////////////

			}

		}//all vertex
		rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2|mode_AdaptTerrain);
	}

	//////////////////////////////////////////
	// TYPE=3  Circle+Complex

	if(renderRoadType==3){
		// 2. INTERSECTIONS
		std::vector<Vertex> intersectCirclesV;

		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roadGraph.graph); vi != vend; ++vi) {
			if (!roadGraph.graph[*vi]->valid) continue;

			int outDegree=0;//boost::out_degree(*vi,roadGraph.graph);
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, roadGraph.graph); oei != oeend; ++oei) {
				if (!roadGraph.graph[*oei]->valid) continue;
				outDegree++;
			}
			////////////////////////
			// 2.1 JUST TWO OR LESS--> CIRCLE BELOW
			if(outDegree<=3){//if(outDegree<=2){
				// get the largest width of the outing edges
				float max_r = 0;
				int max_roadType = 0;
				float offset = 0.3f;
				RoadOutEdgeIter oei, oeend;
				for (boost::tie(oei, oeend) = boost::out_edges(*vi, roadGraph.graph); oei != oeend; ++oei) {
					if (!roadGraph.graph[*oei]->valid) continue;

					float r = roadGraph.graph[*oei]->getWidth();
					if (r > max_r) {
						max_r = r;
					}

					if (roadGraph.graph[*oei]->type > max_roadType) {
						max_roadType = roadGraph.graph[*oei]->type;
					}
				}
				QVector3D center=roadGraph.graph[*vi]->pt3D;
				if(outDegree<=2)
					center.setZ(deltaZ-0.1f);//below
				else
					center.setZ(deltaZ+0.1f);//above

				float radi1 = max_r /2.0f;
				if(outDegree==2)radi1*=1.10f;

				const float numSides=20;
				const float deltaAngle=( 1.0f / numSides ) * 3.14159f * 2.0f;
				float angle=0.0f;
				QVector3D nP,oP;
				oP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );//init point
				for(int i=0;i<numSides+1;i++){
					angle=deltaAngle*i;
					nP=QVector3D( radi1 * cos( angle ), radi1 * sin( angle ),0.0f );

					intersectCirclesV.push_back(Vertex(center,center/7.5f));
					intersectCirclesV.push_back(Vertex(center+oP,(center+oP)/7.5f));
					intersectCirclesV.push_back(Vertex(center+nP,(center+nP)/7.5f));
					oP=nP;
				}

			}else{

					////////////////////////
					// 2.2 FOUR OR MORE--> COMPLEX INTERSECTION

					//printf("a\n");
					////////////
					// 2.2.1 For each vertex find edges and short by angle
					QVector2D referenceVector(0,1);
					QVector2D p0,p1;
					std::vector<std::pair<std::pair<QVector3D,QVector2D>,float>> edgeAngleOut;
					int numOutEdges=0;
					RoadOutEdgeIter Oei, Oei_end;
					float angleRef=atan2(referenceVector.y(),referenceVector.x());
					//printf("a1\n");
					for(boost::tie(Oei, Oei_end) = boost::out_edges(*vi,roadGraph.graph); Oei != Oei_end; ++Oei){
						if (!roadGraph.graph[*Oei]->valid) continue;
						// find first segment 
						RoadEdgePtr edge = roadGraph.graph[*Oei];
						//printf("a11 poly %d\n",edge->polyline.size());

						Polyline2D polyline = GraphUtil::orderPolyLine(roadGraph, *Oei, *vi);
						p0 = polyline[0];
						p1 = polyline[1];

						QVector2D edgeDir=(p1-p0).normalized();// NOTE p1-p0
						p1=p0+edgeDir*30.0f;//expand edge to make sure it is long enough

						float angle=angleRef-atan2(edgeDir.y(),edgeDir.x());
						float width=edge->getWidth()*1.1f;//1.1 (since in render this is also applied)
						edgeAngleOut.push_back(std::make_pair(std::make_pair(QVector3D(p0.x(),p0.y(),width),p1),angle));//z as width
						numOutEdges++;
					}
					//printf("a2\n");
					if(edgeAngleOut.size()>0){
						std::sort( edgeAngleOut.begin(), edgeAngleOut.end(), compare2ndPartTuple);
					}
					//printf("b\n");
					// 2.2.2 Create intersection geometry of the given edges
					QVector3D ed1p0,ed1p1;
					QVector3D ed1p0L,ed1p1L,ed1p0R,ed1p1R;
					QVector3D ed2p0L,ed2p1L,ed2p0R,ed2p1R;//right
					QVector3D ed1Dir,ed1Per;
					QVector3D ed2DirL,ed2DirR,ed2PerL,ed2PerR;
					float ed1W;
					float ed2WL,ed2WR;
					std::vector<QVector3D> interPoints;
					std::vector<Vertex> interVertex;
					std::vector<Vertex> interPedX;
					std::vector<Vertex> interPedXLineR;
					//printf("c\n");
					for(int eN=0;eN<edgeAngleOut.size();eN++){
						//printf("** eN %d\n",eN);
						// a) ED1: this edge
						ed1W=edgeAngleOut[eN].first.first.z();//use z as width
						ed1p0=edgeAngleOut[eN].first.first;
						ed1p0.setZ(0);
						ed1p1=edgeAngleOut[eN].first.second.toVector3D();
						// compute right side
						ed1Dir=(ed1p0-ed1p1).normalized();//ends in 0
						ed1Per=(QVector3D::crossProduct(ed1Dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?
						ed1p0R=ed1p0+ed1Per*ed1W/2.0f;
						ed1p1R=ed1p1+ed1Per*ed1W/2.0f;
						// compute left side
						ed1p0L=ed1p0-ed1Per*ed1W/2.0f;
						ed1p1L=ed1p1-ed1Per*ed1W/2.0f;

						// b) ED2: next edge
						int lastEdge=eN-1;
						if(lastEdge<0)lastEdge=edgeAngleOut.size()-1;
						//printf("last eN %d\n",lastEdge);
						ed2WL=edgeAngleOut[lastEdge].first.first.z();//use z as width
						ed2p0L=edgeAngleOut[lastEdge].first.first;
						ed2p0L.setZ(0);
						ed2p1L=edgeAngleOut[lastEdge].first.second.toVector3D();
						// compute left side
						ed2DirL=(ed2p0L-ed2p1L).normalized();//ends in 0
						ed2PerL=(QVector3D::crossProduct(ed2DirL,QVector3D(0,0,1.0f)).normalized());//need normalized()?
						ed2p0L-=ed2PerL*ed2WL/2.0f;
						ed2p1L-=ed2PerL*ed2WL/2.0f;

						// c) ED2: last edge
						int nextEdge=(eN+1)%edgeAngleOut.size();
						//printf("next eN %d\n",nextEdge);

						ed2WR=edgeAngleOut[nextEdge].first.first.z();//use z as width
						ed2p0R=edgeAngleOut[nextEdge].first.first;
						ed2p0R.setZ(0);
						ed2p1R=edgeAngleOut[nextEdge].first.second.toVector3D();
						// compute left side
						ed2DirR=(ed2p0R-ed2p1R).normalized();//ends in 0
						ed2PerR=(QVector3D::crossProduct(ed2DirR,QVector3D(0,0,1.0f)).normalized());//need normalized()?
						ed2p0R+=ed2PerR*ed2WR/2.0f;
						ed2p1R+=ed2PerR*ed2WR/2.0f;

						//////////////////////////////////////////
						// d) Computer interior coordinates
						// d.1 computer intersection left
						QVector3D intPoint1(FLT_MAX,0,0);
						if(acos(QVector3D::dotProduct(ed1Dir,ed2DirL))<(170.0f*0.0174532925f)){//angle smaller than 45 degrees
							float tab,tcd;
							//printf("ED1 %f %f --> %f %f  ED2 %f %f --> %f %f\n",ed1p0R.x(),ed1p0R.y(),ed1p1R.x(),ed1p1R.y(), ed2p0L.x(),ed2p0L.y(),ed2p1L.x(),ed2p1L.y());
							if(Util::segmentSegmentIntersectXY3D(ed1p0R,ed1p1R,ed2p0L,ed2p1L,&tab,&tcd, false,intPoint1)==false&&false){
								printf("ERROR: Parallel!!!\n");
								continue;
							}else{
								//printf("Int %f %f\n",intPoint1.x(),intPoint1.y());
								//printf("ADD: No Parallel!!!\n");
							}
						}
						// d.2 computer intersecion right
						QVector3D intPoint2(FLT_MAX,0,0);
						if(acos(QVector3D::dotProduct(ed1Dir,ed2DirR))<(170.0f*0.0174532925f)){//angle smaller than 45 degrees
							float tab,tcd;
							//printf("ED1 %f %f --> %f %f  ED2 %f %f --> %f %f\n",ed1p0L.x(),ed1p0L.y(),ed1p1L.x(),ed1p1L.y(), ed2p0R.x(),ed2p0R.y(),ed2p1R.x(),ed2p1R.y());
							if(Util::segmentSegmentIntersectXY3D(ed1p0L,ed1p1L,ed2p0R,ed2p1R,&tab,&tcd, false,intPoint2)==false){
								printf("ERROR: Parallel!!!\n");
								continue;
							}else{
								//printf("Int %f %f\n",intPoint2.x(),intPoint2.y());
								//printf("ADD: No Parallel!!!\n");
							}
						}
						if(intPoint1.x()==FLT_MAX&&intPoint2.x()==FLT_MAX){
							printf("ERROR: No intersect both sides\n");
							printf("angle1 %f\n",acos(QVector3D::dotProduct(ed1Dir,ed2DirR))/0.0174532925f);
							printf("angle2 %f\n",acos(QVector3D::dotProduct(ed1Dir,ed2DirL))/0.0174532925f);
							exit(0);//for now exit program
						}
						if(intPoint1.x()==FLT_MAX){
							intPoint1=intPoint2-ed1Per*ed1W;
						}
						if(intPoint2.x()==FLT_MAX){
							intPoint2=intPoint1+ed1Per*ed1W;
						}

						// middle
						float zOff=0.1f;
						intPoint2.setZ(deltaZ+zOff);
						intPoint1.setZ(deltaZ+zOff);
					
							interPoints.push_back(intPoint1);
							interPoints.push_back(intPoint2);
						
						
						{// POINTS
							

							//if(eN==0){
							// original line
							//ed1p0.setZ(zOff);
							//ed1p1.setZ(zOff);
							//interVertex.push_back(Vertex(ed1p0,QVector3D(0,0,1),QVector3D(),QVector3D()));
							//interVertex.push_back(Vertex(ed1p1,QVector3D(0,0,1),QVector3D(),QVector3D()));

							//inter 2
							/*ed1p0L.setZ(zOff);
							interVertex.push_back(Vertex(ed1p0L,QVector3D(0,1,1),QVector3D(),QVector3D()));
							ed1p1L.setZ(zOff);
							interVertex.push_back(Vertex(ed1p1L,QVector3D(0,1,1),QVector3D(),QVector3D()));
							ed2p0R.setZ(zOff);
							interVertex.push_back(Vertex(ed2p0R,QVector3D(1,1,1),QVector3D(),QVector3D()));
							ed2p1R.setZ(zOff);
							interVertex.push_back(Vertex(ed2p1R,QVector3D(1,1,1),QVector3D(),QVector3D()));*/

							//interVertex.push_back(Vertex(intPoint2,QVector3D(),QVector3D(),QVector3D()));
							//interVertex.push_back(Vertex(intPoint2-ed1Dir*5.0f,QVector3D(1,0,0),QVector3D(),QVector3D()));


							//inter 1
							/*ed1p0R.setZ(zOff);
							interVertex.push_back(Vertex(ed1p0R,QVector3D(0,1,1),QVector3D(),QVector3D()));
							ed1p1R.setZ(zOff);
							interVertex.push_back(Vertex(ed1p1R,QVector3D(0,1,1),QVector3D(),QVector3D()));
							ed2p0L.setZ(zOff);
							interVertex.push_back(Vertex(ed2p0L,QVector3D(1,1,1),QVector3D(),QVector3D()));
							ed2p1L.setZ(zOff);
							interVertex.push_back(Vertex(ed2p1L,QVector3D(1,1,1),QVector3D(),QVector3D()));*/

							//interVertex.push_back(Vertex(intPoint1-ed1Dir*5.0f,QVector3D(1,0,0),QVector3D(),QVector3D()));
							//interVertex.push_back(Vertex(intPoint1,QVector3D(),QVector3D(),QVector3D()));
							//}
						}

						// EDIT: to make the stop line perpendicular to the road direction
						if (QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir) >= 0) {
							intPoint1 -= ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
						} else {
							intPoint2 += ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
						}

						// pedX
						interPedX.push_back(Vertex(intPoint1,QVector3D(0-0.07f,0,0)));
						interPedX.push_back(Vertex(intPoint2,QVector3D(ed1W/7.5f+0.07f,0,0)));
						interPedX.push_back(Vertex(intPoint2-ed1Dir*3.5f,QVector3D(ed1W/7.5f+0.07f,1.0f,0)));
						interPedX.push_back(Vertex(intPoint1-ed1Dir*3.5f,QVector3D(0.0f-0.07f,1.0f,0)));
						// Line in right lines
						QVector3D midPoint=(intPoint2+intPoint1)/2.0f+0.2f*ed1Per;


						interPedXLineR.push_back(Vertex(intPoint1-ed1Dir*3.5f,QVector3D(0,0.0f,0)));
						interPedXLineR.push_back(Vertex(midPoint-ed1Dir*3.5f,QVector3D(1.0f,0.0f,0)));
						interPedXLineR.push_back(Vertex(midPoint-ed1Dir*4.25f,QVector3D(1.0f,1.0f,0)));
						interPedXLineR.push_back(Vertex(intPoint1-ed1Dir*4.25f,QVector3D(0.0f,1.0f,0)));

					}
					//printf("EdgeOut %d interVertex %d\n",edgeAngleOut.size(),interVertex.size());
					//rendManager.addStaticGeometry("3d_roads_interCom",interVertex,"../data/textures/roads/road_pedX.jpg",GL_POINTS,1|mode_AdaptTerrain);//POINTS (tex meant to set points)
					if(interPoints.size()>2){
						
						{
							for(int iP=0;iP<interPoints.size()-1;iP++){//remove duplicated
								if((interPoints[iP]-interPoints[iP+1]).lengthSquared()<0.5f){
									interPoints.erase(interPoints.begin()+iP);
									iP--;
								}
							}
						}
						/*{
							
							for(int iP=0;iP<interPoints.size();iP++)
								interVertex.push_back(Vertex(interPoints[iP]+iP*QVector3D(0.2,0,0.1f+1.0f/iP),QVector3D(float(iP)/interPoints.size(),float(iP)/interPoints.size(),float(iP)/interPoints.size()),QVector3D(),QVector3D()));
							rendManager.addStaticGeometry("3d_roads_interCom",interVertex,"../data/textures/street_1b.png",GL_POINTS,1|mode_AdaptTerrain);//POINTS (tex meant to set points)
						}
						if(outDegree==5){
							printf("outDegree %d\n",outDegree);
							for(int iP=0;iP<interPoints.size();iP++)
								printf("interPoints %f %f\n",interPoints[iP].x(),interPoints[iP].y());
						}*/
						//rendManager.addStaticGeometry2("3d_roads_interCom",interPoints,1.8f,false,"",GL_QUADS,1|mode_AdaptTerrain,QVector3D(1,1,1),QVector3D());
						rendManager.addStaticGeometry2("3d_roads_interCom",interPoints,0.0f,false,"../data/textures/roads/road_0lines.jpg",GL_QUADS,2|mode_AdaptTerrain,QVector3D(1.0f/7.5f,1.0f/7.5f,1),QVector3D());//0.0f (moved before)
					}
					rendManager.addStaticGeometry("3d_roads_interCom",interPedX,"../data/textures/roads/road_pedX.jpg",GL_QUADS,2|mode_AdaptTerrain);
					rendManager.addStaticGeometry("3d_roads_interCom",interPedXLineR,"../data/textures/roads/road_pedXLineR.jpg",GL_QUADS,2|mode_AdaptTerrain);
					//printf("interX %d\n",interPedX.size());
					//////////////

				
			}

		}//all vertex
		rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2|mode_AdaptTerrain);
	}


}//


void VBORoadGraph::renderRoadGraphLines(VBORenderManager& rendManager){

	/*//glLineWidth(6.0f);
	glLineWidth(6.0f);
	GLuint vao;
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);
	//
	if(line_wasModified==true){// it was modified
	if(linesVBO!=INT_MAX){//it was already created--> remove
	glDeleteVertexArrays(1, &linesVBO);
	}
	glGenVertexArrays(1,&linesVBO);
	glBindBuffer(GL_ARRAY_BUFFER,linesVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*line_Vert.size(),line_Vert.data(),GL_STATIC_DRAW);
	line_wasModified=false;
	}else{
	glBindBuffer(GL_ARRAY_BUFFER,linesVBO);
	}

	glUniform1i (glGetUniformLocation (rendManager.program, "terrain_tex"), 7);//tex0: 0
	glUniform4f (glGetUniformLocation (rendManager.program, "terrain_size"), 
	rendManager.vboTerrain.terrainLayer.minPos.x(),
	rendManager.vboTerrain.terrainLayer.minPos.y(),
	(rendManager.vboTerrain.terrainLayer.maxPos.x()-rendManager.vboTerrain.terrainLayer.minPos.x()), 
	(rendManager.vboTerrain.terrainLayer.maxPos.y()-rendManager.vboTerrain.terrainLayer.minPos.y())
	);//tex0: 0

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
	glEnableVertexAttribArray(1);
	VBOUtil::check_gl_error("aa editionMode");
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));
	glUniform1i (glGetUniformLocation (rendManager.program, "mode"), 1|LC::mode_AdaptTerrain);//MODE: color


	//RendUtil::enableAttributesInShader(programId);
	glDrawArrays(GL_LINES,0,line_Vert.size());
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	glDeleteVertexArrays(1,&vao);*/
}//

