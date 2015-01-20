#include "RoadMeshGenerator.h"
#include "Util.h"
#include "GraphUtil.h"
#include "global.h"

bool compare2ndPartTuple2 (const std::pair<float, RoadEdgeDesc> &i, const std::pair<float, RoadEdgeDesc> &j) {
	return (i.first > j.first);
}

void RoadMeshGenerator::generateRoadMesh(VBORenderManager& rendManager, RoadGraph& roads) {
	float deltaZ = 2.0f;//G::global().getFloat("3d_road_deltaZ");
	const float deltaL = 1.0f;

	std::vector<Vertex> vertSide;

	//////////////////////////////////////////
	// POLYLINES
	{
		float const maxSegmentLeng=5.0f;//5.0f

		RoadEdgeIter ei, eiEnd;
		int numEdges=0;

		std::vector<Vertex> vertROAD[2];
		std::vector<Vertex> intersectCirclesV;
		for (boost::tie(ei, eiEnd) = boost::edges(roads.graph); ei != eiEnd; ++ei) {
			if (!roads.graph[*ei]->valid) continue;
			numEdges++;

			RoadEdgePtr edge = roads.graph[*ei];
			float hWidth = roads.graph[*ei]->getWidth() * 0.5f;
			//printf("roadGraph.graph[*ei]->type %d\n",roadGraph.graph[*ei]->type);
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

			//float lengthMoved=0;//road texture dX
			
			float lengthMovedL=0;//road texture dX
			float lengthMovedR=0;//road texture dX

			QVector2D a0,a1,a2,a3;

			for (int pL = 0; pL < edge->polyline.size() - 1; pL++) {
				bool bigAngle=false;
				QVector2D p0 = edge->polyline[pL];
				QVector2D p1 = edge->polyline[pL+1];
				if ((p0 - p1).lengthSquared() <= 0.00001f) continue;

				QVector2D dir = p1 -p0;
				float length = dir.length();
				dir /= length;
				
				QVector2D per(-dir.y(), dir.x());
				if (pL == 0) {
					a0 = p0 - per * hWidth;
					a3 = p0 + per * hWidth;
				}
				a1 = p1 - per * hWidth;
				a2 = p1 + per * hWidth;

				QVector2D p2;
				if (pL < edge->polyline.size() - 2) {
					p2 = edge->polyline[pL + 2];
					
					// もし隣接する２つのセグメントの方向ベクトルがほぼ平行なら、getIrregularBisector関数は失敗するので、
					// 実行しない。そもそもほぼ平行なので、この関数を実行する必要がない！！
					if (fabs(QVector2D::dotProduct((p2 - p1).normalized(), (p1 - p0).normalized())) < 0.99f) {
						Util::getIrregularBisector(p0, p1, p2, hWidth, hWidth, a2);
						Util::getIrregularBisector(p0, p1, p2, -hWidth, -hWidth, a1);
					}
				}
				
				float middLenghtR = length;
				float middLenghtL = length;
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

					QVector3D b03 = (b0 + b3) * 0.5f;
					QVector3D b12 = (b1 + b2) * 0.5f;
					float z1 = rendManager.getTerrainHeight(b03.x(), b03.y()) + deltaZ;
					float z2 = rendManager.getTerrainHeight(b12.x(), b12.y()) + deltaZ;

					b0.setZ(z1);
					b3.setZ(z1);
					b1.setZ(z2);
					b2.setZ(z2);

					vertROAD[type].push_back(Vertex(b0,QColor(),QVector3D(0,0,1.0f),QVector3D(1,lengthMovedR / dW,0)));
					vertROAD[type].push_back(Vertex(b1,QColor(),QVector3D(0,0,1.0f),QVector3D(1,(lengthMovedR + segmentLengR) / dW,0)));
					vertROAD[type].push_back(Vertex(b2,QColor(),QVector3D(0,0,1.0f),QVector3D(0,(lengthMovedL + segmentLengL) / dW,0)));
					vertROAD[type].push_back(Vertex(b3,QColor(),QVector3D(0,0,1.0f),QVector3D(0,lengthMovedL / dW,0)));

					// 側面のジオメトリ
					QVector3D b0_d = b0 + QVector3D(0, 0, -deltaL*2);
					QVector3D b1_d = b1 + QVector3D(0, 0, -deltaL*2);
					QVector3D b2_d = b2 + QVector3D(0, 0, -deltaL*2);
					QVector3D b3_d = b3 + QVector3D(0, 0, -deltaL*2);
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
		rendManager.addStaticGeometry("3d_roads", vertROAD[0], "../data/textures/roads/road_2lines.jpg", GL_QUADS, 2);
		rendManager.addStaticGeometry("3d_roads", vertROAD[1], "../data/textures/roads/road_4lines.jpg", GL_QUADS, 2);
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

				float z = rendManager.getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y()) + deltaZ - 0.1f;
				QVector3D center(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), z);

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

					// 側面のジオメトリ
					QVector3D side0_u = center + oP;
					QVector3D side0_b = center + oP + QVector3D(0, 0, -deltaZ * 2.0f);
					QVector3D side1_u = center + nP;
					QVector3D side1_b = center + nP + QVector3D(0, 0, -deltaZ * 2.0f);

					vertSide.push_back(Vertex(side0_b, QColor(64, 64, 64), oP, QVector3D()));
					vertSide.push_back(Vertex(side1_b, QColor(64, 64, 64), nP, QVector3D()));
					vertSide.push_back(Vertex(side1_u, QColor(64, 64, 64), nP, QVector3D()));
					vertSide.push_back(Vertex(side0_u, QColor(64, 64, 64), oP, QVector3D()));

					oP=nP;
				}

			}else{
				////////////////////////
				// 2.2 FOUR OR MORE--> COMPLEX INTERSECTION
				float z = rendManager.getTerrainHeight(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y()) + deltaZ + 0.1f;

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
				for (int eN = 0; eN < edgeAngleOut.size(); eN++) {
					//printf("** eN %d\n",eN);
					// a) ED1: this edge
					float ed1W = roads.graph[edgeAngleOut[eN].second]->getWidth();
					Polyline2D ed1poly = GraphUtil::orderPolyLine(roads, edgeAngleOut[eN].second, *vi, edgeAngleOut[eN].first);
					QVector2D ed1p1 = ed1poly[1];
					// compute right side
					QVector2D ed1Dir = (roads.graph[*vi]->pt - ed1p1).normalized();//ends in 0
					QVector2D ed1Per(ed1Dir.y(), -ed1Dir.x());// = (QVector3D::crossProduct(ed1Dir,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					QVector2D ed1p0R = roads.graph[*vi]->pt + ed1Per*ed1W/2.0f;
					QVector2D ed1p1R = ed1p1 + ed1Per*ed1W/2.0f;
					// compute left side
					QVector2D ed1p0L = roads.graph[*vi]->pt - ed1Per*ed1W/2.0f;
					QVector2D ed1p1L = ed1p1 - ed1Per*ed1W/2.0f;

					// b) ED2: next edge
					int lastEdge = eN - 1;
					if (lastEdge < 0) lastEdge = edgeAngleOut.size() - 1;
					float ed2WL = roads.graph[edgeAngleOut[lastEdge].second]->getWidth();
					QVector2D ed2p0L = roads.graph[*vi]->pt;
					Polyline2D ed2polyL = GraphUtil::orderPolyLine(roads, edgeAngleOut[lastEdge].second, *vi, edgeAngleOut[lastEdge].first);
					QVector2D ed2p1L = ed2polyL[1];
					// compute left side
					QVector2D ed2DirL = (ed2p0L - ed2p1L).normalized();//ends in 0
					QVector2D ed2PerL(ed2DirL.y(), -ed2DirL.x());//(QVector3D::crossProduct(ed2DirL,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed2p0L-=ed2PerL*ed2WL/2.0f;
					ed2p1L-=ed2PerL*ed2WL/2.0f;

					// c) ED2: last edge
					int nextEdge = (eN + 1) % edgeAngleOut.size();
					float ed2WR = roads.graph[edgeAngleOut[nextEdge].second]->getWidth();
					QVector2D ed2p0R = roads.graph[*vi]->pt;
					Polyline2D ed2polyR = GraphUtil::orderPolyLine(roads, edgeAngleOut[nextEdge].second, *vi, edgeAngleOut[nextEdge].first);
					QVector2D ed2p1R = ed2polyR[1];
					// compute left side
					QVector2D ed2DirR = (ed2p0R - ed2p1R).normalized();//ends in 0
					QVector2D ed2PerR(ed2DirR.y(), -ed2DirR.x());// = (QVector3D::crossProduct(ed2DirR,QVector3D(0,0,1.0f)).normalized());//need normalized()?
					ed2p0R+=ed2PerR*ed2WR/2.0f;
					ed2p1R+=ed2PerR*ed2WR/2.0f;

					//////////////////////////////////////////
					// d) Computer interior coordinates
					// d.1 computer intersection left
					QVector2D intPt1(FLT_MAX,0);
					if (fabs(QVector2D::dotProduct(ed1Dir, ed2DirL)) < 0.95f) { // ２つの隣接道路が平行じゃないなら、
						float tab,tcd;
						Util::segmentSegmentIntersectXY(ed1p0R, ed1p1R, ed2p0L, ed2p1L, &tab, &tcd, false, intPt1);
					} else { // ２つの隣接道路が平行なら、
						intPt1 = (ed1p0R + ed2p0L) * 0.5f;
					}
					// d.2 computer intersecion right
					QVector2D intPt2(FLT_MAX,0);
					if (fabs(QVector2D::dotProduct(ed1Dir, ed2DirR)) < 0.95f) {
						float tab,tcd;
						Util::segmentSegmentIntersectXY(ed1p0L, ed1p1L, ed2p0R, ed2p1R, &tab, &tcd, false, intPt2);
					} else {
						intPt2 = (ed1p0L + ed2p0R) * 0.5f;
					}

					QVector3D intPoint1(intPt1.x(), intPt1.y(), z);
					QVector3D intPoint2(intPt2.x(), intPt2.y(), z);

					//intPoint1 -= ed1Dir * deltaL;
					//intPoint2 -= ed1Dir * deltaL;

					interPoints.push_back(intPoint1);
					// intPoint1、intPoint2を、道路の方向に直行するよう、位置をそろえる
					if (QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir) >= 0) {
						intPoint1 -= ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
						interPoints.push_back(intPoint1);
					} else {
						intPoint2 += ed1Dir * QVector3D::dotProduct(intPoint1 - intPoint2, ed1Dir);
						interPoints.push_back(intPoint2);
					}


					stopPoints.push_back(intPoint1);
					stopPoints.push_back(intPoint2);

					// create crossing and stop line only if the degree >= 3 and the edge is not self-looping and the edge length is long enough
					if (outDegree >= 3 && roads.graph[edgeAngleOut[eN].second]->type == RoadEdge::TYPE_AVENUE && (ed1poly[0] - ed1poly.back()).length() > 10.0f && ed1poly.length() > 50.0f) {
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
				/*std::vector<QVector3D> curvedInterPoints;
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
				}*/
								
				if (interPoints.size() > 2) {
					rendManager.addStaticGeometry2("3d_roads_inter",interPoints,0.0f,false,"../data/textures/roads/road_0lines.jpg",GL_QUADS,2,QVector3D(1.0f/7.5f,1.0f/7.5f,1),QColor());//0.0f (moved before)
				}
			}
		}

		rendManager.addStaticGeometry("3d_roads_inter",intersectCirclesV,"../data/textures/roads/road_0lines.jpg",GL_TRIANGLES,2);
		rendManager.addStaticGeometry("3d_roads_inter",interPedX,"../data/textures/roads/road_pedX.jpg",GL_QUADS,2);
		rendManager.addStaticGeometry("3d_roads_inter",interPedXLineR,"../data/textures/roads/road_pedXLineR.jpg",GL_QUADS,2);
	}

	rendManager.addStaticGeometry("3d_roads", vertSide, "", GL_QUADS, 1|mode_Lighting);
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

void RoadMeshGenerator::generate2DRoadMesh(VBORenderManager& renderManager, RoadGraph& roads) {
	float deltaZ = 2.0f;

	//////////////////////////////////////
	// EDGES
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			int num = roads.graph[*ei]->polyline.size();
			if (num <= 1) continue;

			float halfWidth = roads.graph[*ei]->getWidth()*0.5f;//it should not have /2.0f (but compensated below)
			

			std::vector<Vertex> vert(4*(num - 1));
			std::vector<Vertex> vertBg(4*(num - 1));
			
			// Type
			QColor color;// = graph[*ei]->color;
			QColor colorO;
			float heightOffset = 0.0f;
			float heightOffsetO=0.0f;

			switch (roads.graph[*ei]->type) {
			case RoadEdge::TYPE_HIGHWAY:
				heightOffset = 0.8f;
				heightOffsetO = 0.3f;
				color=QColor(0xfa,0x9e,0x25);
				colorO=QColor(0x00, 0x00, 0x00);//QColor(0xdf,0x9c,0x13);
				break;
			case RoadEdge::TYPE_BOULEVARD:
				heightOffset = 0.5f;
				heightOffsetO = 0.2f;
				color=QColor(0xff,0xe1,0x68);
				colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
				//halfWidth*=1.4f;
				break;
			case RoadEdge::TYPE_AVENUE:
				heightOffset = 0.6f;
				heightOffsetO = 0.1f;
				color=QColor(0xff,0xe1,0x68);
				colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
				break;
			case RoadEdge::TYPE_STREET:
				heightOffset = 0.4f;
				heightOffsetO = 0.1f;
				color=QColor(0xff,0xff,0xff);
				colorO=QColor(0x00, 0x00, 0x00);//QColor(0xd7,0xd1,0xc7);
				break;
			}

			heightOffset+=0.45f;//to have park below
			heightOffsetO+=0.45f;//to have park below

			float halfWidthBg = halfWidth + G::global().getFloat("2DroadsStroke");//it should not depend on the type 3.5f

			QVector2D p0, p1, p2, p3;
			QVector2D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector2D pt1 = roads.graph[*ei]->polyline[i];
				QVector2D pt2 = roads.graph[*ei]->polyline[i + 1];

				QVector2D perp = pt2 - pt1;
				perp = QVector2D(-perp.y(), perp.x());
				perp.normalize();

				if (i == 0) {
					p0 = pt1 + perp * halfWidth;
					p1 = pt1 - perp * halfWidth;
					p0Bg = pt1 + perp * halfWidthBg;
					p1Bg = pt1 - perp * halfWidthBg;
				}
				p2 = pt2 - perp * halfWidth;
				p3 = pt2 + perp * halfWidth;
				p2Bg = pt2 - perp * halfWidthBg;
				p3Bg = pt2 + perp * halfWidthBg;
				QVector3D normal(0, 0, 1);// = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector2D pt3 = roads.graph[*ei]->polyline[i + 2];

					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

				vert[i*4+0]=Vertex(p0.x(),p0.y(),deltaZ+heightOffset,color,0,0,1.0f,0,0,0);// pos color normal texture
				vert[i*4+1]=Vertex(p1.x(),p1.y(),deltaZ+heightOffset,color,0,0,1.0f,0,0,0);// pos color normal texture
				vert[i*4+2]=Vertex(p2.x(),p2.y(),deltaZ+heightOffset,color,0,0,1.0f,0,0,0);// pos color normal texture
				vert[i*4+3]=Vertex(p3.x(),p3.y(),deltaZ+heightOffset,color,0,0,1.0f,0,0,0);// pos color normal texture
					
				vertBg[i*4+0]=Vertex(p0Bg.x(),p0Bg.y(),deltaZ+heightOffsetO,colorO,0,0,1.0f,0,0,0);// pos color normal texture
				vertBg[i*4+1]=Vertex(p1Bg.x(),p1Bg.y(),deltaZ+heightOffsetO,colorO,0,0,1.0f,0,0,0);// pos color normal texture
				vertBg[i*4+2]=Vertex(p2Bg.x(),p2Bg.y(),deltaZ+heightOffsetO,colorO,0,0,1.0f,0,0,0);// pos color normal texture
				vertBg[i*4+3]=Vertex(p3Bg.x(),p3Bg.y(),deltaZ+heightOffsetO,colorO,0,0,1.0f,0,0,0);// pos color normal texture
					

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}

			renderManager.addStaticGeometry("3d_roads", vert, "", GL_QUADS, 1);//MODE=1 color
			renderManager.addStaticGeometry("3d_roads", vertBg, "", GL_QUADS, 1);//MODE=1 color
		}
	}

	/////////////////////////////////////////////////////
	// INTERSECTIONS
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
			if (!roads.graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			QColor color;// = graph[*ei]->color;
			QColor colorO;
			float heightOffset = 0.0f;
			float heightOffsetO=0.0f;
			int maxType=-1;
			float halfWidth;

			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, roads.graph); oei != oeend; ++oei) {
				if (!roads.graph[*oei]->valid) continue;
				//printf("type %d\n",graph[*oei]->type);
				if(maxType>roads.graph[*oei]->type)
					continue;
				maxType=roads.graph[*oei]->type;
				halfWidth=roads.graph[*oei]->getWidth()*0.5f;//it should not have /2.0f (but compensated below)

				switch (roads.graph[*oei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					heightOffset = 0.6f;
					heightOffsetO = 0.3f;
					color=QColor(0xfa,0x9e,0x25);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xdf,0x9c,0x13);
					continue;
				case RoadEdge::TYPE_BOULEVARD:
					heightOffset = 0.5f;
					heightOffsetO = 0.2f;
					color=QColor(0xff,0xe1,0x68);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
					continue;
				case RoadEdge::TYPE_AVENUE:
					heightOffset = 0.5f;
					heightOffsetO = 0.2f;
					color=QColor(0xff,0xe1,0x68);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xe5,0xbd,0x4d);
					continue;
				case RoadEdge::TYPE_STREET:
					heightOffset = 0.4f;
					heightOffsetO = 0.2f;
					color=QColor(0xff,0xff,0xff);
					colorO=QColor(0x00, 0x00, 0x00);//QColor(0xd7,0xd1,0xc7);
					continue;
				}
			}

			heightOffset+=0.45f;//to have park below
			heightOffsetO+=0.45f;//to have park below

			float max_r=halfWidth;
			float max_rO=halfWidth + G::global().getFloat("2DroadsStroke");//it should not depend on the type 3.5f

			std::vector<Vertex> vert(3*20);
			std::vector<Vertex> vertBg(3*20);

			for (int i = 0; i < 20; ++i) {
				float angle1 = 2.0 * M_PI * i / 20.0f;
				float angle2 = 2.0 * M_PI * (i + 1) / 20.0f;

				vert[i*3+0]=Vertex(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), deltaZ + heightOffset, color, 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+1]=Vertex(roads.graph[*vi]->pt.x() + max_r * cosf(angle1), roads.graph[*vi]->pt.y() + max_r * sinf(angle1), deltaZ + heightOffset, color, 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+2]=Vertex(roads.graph[*vi]->pt.x() + max_r * cosf(angle2), roads.graph[*vi]->pt.y() + max_r * sinf(angle2), deltaZ + heightOffset, color, 0, 0, 1.0f, 0, 0, 0);

				vertBg[i*3+0]=Vertex(roads.graph[*vi]->pt.x(), roads.graph[*vi]->pt.y(), deltaZ + heightOffsetO, colorO, 0, 0, 1.0f, 0, 0, 0);
				vertBg[i*3+1]=Vertex(roads.graph[*vi]->pt.x() + max_rO * cosf(angle1), roads.graph[*vi]->pt.y() + max_rO * sinf(angle1), deltaZ + heightOffsetO, colorO, 0, 0, 1.0f, 0, 0, 0);
				vertBg[i*3+2]=Vertex(roads.graph[*vi]->pt.x() + max_rO * cosf(angle2), roads.graph[*vi]->pt.y() + max_rO * sinf(angle2), deltaZ + heightOffsetO, colorO, 0, 0, 1.0f, 0, 0, 0);
			}
						
			renderManager.addStaticGeometry("3d_roads_inter", vert, "", GL_TRIANGLES, 1);//MODE=1 color
			renderManager.addStaticGeometry("3d_roads_inter", vertBg, "", GL_TRIANGLES, 1);//MODE=1 color
		}
	}
}