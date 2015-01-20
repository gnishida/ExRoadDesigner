#include "RoadMeshGenerator.h"
#include "Util.h"
#include "GraphUtil.h"

bool compare2ndPartTuple2 (const std::pair<float, RoadEdgeDesc> &i, const std::pair<float, RoadEdgeDesc> &j) {
	return (i.first > j.first);
}

void RoadMeshGenerator::generateRoadMesh(VBORenderManager& rendManager, RoadGraph& roads) {
	rendManager.removeStaticGeometry("3d_roads");
	rendManager.removeStaticGeometry("3d_roads_interCom");
	rendManager.removeStaticGeometry("3d_roads_inter");

	const float deltaL = 1.0f;

	//////////////////////////////////////////
	// POLYLINES 
	{
		float const maxSegmentLeng=5.0f;//5.0f

		RoadEdgeIter ei, eiEnd;
		QVector3D p0,p1;

		std::vector<Vertex> vertROAD[2];
		std::vector<Vertex> vertSide;	// 道路の側面部

		QVector3D a0,a1,a2,a3;
		QVector3D per,dir,lastDir;
		float length;
		for (boost::tie(ei, eiEnd) = boost::edges(roads.graph); ei != eiEnd; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			RoadEdgePtr edge = roads.graph[*ei];
			float hWidth = roads.graph[*ei]->getWidth()/2.0f;//magic 1.1f !!!! (make roads go below buildings

			int type;
			switch (roads.graph[*ei]->type) {
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

			// GEN
			type = 0;
			if (roads.graph[*ei]->lanes > 2) {
				type = 1;
			}
			
			float lengthMovedL=0;//road texture dX
			float lengthMovedR=0;//road texture dX

			for (int pL = 0; pL < edge->polyline3D.size()-1; pL++) {
				bool bigAngle=false;
				p0 = edge->polyline3D[pL];
				p1 = edge->polyline3D[pL+1];
				//p0.setZ(deltaZ);
				//p1.setZ(deltaZ);

				lastDir=dir;//save to compare

				dir=(p1-p0);//.normalized();
				length=dir.length();
				dir/=length;
				
				per = QVector3D(-dir.y(), dir.x(), 0.0f).normalized();
				if (pL == 0) {
					a0 = p0 - per * hWidth;
					a3 = p0 + per * hWidth;
				}
				a1 = p1 - per * hWidth;
				a2 = p1 + per * hWidth;

				if (pL < edge->polyline3D.size() - 2) {
					QVector3D p2 = edge->polyline3D[pL + 2];
					//p2.setZ(deltaZ);
					
					Util::getIrregularBisector(p0, p1, p2, hWidth, hWidth, a2);
					Util::getIrregularBisector(p0, p1, p2, -hWidth, -hWidth, a1);
					//a1.setZ(deltaZ);
					//a2.setZ(deltaZ);
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
					vertROAD[type].push_back(Vertex(b0,QColor(),QVector3D(0,0,1.0f),QVector3D(1,lengthMovedR / dW,0)));
					vertROAD[type].push_back(Vertex(b1,QColor(),QVector3D(0,0,1.0f),QVector3D(1,(lengthMovedR + segmentLengR) / dW,0)));
					vertROAD[type].push_back(Vertex(b2,QColor(),QVector3D(0,0,1.0f),QVector3D(0,(lengthMovedL + segmentLengL) / dW,0)));
					vertROAD[type].push_back(Vertex(b3,QColor(),QVector3D(0,0,1.0f),QVector3D(0,lengthMovedL / dW,0)));

					// 側面のジオメトリ
					QVector3D b0_d = b0 + QVector3D(0, 0, -1);
					QVector3D b1_d = b1 + QVector3D(0, 0, -1);
					QVector3D b2_d = b2 + QVector3D(0, 0, -1);
					QVector3D b3_d = b3 + QVector3D(0, 0, -1);
					vertSide.push_back(Vertex(b0_d, QColor(64, 64, 64), -per, QVector3D()));
					vertSide.push_back(Vertex(b1_d, QColor(64, 64, 64), -per, QVector3D()));
					vertSide.push_back(Vertex(b1, QColor(64, 64, 64), -per, QVector3D()));
					vertSide.push_back(Vertex(b0, QColor(64, 64, 64), -per, QVector3D()));
					vertSide.push_back(Vertex(b2_d, QColor(64, 64, 64), per, QVector3D()));
					vertSide.push_back(Vertex(b3_d, QColor(64, 64, 64), per, QVector3D()));
					vertSide.push_back(Vertex(b3, QColor(64, 64, 64), per, QVector3D()));
					vertSide.push_back(Vertex(b2, QColor(64, 64, 64), per, QVector3D()));

					lengthMovedR+=segmentLengR;
					lengthMovedL+=segmentLengL;
					middLenghtR-=segmentLengR;
					middLenghtL-=segmentLengL;
				}				

				a3 = a2;
				a0 = a1;
			}
		}

		// add all geometry
		rendManager.addStaticGeometry("3d_roads",vertROAD[0],"../data/textures/roads/road_2lines.jpg",GL_QUADS,2);
		rendManager.addStaticGeometry("3d_roads",vertROAD[1],"../data/textures/roads/road_4lines.jpg",GL_QUADS,2);
		rendManager.addStaticGeometry("3d_roads",vertSide,"",GL_QUADS, 1|mode_Lighting);
	}

	//////////////////////////////////////////
	// Circle+Complex
	{
		// 2. INTERSECTIONS
		std::vector<Vertex> intersectCirclesV;
		std::vector<Vertex> interPedX;
		std::vector<Vertex> interPedXLineR;

		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			int outDegree = GraphUtil::getDegree(roads, *vi);

			if (outDegree == 0) {
				continue;
			} else if (outDegree ==1) { // デッドエンド
				// get the largest width of the outing edges
				float max_r = 0;
				int max_roadType = 0;
				float offset = 0.3f;
				RoadOutEdgeIter oei, oeend;
				for (boost::tie(oei, oeend) = boost::out_edges(*vi, roads.graph); oei != oeend; ++oei) {
					if (!roads.graph[*oei]->valid) continue;

					float r = roads.graph[*oei]->getWidth();
					if (r > max_r) {
						max_r = r;
					}

					if (roads.graph[*oei]->type > max_roadType) {
						max_roadType = roads.graph[*oei]->type;
					}
				}
				QVector3D center = roads.graph[*vi]->pt3D;
				center.setZ(center.z() - 0.1f);//deltaZ-0.1f);//below

				float radi1 = max_r /2.0f;

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

				////////////
				// 2.2.1 For each vertex find edges and sort them in clockwise order
				std::vector<std::pair<float, RoadEdgeDesc> > edgeAngleOut;
				RoadOutEdgeIter Oei, Oei_end;
				QMap<RoadEdgeDesc, bool> visited;
				//printf("a1\n");
				for (boost::tie(Oei, Oei_end) = boost::out_edges(*vi, roads.graph); Oei != Oei_end; ++Oei) {
					if (!roads.graph[*Oei]->valid) continue;
					if (visited[*Oei]) continue;

					// GEN 1/12/2015
					// to avoid some garbage in the boost graph
					RoadVertexDesc tgt = boost::target(*Oei, roads.graph);
					if (*vi == 0 && *vi == tgt) continue;

					Polyline2D polyline = GraphUtil::orderPolyLine(roads, *Oei, *vi);
					QVector2D p0 = polyline[0];
					QVector2D p1 = polyline[1];

					QVector2D edgeDir=(p1-p0).normalized();// NOTE p1-p0

					float angle = atan2(edgeDir.y(),edgeDir.x());
					edgeAngleOut.push_back(std::make_pair(angle, *Oei));//z as width

					// For self-loop edge
					if (tgt == *vi) {
						p0 = polyline.back();
						p1 = polyline[polyline.size() - 2];
						edgeDir = (p1 - p0).normalized();
						float angle = atan2(edgeDir.y(),edgeDir.x());
						edgeAngleOut.push_back(std::make_pair(angle, *Oei));//z as width
					}

					visited[*Oei] = true;
				}
				std::sort(edgeAngleOut.begin(), edgeAngleOut.end(), compare2ndPartTuple2);

				// 2.2.2 Create intersection geometry of the given edges
				std::vector<QVector3D> interPoints;
				std::vector<QVector3D> stopPoints;
				//printf("c\n");
				for(int eN=0;eN<edgeAngleOut.size();eN++){
					//printf("** eN %d\n",eN);
					// a) ED1: this edge
					float ed1W = roads.graph[edgeAngleOut[eN].second]->getWidth();
					Polyline3D ed1poly = GraphUtil::orderPolyLine3D(roads, edgeAngleOut[eN].second, *vi, edgeAngleOut[eN].first);
					QVector3D ed1p1 = ed1poly[1];
					// compute right side
					QVector3D ed1Dir = (roads.graph[*vi]->pt3D - ed1p1).normalized();//ends in 0
					QVector3D ed1Per = (QVector3D::crossProduct(ed1Dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					QVector3D ed1p0R = roads.graph[*vi]->pt3D + ed1Per*ed1W/2.0f;
					QVector3D ed1p1R = ed1p1 + ed1Per*ed1W/2.0f;
					// compute left side
					QVector3D ed1p0L = roads.graph[*vi]->pt3D - ed1Per*ed1W/2.0f;
					QVector3D ed1p1L = ed1p1 - ed1Per*ed1W/2.0f;

					// b) ED2: next edge
					int lastEdge = eN-1;
					if(lastEdge<0)lastEdge = edgeAngleOut.size()-1;
					//printf("last eN %d\n",lastEdge);
					float ed2WL = roads.graph[edgeAngleOut[lastEdge].second]->getWidth();
					QVector3D ed2p0L = roads.graph[*vi]->pt3D;
					Polyline3D ed2polyL = GraphUtil::orderPolyLine3D(roads, edgeAngleOut[lastEdge].second, *vi, edgeAngleOut[lastEdge].first);
					QVector3D ed2p1L = ed2polyL[1];
					// compute left side
					QVector3D ed2DirL = (ed2p0L - ed2p1L).normalized();//ends in 0
					QVector3D ed2PerL = (QVector3D::crossProduct(ed2DirL,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed2p0L-=ed2PerL*ed2WL/2.0f;
					ed2p1L-=ed2PerL*ed2WL/2.0f;

					// c) ED2: last edge
					int nextEdge=(eN+1)%edgeAngleOut.size();

					float ed2WR = roads.graph[edgeAngleOut[nextEdge].second]->getWidth();
					QVector3D ed2p0R = roads.graph[*vi]->pt3D;
					Polyline3D ed2polyR = GraphUtil::orderPolyLine3D(roads, edgeAngleOut[nextEdge].second, *vi, edgeAngleOut[nextEdge].first);
					QVector3D ed2p1R = ed2polyR[1];
					// compute left side
					QVector3D ed2DirR = (ed2p0R - ed2p1R).normalized();//ends in 0
					QVector3D ed2PerR = (QVector3D::crossProduct(ed2DirR,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed2p0R+=ed2PerR*ed2WR/2.0f;
					ed2p1R+=ed2PerR*ed2WR/2.0f;

					//////////////////////////////////////////
					// d) Computer interior coordinates
					// d.1 computer intersection left
					QVector3D intPoint1(FLT_MAX,0,0);
					if (fabs(QVector3D::dotProduct(ed1Dir, ed2DirL)) < 0.95f) { // ２つの隣接道路が平行じゃないなら、
						float tab,tcd;
						if (Util::segmentSegmentIntersectXY3D(ed1p0R,ed1p1R,ed2p0L,ed2p1L,&tab,&tcd, false,intPoint1)==false&&false) {
							printf("ERROR: Parallel!!!\n");
						}
					} else { // ２つの隣接道路が平行なら、
						intPoint1 = (ed1p0R + ed2p0L) * 0.5f;
					}
					// d.2 computer intersecion right
					QVector3D intPoint2(FLT_MAX,0,0);
					if (fabs(QVector3D::dotProduct(ed1Dir, ed2DirR)) < 0.95f) {
						float tab,tcd;
						if (Util::segmentSegmentIntersectXY3D(ed1p0L,ed1p1L,ed2p0R,ed2p1R,&tab,&tcd, false,intPoint2)==false) {
							printf("ERROR: Parallel!!!\n");
						}
					} else {
						intPoint2 = (ed1p0L + ed2p0R) * 0.5f;
					}

					// middle
					float zOff=0.1f;
					intPoint2.setZ(roads.graph[*vi]->pt3D.z() + zOff);//deltaZ+zOff);
					intPoint1.setZ(roads.graph[*vi]->pt3D.z() + zOff);//deltaZ+zOff);
					

					interPoints.push_back(intPoint1);
					intPoint1 -= ed1Dir * deltaL;
					intPoint2 -= ed1Dir * deltaL;
					interPoints.push_back(intPoint1);
					interPoints.push_back(intPoint2);

					// intPoint1、intPoint2を、道路の方向に直行するよう、位置をそろえる
					if (QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir) >= 0) {
						intPoint1 -= ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
					} else {
						intPoint2 += ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
					}

					stopPoints.push_back(intPoint1);
					stopPoints.push_back(intPoint2);

					// create crossing and stop line only if the degree >= 3 and the edge is not self-looping and the edge length is long enough
					if (outDegree >= 3 && (ed1poly[0] - ed1poly.back()).length() > 10.0f && ed1poly.length() > 50.0f) {
						// 横断歩道
						interPedX.push_back(Vertex(intPoint1,QVector3D(0-0.07f,0,0)));
						interPedX.push_back(Vertex(intPoint2,QVector3D(ed1W/7.5f+0.07f,0,0)));
						interPedX.push_back(Vertex(intPoint2-ed1Dir*3.5f,QVector3D(ed1W/7.5f+0.07f,1.0f,0)));
						interPedX.push_back(Vertex(intPoint1-ed1Dir*3.5f,QVector3D(0.0f-0.07f,1.0f,0)));

						// 停止線
						QVector3D midPoint=(intPoint2+intPoint1)/2.0f+0.2f*ed1Per;
					
						interPedXLineR.push_back(Vertex(intPoint1-ed1Dir*3.5f,QVector3D(0,0.0f,0)));
						interPedXLineR.push_back(Vertex(midPoint-ed1Dir*3.5f,QVector3D(1.0f,0.0f,0)));
						interPedXLineR.push_back(Vertex(midPoint-ed1Dir*4.25f,QVector3D(1.0f,1.0f,0)));
						interPedXLineR.push_back(Vertex(intPoint1-ed1Dir*4.25f,QVector3D(0.0f,1.0f,0)));
					}
				}

				// interPointsに基づいて、交差点のポリゴンを生成する
				std::vector<QVector3D> curvedInterPoints;
				for (int pi = 0; pi < interPoints.size() / 3; ++pi) {
					int next = (pi * 3 + 1) % interPoints.size();
					int prev = (pi * 3 - 1 + interPoints.size()) % interPoints.size();
					int nextnext = (pi * 3 + 2) % interPoints.size();

					std::vector<QVector3D> curvePoints = generateCurvePoints(interPoints[pi * 3], interPoints[prev], interPoints[next]);
					curvedInterPoints.insert(curvedInterPoints.end(), curvePoints.begin(), curvePoints.end());
					if ((interPoints[next] - stopPoints[pi * 2]).lengthSquared() > 0.1f) {
						curvedInterPoints.push_back(stopPoints[pi * 2]);
					}
					if ((interPoints[nextnext] - stopPoints[pi * 2 + 1]).lengthSquared() > 0.1f) {
						curvedInterPoints.push_back(stopPoints[pi * 2 + 1]);
					}
				}
								
				if (curvedInterPoints.size() > 2) {
					rendManager.addStaticGeometry2("3d_roads_interCom",curvedInterPoints,0.0f,false,"../data/textures/roads/road_0lines.jpg",GL_QUADS,2,QVector3D(1.0f/7.5f,1.0f/7.5f,1),QColor());//0.0f (moved before)
					//rendManager.addStaticGeometry2("3d_roads_interCom", curvedInterPoints, 0.0f, false, "", GL_QUADS, 1|mode_Lighting,QVector3D(), QColor(0, 0, 255));//0.0f (moved before)
				}
			}
		}//all vertex

		rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2);
		rendManager.addStaticGeometry("3d_roads_interCom",interPedX,"../data/textures/roads/road_pedX.jpg",GL_QUADS,2);
		rendManager.addStaticGeometry("3d_roads_interCom",interPedXLineR,"../data/textures/roads/road_pedXLineR.jpg",GL_QUADS,2);
	}
}

std::vector<QVector3D> RoadMeshGenerator::generateCurvePoints(const QVector3D& intPoint, const QVector3D& p1, const QVector3D& p2) {
	std::vector<QVector3D> points;

	// もしco-linearなら、p1-intPoint-p2を返却する
	if (fabs(QVector3D::dotProduct((p1 - intPoint).normalized(), (p2 - intPoint).normalized())) > 0.95f) {
		points.push_back(p1);
		points.push_back(intPoint);
		points.push_back(p2);
		return points;
	}

	QVector3D d1 = intPoint - p1;
	QVector3D per1 = QVector3D(-d1.y(), d1.x(), 0);
	QVector3D d2 = p2 - intPoint;
	QVector3D per2 = QVector3D(-d2.y(), d2.x(), 0);

	float tab, tcd;
	QVector3D center;
	if (!Util::segmentSegmentIntersectXY3D(p1, p1 + per1 * 100, p2, p2 + per2 * 100, &tab, &tcd, false, center)) return points;

	QVector2D v1(p1 - center);
	QVector2D v2(p2 - center);
	float r = v1.length();
	float theta1 = atan2f(v1.y(), v1.x());
	float theta2 = atan2f(v2.y(), v2.x());
	if (theta2 < theta1) theta2 += 2 * M_PI;

	points.push_back(p1);
	for (float theta = theta1 + 0.2f; theta < theta2; theta += 0.2f) {
		points.push_back(center + QVector3D(cosf(theta) * r, sinf(theta) * r, 0));
	}
	points.push_back(p2);

	return points;
}
