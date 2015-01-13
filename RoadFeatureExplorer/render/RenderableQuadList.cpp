/*********************************************************************
This file is part of QtUrban.

    QtUrban is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    QtUrban is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtUrban.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

#include "RenderableQuadList.h"

namespace mylib {

RenderableQuadList::RenderableQuadList() : Renderable(GL_QUADS) {
}

RenderableQuadList::RenderableQuadList(Texture* texture) : Renderable(GL_QUADS) {
	this->texture = texture;
}

void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal, float s0, float s1, float t0, float t1) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z(), normal.x(), normal.y(), normal.z(), s0, t0);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z(), normal.x(), normal.y(), normal.z(), s1, t0);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z(), normal.x(), normal.y(), normal.z(), s1, t1);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z(), normal.x(), normal.y(), normal.z(), s0, t1);
}

void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal, float s0, float s1, float t0, float t1, float heightOffset) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z() + heightOffset, normal.x(), normal.y(), normal.z(), s0, t0);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z() + heightOffset, normal.x(), normal.y(), normal.z(), s1, t0);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z() + heightOffset, normal.x(), normal.y(), normal.z(), s1, t1);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z() + heightOffset, normal.x(), normal.y(), normal.z(), s0, t1);
}

void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal, const QColor& color) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z(), normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z(), normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z(), normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z(), normal.x(), normal.y(), normal.z(), color);
}

/**
 * Z座標を、全てhightOffset足して表示する
 */
void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal, const QColor& color, float heightOffset) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z() + heightOffset, normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z() + heightOffset, normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z() + heightOffset, normal.x(), normal.y(), normal.z(), color);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z() + heightOffset, normal.x(), normal.y(), normal.z(), color);
}

void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal, const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z(), normal.x(), normal.y(), normal.z(), color1);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z(), normal.x(), normal.y(), normal.z(), color2);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z(), normal.x(), normal.y(), normal.z(), color3);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z(), normal.x(), normal.y(), normal.z(), color4);
}

void RenderableQuadList::addQuad(const QVector3D& pt1, const QVector3D& pt2, const QVector3D& pt3, const QVector3D& pt4, const QVector3D& normal1, const QVector3D& normal2, const QVector3D& normal3, const QVector3D& normal4, const QColor& color1, const QColor& color2, const QColor& color3, const QColor& color4) {
	generateMeshVertex(pt1.x(), pt1.y(), pt1.z(), normal1.x(), normal1.y(), normal1.z(), color1);
	generateMeshVertex(pt2.x(), pt2.y(), pt2.z(), normal2.x(), normal2.y(), normal2.z(), color2);
	generateMeshVertex(pt3.x(), pt3.y(), pt3.z(), normal3.x(), normal3.y(), normal3.z(), color3);
	generateMeshVertex(pt4.x(), pt4.y(), pt4.z(), normal4.x(), normal4.y(), normal4.z(), color4);
}

} // namespace mylib