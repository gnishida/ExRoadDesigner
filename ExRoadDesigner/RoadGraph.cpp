#include "RoadGraph.h"
#include <QGLWidget>
#include "GraphUtil.h"
#include "Util.h"
/*
#include "../render/Terrain.h"
#include "../render/RenderableCircleList.h"
#include "../render/RenderableCylinderList.h"
#include "../render/TextureManager.h"
*/

RoadGraph::RoadGraph() {
	showHighways = true;
	showBoulevards = true;
	showAvenues = true;
	showLocalStreets = true;

	renderMode = RENDER_DEFAULT;
	//renderMode = RENDER_GROUPBY;
}

RoadGraph::~RoadGraph() {
}

void RoadGraph::generateMesh(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN) {
	if (!modified) return;

	renderManager.removeStaticGeometry(linesN);
	renderManager.removeStaticGeometry(pointsN);

	if (renderMode == RENDER_DEFAULT) {
		_generateMeshVerticesDefault(renderManager, linesN, pointsN);
	} else if (renderMode == RENDER_GROUPBY) {
		_generateMeshVerticesGroupBy(renderManager, linesN, pointsN);
	}

	modified = false;
}

/*
void RoadGraph::_generateMeshVertices(mylib::TextureManager* textureManager) {
	if (renderMode == RENDER_DEFAULT) {
		_generateMeshVerticesDefault(textureManager);
	} else if (renderMode == RENDER_TEXTURE) {
		_generateMeshVerticesTexture(textureManager);
	} else if (renderMode == RENDER_GROUPBY) {
		_generateMeshVerticesGroupBy(textureManager);
	} else if (renderMode == RENDER_GENERATION_TYPE) {
		_generateMeshVerticesGenerationType(textureManager);
	}
}

void RoadGraph::_generateMeshVerticesTexture(mylib::TextureManager* textureManager) {
	mylib::RenderableQuadList* renderable1 = new mylib::RenderableQuadList(textureManager->get("data/textures/street_segment.jpg"));
	mylib::RenderableQuadList* renderable2 = new mylib::RenderableQuadList(textureManager->get("data/textures/avenue_segment.jpg"));
	mylib::RenderableCircleList* renderable3 = new mylib::RenderableCircleList(textureManager->get("data/textures/street_intersection.jpg"));
	mylib::RenderableCylinderList* renderable4 = new mylib::RenderableCylinderList(textureManager->get("data/textures/bridge.jpg"));

	// エッジを描画
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			float halfWidth = graph[*ei]->getWidth() / 2.0f;

			// draw the bridge
			float bridgeHeight = 0.0f;
			if (graph[*ei]->properties.contains("bridgeHeight")) {
				bridgeHeight = graph[*ei]->properties["bridgeHeight"].toFloat();
			}

			QVector3D p0, p1, p2, p3;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D vec = pt2 - pt1;
				vec = QVector3D(-vec.y(), vec.x(), 0.0f);
				vec.normalize();

				if (i == 0) {
					p0 = pt1 + vec * halfWidth;
					p1 = pt1 - vec * halfWidth;
				}
				p2 = pt2 - vec * halfWidth;
				p3 = pt2 + vec * halfWidth;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];


					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
				}

				switch (graph[*ei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					if (showHighways) {
						renderable2->addQuad(p0, p1, p2, p3, normal, 0, 1, 0, (pt1 - pt2).length() / 10.0f, 0.3f);

						// draw a bridge
						if (bridgeHeight > 5.0f) {
							renderable4->addCylinder(pt1.x(), pt1.y(), -20, 1.5f, 1.5f, pt1.z() + 20, 10, 10);
						}
					}
					break;
				case RoadEdge::TYPE_BOULEVARD:
					if (showBoulevards) {
						renderable2->addQuad(p0, p1, p2, p3, normal, 0, 1, 0, (pt1 - pt2).length() / 10.0f, 0.2f);

						// draw a bridge
						if (bridgeHeight > 5.0f) {
							renderable4->addCylinder(pt1.x(), pt1.y(), -20, 1.5f, 1.5f, pt1.z() + 20, 10, 10);
						}
					}
					break;
				case RoadEdge::TYPE_AVENUE:
					if (showAvenues) {
						renderable2->addQuad(p0, p1, p2, p3, normal, 0, 1, 0, (pt1 - pt2).length() / 10.0f, 0.1f);

						// draw a bridge
						if (bridgeHeight > 5.0f) {
							renderable4->addCylinder(pt1.x(), pt1.y(), -20, 1.5f, 1.5f, pt1.z() + 20, 10, 10);
						}
					}
					break;
				case RoadEdge::TYPE_STREET:
					if (showLocalStreets) {
						renderable1->addQuad(p0, p1, p2, p3, normal, 0, 1, 0, (pt1 - pt2).length() / 10.0f);

						// draw a bridge
						if (bridgeHeight > 5.0f) {
							renderable4->addCylinder(pt1.x(), pt1.y(), -20, 1.5f, 1.5f, pt1.z() + 20, 10, 10);
						}
					}
					break;
				}

				p0 = p3;
				p1 = p2;
			}

			// draw a bridge
			if (bridgeHeight > 5.0f) {
				renderable4->addCylinder(graph[*ei]->polyline3D.last().x(), graph[*ei]->polyline3D.last().y(), -20, 1.5f, 1.5f, graph[*ei]->polyline3D.last().z() + 20, 10, 10);
			}

		}
	}

	// draw intersections
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_width = 0;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;

				float width = graph[*oei]->getWidth();
				if (width > max_width) {
					max_width = width;
				}
			}

			renderable3->addCircle(graph[*vi]->pt3D, max_width * 0.5f, 20, -0.1f);
		}
	}

	renderables.push_back(mylib::RenderablePtr(renderable1));
	renderables.push_back(mylib::RenderablePtr(renderable2));
	renderables.push_back(mylib::RenderablePtr(renderable3));
	renderables.push_back(mylib::RenderablePtr(renderable4));
}
*/

void RoadGraph::_generateMeshVerticesDefault(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN) {
	// エッジを描画
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			float halfWidth = graph[*ei]->getWidth() / 2.0f;
			float halfWidthBg = halfWidth * 1.2f;

			std::vector<Vertex> vert(4*(num - 1));
			QColor color = graph[*ei]->color;

			QVector3D p0, p1, p2, p3;
			QVector3D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D vec = pt2 - pt1;
				vec = QVector3D(-vec.y(), vec.x(), 0.0f);
				vec.normalize();

				if (i == 0) {
					p0 = pt1 + vec * halfWidth;
					p1 = pt1 - vec * halfWidth;
					p0Bg = pt1 + vec * halfWidthBg;
					p1Bg = pt1 - vec * halfWidthBg;
				}
				p2 = pt2 - vec * halfWidth;
				p3 = pt2 + vec * halfWidth;
				p2Bg = pt2 - vec * halfWidthBg;
				p3Bg = pt2 + vec * halfWidthBg;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];
					
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

				float heightOffset = 0.0f;
				bool render = false;

				switch (graph[*ei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					if (showHighways) {
						render = true;
						heightOffset = 0.7f;
					}
					break;
				case RoadEdge::TYPE_BOULEVARD:
					if (showBoulevards) {
						render = true;
						heightOffset = 0.5f;
					}
					break;
				case RoadEdge::TYPE_AVENUE:
					if (showAvenues) {
						render = true;
						heightOffset = 0.5f;
					}
					break;
				case RoadEdge::TYPE_STREET:
					if (showLocalStreets) {
						render = true;
						heightOffset = 0.3f;
					}
					break;
				}

				if (render) {
					vert[i*4+0]=Vertex(p0.x(),p0.y(),p0.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+1]=Vertex(p1.x(),p1.y(),p1.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+2]=Vertex(p2.x(),p2.y(),p2.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+3]=Vertex(p3.x(),p3.y(),p3.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
				}

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}

			renderManager.addStaticGeometry(linesN, vert, "", GL_QUADS, 1);//MODE=1 color
		}
	}

	// draw intersections
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_r = 0;
			int max_roadType = 0;
			QColor color, bgColor;
			float offset = 0.3f;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;

				float r = graph[*oei]->getWidth() * 0.5f;
				if (r > max_r) {
					max_r = r;
				}

				if (graph[*oei]->type > max_roadType) {
					max_roadType = graph[*oei]->type;
					color = graph[*oei]->color;
					bgColor = graph[*oei]->bgColor;
				}
			}

			float max_rBg = max_r * 1.2f;

			if (max_roadType == RoadEdge::TYPE_AVENUE) {
				offset = 0.5f;
			}

			std::vector<Vertex> vert(3*20);

			for (int i = 0; i < 20; ++i) {
				float angle1 = 2.0 * M_PI * i / 20.0f;
				float angle2 = 2.0 * M_PI * (i + 1) / 20.0f;

				vert[i*3+0]=Vertex(graph[*vi]->pt3D.x(), graph[*vi]->pt3D.y(), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+1]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle1), graph[*vi]->pt3D.y() + max_r * sinf(angle1), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+2]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle2), graph[*vi]->pt3D.y() + max_r * sinf(angle2), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				/*
				generateMeshVertex(o.x(), o.y(), o.z(), 0, 0, 1, color);
				generateMeshVertex(o.x() + r * cos(angle1), o.y() + r * sin(angle1), o.z(), 0, 0, 1, color);
				generateMeshVertex(o.x() + r * cos(angle2), o.y() + r * sin(angle2), o.z(), 0, 0, 1, color);
				*/
			}

			renderManager.addStaticGeometry(pointsN, vert, "", GL_TRIANGLES, 1);//MODE=1 color
			//renderable2->addCircle(graph[*vi]->pt3D, max_width * 0.5f, 20, color, offset);
			//renderable2->addCircle(graph[*vi]->pt3D, max_widthBg * 0.5f, 20, bgColor);
		}
	}
}

void RoadGraph::_generateMeshVerticesGroupBy(VBORenderManager& renderManager, const QString &linesN, const QString &pointsN) {
	// エッジを描画
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			QColor color, bgColor;							
			bool mixed = graph[*ei]->properties["generation_type"] == "mixed" ? true : false;
			if (graph[*ei]->properties["generation_type"] == "example" || mixed) {
				// group_idに基づいて色を決定
				if (graph[*ei]->properties["group_id"].toInt() == 0) {
					color = QColor(255, 0, 0);
					bgColor = QColor(128, 0, 0);
				} else if (graph[*ei]->properties["group_id"].toInt() == 1) {
					color = QColor(0, 0, 255);
					bgColor = QColor(0, 0, 128);
				} else if (graph[*ei]->properties["group_id"].toInt() == 2) {
					color = QColor(0, 196, 0);
					bgColor = QColor(0, 64, 0);
				} else if (graph[*ei]->properties["group_id"].toInt() == 3) {
					color = QColor(255, 0, 255);
					bgColor = QColor(128, 0, 128);
				} else if (graph[*ei]->properties["group_id"].toInt() == 4) {
					color = QColor(128, 255, 255);
					bgColor = QColor(0, 128, 128);
				} else if (graph[*ei]->properties["group_id"].toInt() == 5) {
					color = QColor(255, 0, 0);
					bgColor = QColor(128, 0, 0);
				} else if (graph[*ei]->properties["group_id"].toInt() == 6) {
					color = QColor(0, 0, 255);
					bgColor = QColor(0, 0, 128);
				} else if (graph[*ei]->properties["group_id"].toInt() == 7) {
					color = QColor(0, 196, 0);
					bgColor = QColor(0, 64, 0);
				} else {
					color = QColor(255, 255, 255);
					bgColor = QColor(128, 128, 128);
				}

				if (mixed) {
					color = color.lighter(150);
					bgColor = bgColor.lighter(150);
				}
			} else {
				// PMベースは、白色
				color = QColor(255, 255, 255);
				bgColor = QColor(128, 128, 128);
			}





			float halfWidth = graph[*ei]->getWidth() / 2.0f;
			float halfWidthBg = halfWidth * 1.2f;

			std::vector<Vertex> vert(4*(num - 1));

			QVector3D p0, p1, p2, p3;
			QVector3D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D vec = pt2 - pt1;
				vec = QVector3D(-vec.y(), vec.x(), 0.0f);
				vec.normalize();

				if (i == 0) {
					p0 = pt1 + vec * halfWidth;
					p1 = pt1 - vec * halfWidth;
					p0Bg = pt1 + vec * halfWidthBg;
					p1Bg = pt1 - vec * halfWidthBg;
				}
				p2 = pt2 - vec * halfWidth;
				p3 = pt2 + vec * halfWidth;
				p2Bg = pt2 - vec * halfWidthBg;
				p3Bg = pt2 + vec * halfWidthBg;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];

					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

				float heightOffset = 0.0f;
				bool render = false;

				switch (graph[*ei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					if (showHighways) {
						render = true;
						heightOffset = 0.7f;
						//renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.7f);
						//renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_BOULEVARD:
					if (showBoulevards) {
						render = true;
						heightOffset = 0.5f;
						//renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.5f);
						//renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_AVENUE:
					if (showAvenues) {
						render = true;
						heightOffset = 0.5f;
						//renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.5f);
						//renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_STREET:
					if (showLocalStreets) {
						render = true;
						heightOffset = 0.3f;
						//renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.3f);
						//renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				}

				if (render) {
					vert[i*4+0]=Vertex(p0.x(),p0.y(),p0.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+1]=Vertex(p1.x(),p1.y(),p1.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+2]=Vertex(p2.x(),p2.y(),p2.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
					vert[i*4+3]=Vertex(p3.x(),p3.y(),p3.z()+heightOffset,color.redF(),color.greenF(),color.blueF(),0,0,1.0f,0,0,0);// pos color normal texture
				}

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}

			renderManager.addStaticGeometry(linesN, vert, "", GL_QUADS, 1);//MODE=1 color
		}
	}

	// draw intersections
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_r = 0;
			int max_roadType = 0;
			float offset = 0.3f;
			int group_id = -1;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;

				float r = graph[*oei]->getWidth();
				if (r > max_r) {
					max_r = r;
				}

				if (graph[*oei]->type > max_roadType) {
					max_roadType = graph[*oei]->type;
				}

				if (graph[*oei]->properties.contains("group_id")) {
					group_id = graph[*oei]->properties["group_id"].toInt();
				}
			}

			float max_rBg = max_r * 1.2f;

			if (max_roadType == RoadEdge::TYPE_AVENUE) {
				offset = 0.5f;
			}
		
			// group_idに基づいて色を決定
			QColor color, bgColor;
			if (graph[*vi]->properties["generation_type"] == "example") {
				if (group_id == 0) {
					color = QColor(255, 128, 128);
					bgColor = QColor(128, 0, 0);
				} else if (group_id == 1) {
					color = QColor(128, 255, 128);
					bgColor = QColor(0, 128, 0);
				} else if (group_id == 2) {
					color = QColor(128, 128, 255);
					bgColor = QColor(0, 0, 128);
				} else {
					color = QColor(255, 255, 255);
					bgColor = QColor(128, 128, 128);
				}
			} else {
				color = QColor(255, 255, 255);
				bgColor = QColor(128, 128, 128);
			}

			std::vector<Vertex> vert(3*20);

			for (int i = 0; i < 20; ++i) {
				float angle1 = 2.0 * M_PI * i / 20.0f;
				float angle2 = 2.0 * M_PI * (i + 1) / 20.0f;

				vert[i*3+1]=Vertex(graph[*vi]->pt3D.x(), graph[*vi]->pt3D.y(), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+2]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle1), graph[*vi]->pt3D.y() + max_r * sinf(angle1), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				vert[i*3+3]=Vertex(graph[*vi]->pt3D.x() + max_r * cosf(angle2), graph[*vi]->pt3D.y() + max_r * sinf(angle2), graph[*vi]->pt3D.z() + offset, color.redF(), color.greenF(), color.blueF(), 0, 0, 1.0f, 0, 0, 0);
				/*
				generateMeshVertex(o.x(), o.y(), o.z(), 0, 0, 1, color);
				generateMeshVertex(o.x() + r * cos(angle1), o.y() + r * sin(angle1), o.z(), 0, 0, 1, color);
				generateMeshVertex(o.x() + r * cos(angle2), o.y() + r * sin(angle2), o.z(), 0, 0, 1, color);
				*/
			}

			renderManager.addStaticGeometry(pointsN, vert, "", GL_TRIANGLES, 1);//MODE=1 color
			//renderable2->addCircle(graph[*vi]->pt3D, max_width * 0.5f, 20, color, offset);
			//renderable2->addCircle(graph[*vi]->pt3D, max_widthBg * 0.5f, 20, bgColor);
		}
	}
}

/*
void RoadGraph::_generateMeshVerticesGenerationType(mylib::TextureManager* textureManager) {
	mylib::RenderableQuadList* renderable1 = new mylib::RenderableQuadList();
	mylib::RenderableCircleList* renderable2 = new mylib::RenderableCircleList();

	// エッジを描画
	{
		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
			if (!graph[*ei]->valid) continue;

			int num = graph[*ei]->polyline3D.size();
			if (num <= 1) continue;

			// generation_typeに基づいて色を決定
			QColor color, bgColor;							
			if (graph[*ei]->properties["generation_type"] == "example") {
				color = QColor(255, 128, 128);
				bgColor = QColor(128, 0, 0);
			} else {
				color = QColor(255, 255, 255);
				bgColor = QColor(128, 128, 128);
			}

			float halfWidth = graph[*ei]->getWidth() / 2.0f;
			float halfWidthBg = halfWidth * 1.2f;

			QVector3D p0, p1, p2, p3;
			QVector3D p0Bg, p1Bg, p2Bg, p3Bg;
			for (int i = 0; i < num - 1; ++i) {
				QVector3D pt1 = graph[*ei]->polyline3D[i];
				QVector3D pt2 = graph[*ei]->polyline3D[i + 1];

				QVector3D vec = pt2 - pt1;
				vec = QVector3D(-vec.y(), vec.x(), 0.0f);
				vec.normalize();

				if (i == 0) {
					p0 = pt1 + vec * halfWidth;
					p1 = pt1 - vec * halfWidth;
					p0Bg = pt1 + vec * halfWidthBg;
					p1Bg = pt1 - vec * halfWidthBg;
				}
				p2 = pt2 - vec * halfWidth;
				p3 = pt2 + vec * halfWidth;
				p2Bg = pt2 - vec * halfWidthBg;
				p3Bg = pt2 + vec * halfWidthBg;
				QVector3D normal = Util::calculateNormal(p0, p1, p2);

				if (i < num - 2) {
					QVector3D pt3 = graph[*ei]->polyline3D[i + 2];

					Util::getIrregularBisector(pt1, pt2, pt3, halfWidth, halfWidth, p3);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidth, -halfWidth, p2);
					Util::getIrregularBisector(pt1, pt2, pt3, halfWidthBg, halfWidthBg, p3Bg);
					Util::getIrregularBisector(pt1, pt2, pt3, -halfWidthBg, -halfWidthBg, p2Bg);
				}

				switch (graph[*ei]->type) {
				case RoadEdge::TYPE_HIGHWAY:
					if (showHighways) {
						renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.7f);
						renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_BOULEVARD:
					if (showBoulevards) {
						renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.5f);
						renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_AVENUE:
					if (showAvenues) {
						renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.5f);
						renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				case RoadEdge::TYPE_STREET:
					if (showLocalStreets) {
						renderable1->addQuad(p0, p1, p2, p3, QVector3D(0, 0, 1), color, 0.3f);
						renderable1->addQuad(p0Bg, p1Bg, p2Bg, p3Bg, QVector3D(0, 0, 1), bgColor);
					}
					break;
				}

				p0 = p3;
				p1 = p2;
				p0Bg = p3Bg;
				p1Bg = p2Bg;
			}
		}
	}

	// draw intersections
	{
		RoadVertexIter vi, vend;
		for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
			if (!graph[*vi]->valid) continue;

			// get the largest width of the outing edges
			float max_width = 0;
			int max_roadType = 0;
			float offset = 0.3f;
			int group_id = -1;
			RoadOutEdgeIter oei, oeend;
			for (boost::tie(oei, oeend) = boost::out_edges(*vi, graph); oei != oeend; ++oei) {
				if (!graph[*oei]->valid) continue;

				float width = graph[*oei]->getWidth();
				if (width > max_width) {
					max_width = width;
				}

				if (graph[*oei]->type > max_roadType) {
					max_roadType = graph[*oei]->type;
				}

				if (graph[*oei]->properties.contains("group_id")) {
					group_id = graph[*oei]->properties["group_id"].toInt();
				}
			}

			float max_widthBg = max_width * 1.2f;

			if (max_roadType == RoadEdge::TYPE_AVENUE) {
				offset = 0.5f;
			}
		
			// group_idに基づいて色を決定
			QColor color, bgColor;
			if (graph[*vi]->properties["generation_type"] == "example") {
				if (group_id == 0) {
					color = QColor(255, 128, 128);
					bgColor = QColor(128, 0, 0);
				} else if (group_id == 1) {
					color = QColor(128, 255, 128);
					bgColor = QColor(0, 128, 0);
				} else if (group_id == 2) {
					color = QColor(128, 128, 255);
					bgColor = QColor(0, 0, 128);
				} else {
					color = QColor(255, 255, 255);
					bgColor = QColor(128, 128, 128);
				}
			} else {
				color = QColor(255, 255, 255);
				bgColor = QColor(128, 128, 128);
			}

			renderable2->addCircle(graph[*vi]->pt3D, max_width * 0.5f, 20, color, offset);
			renderable2->addCircle(graph[*vi]->pt3D, max_widthBg * 0.5f, 20, bgColor);
		}
	}

	renderables.push_back(mylib::RenderablePtr(renderable1));
	renderables.push_back(mylib::RenderablePtr(renderable2));
}
*/

/**
 * Add a mesh for the specified edge.
 */
/*void RoadGraph::add3DMeshOfEdge(mylib::RenderableQuadList* renderable, RoadEdgePtr edge, float width, QColor color, float heightOffset) {
	int num = edge->polyline.size();

	QVector3D p0, p1, p2, p3;
	for (int i = 0; i < num - 1; ++i) {
		QVector3D pt1 = edge->polyline3D[i];
		QVector3D pt2 = edge->polyline3D[i + 1];
		QVector3D vec = pt2 - pt1;
		vec = QVector3D(-vec.y(), vec.x(), 0.0f);
		vec.normalize();

		if (i == 0) {
			p0 = pt1 + vec * width * 0.5f;
			p1 = pt1 - vec * width * 0.5f;
		}
		p2 = pt2 - vec * width * 0.5f;
		p3 = pt2 + vec * width * 0.5f;
		QVector3D normal(0, 0, 1);// = Util::calculateNormal(p0, p1, p2);

		if (i < num - 2) {
			QVector3D pt3 = edge->polyline3D[i + 2];

			Util::getIrregularBisector(pt1, pt2, pt3, width * 0.5f, width * 0.5f, p3);
			Util::getIrregularBisector(pt1, pt2, pt3, -width * 0.5f, -width * 0.5f, p2);
		}

		renderable->addQuad(p0, p1, p2, p3, normal, color, heightOffset);

		p0 = p3;
		p1 = p2;
	}
}*/

/*
void RoadGraph::generate2DMesh() {
	if (!modified) return;

	renderables.clear();

	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(GL_TRIANGLES)));
	renderables.push_back(mylib::RenderablePtr(new mylib::Renderable(GL_POINTS, 10.0f)));

	// road edge
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		if (!graph[*ei]->valid) continue;

		RoadEdgePtr edge = graph[*ei];

		float height;
		switch (edge->type) {
		case RoadEdge::TYPE_HIGHWAY:
			height = highwayHeight;
			break;
		case RoadEdge::TYPE_BOULEVARD:
			height = avenueHeight;
			break;
		case RoadEdge::TYPE_AVENUE:
			height = avenueHeight;
			break;
		case RoadEdge::TYPE_STREET:
			height = avenueHeight;
			break;
		}

		QColor color = graph[*ei]->color;
		QColor bgColor = graph[*ei]->bgColor;

		// draw the border of the road segment
		if ((showHighways && edge->type == RoadEdge::TYPE_HIGHWAY) || (showBoulevards && edge->type ==  RoadEdge::TYPE_BOULEVARD) || (showAvenues && edge->type ==  RoadEdge::TYPE_AVENUE) || (showLocalStreets && edge->type ==  RoadEdge::TYPE_STREET)) {
			add2DMeshOfEdge(renderables[0], edge, widthBase * (1.0f + curbRatio), bgColor, height * 0.5f);
			add2DMeshOfEdge(renderables[0], edge, widthBase, color, height);
		}
	}

	// road vertex
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		if (!graph[*vi]->valid) continue;

		if (graph[*vi]->seed) {
			add2DMeshOfVertex(renderables[1], graph[*vi], QColor(0, 0, 255), 2.0f);
		}
	}

	modified = false;
}
*/

/**
 * Add a mesh for the specified edge.
 */
/*
void RoadGraph::add2DMeshOfEdge(mylib::RenderablePtr renderable, RoadEdgePtr edge, float widthBase, QColor color, float height) {
	mylib::Vertex v;

	// define the width of the road segment
	float width;
	switch (edge->type) {
	case RoadEdge::TYPE_HIGHWAY:
		width = widthBase * 2.0f;
		break;
	case RoadEdge::TYPE_BOULEVARD:
	case RoadEdge::TYPE_AVENUE:
		width = widthBase * 1.5f;
		break;
	case RoadEdge::TYPE_STREET:
		width = widthBase * 1.0f;
		break;
	}

	int num = edge->polyline.size();

	// draw the edge
	for (int i = 0; i < num - 1; ++i) {
		QVector2D pt1 = edge->polyline[i];
		QVector2D pt2 = edge->polyline[i + 1];
		QVector2D vec = pt2 - pt1;
		vec = QVector2D(-vec.y(), vec.x());
		vec.normalize();

		QVector2D p0 = pt1 + vec * width * 0.5f;
		QVector2D p1 = pt1 - vec * width * 0.5f;
		QVector2D p2 = pt2 - vec * width * 0.5f;
		QVector2D p3 = pt2 + vec * width * 0.5f;

		v.color[0] = color.redF();
		v.color[1] = color.greenF();
		v.color[2] = color.blueF();
		v.color[3] = color.alphaF();
		v.normal[0] = 0.0f;
		v.normal[1] = 0.0f;
		v.normal[2] = 1.0f;

		v.location[2] = height;

		v.location[0] = p0.x();
		v.location[1] = p0.y();
		renderable->vertices.push_back(v);

		v.location[0] = p1.x();
		v.location[1] = p1.y();
		renderable->vertices.push_back(v);

		v.location[0] = p2.x();
		v.location[1] = p2.y();
		renderable->vertices.push_back(v);

		v.location[0] = p0.x();
		v.location[1] = p0.y();
		renderable->vertices.push_back(v);

		v.location[0] = p2.x();
		v.location[1] = p2.y();
		renderable->vertices.push_back(v);

		v.location[0] = p3.x();
		v.location[1] = p3.y();
		renderable->vertices.push_back(v);
	}
}
*/

/**
 * Add a mesh for the specified edge.
 */
/*
void RoadGraph::add2DMeshOfVertex(mylib::RenderablePtr renderable, RoadVertexPtr vertex, QColor color, float height) {
	mylib::Vertex v;

	// draw the vertex
	v.color[0] = color.redF();
	v.color[1] = color.greenF();
	v.color[2] = color.blueF();
	v.color[3] = color.alphaF();
	v.normal[0] = 0.0f;
	v.normal[1] = 0.0f;
	v.normal[2] = 1.0f;

	v.location[0] = vertex->pt.x();
	v.location[1] = vertex->pt.y();
	v.location[2] = height;

	renderable->vertices.push_back(v);
}
*/

void RoadGraph::clear() {
	graph.clear();

	modified = true;
}

void RoadGraph::setZ(float z) {
	// define the width per lane
	float widthBase2;
	if (z < 450.0f) {
		widthBase2 = 2.0f;
	} else if (z < 900.0f) {
		widthBase2 = 4.0f;
	} else if (z < 1620.0f) {
		widthBase2 = 10.0f;
	} else if (z < 5760.0f) {
		widthBase2 = 12.0f;
	} else {
		widthBase2 = 24.0f;
	}
	if (widthBase != widthBase2) {
		widthBase = widthBase2;
		setModified();
	}

	// define the curb ratio
	float curbRatio2;
	if (z < 2880.0f) {
		curbRatio2 = 0.4f;
	} else {
		curbRatio2 = 0.8f;
	}
	if (curbRatio != curbRatio2) {
		curbRatio = curbRatio2;
		setModified();
	}

	// define the height
	float highwayHeight2 = (float)((int)(z * 0.012f)) * 0.1f;
	if (highwayHeight != highwayHeight2) {
		highwayHeight = highwayHeight2;
		avenueHeight = highwayHeight2 * 0.66f;
		setModified();
	}
}

/**
 * 道路網のGeometryを更新した場合は、必ずこの関数を実行して、3D Geometryを更新すること。
 */
void RoadGraph::adaptToTerrain(Terrain* terrain) {
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(graph); vi != vend; ++vi) {
		float z = terrain->getValue(graph[*vi]->pt.x(), graph[*vi]->pt.y());
		if (z < 0.0f) {
			graph[*vi]->properties["bridgeHeight"] = -z + 2.0f;
			z = 0.0f;
		}
		graph[*vi]->pt3D = QVector3D(graph[*vi]->pt.x(), graph[*vi]->pt.y(), z + 1);
	}

	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(graph); ei != eend; ++ei) {
		graph[*ei]->polyline3D.clear();

		float bridgeHeight = 0.0f;
		for (int i = 0; i < graph[*ei]->polyline.size(); ++i) {
			// たまに、同じポイントが重複していることがあり、そのせいで道路表示がねじれてしまう。
			// 仕方がないので、同じポイントならスキップするようにした。
			if (i > 0 && (graph[*ei]->polyline[i] - graph[*ei]->polyline[i - 1]).lengthSquared() < 1.0f) continue;

			float z = terrain->getValue(graph[*ei]->polyline[i].x(), graph[*ei]->polyline[i].y());
			if (z < 0.0f) {
				if (-z + 2.0f > bridgeHeight) {
					bridgeHeight = -z + 2.0f;
				}
				z = 0.0f;
			}
			graph[*ei]->polyline3D.push_back(QVector3D(graph[*ei]->polyline[i].x(), graph[*ei]->polyline[i].y(), z + 1));
		}
		
		graph[*ei]->properties["bridgeHeight"] = bridgeHeight;
	}

	setModified();
}

