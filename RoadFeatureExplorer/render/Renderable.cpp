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

#include "Renderable.h"

namespace mylib {

Renderable::Renderable() {
	glBeginMode = GL_TRIANGLES;	// This should be always GL_TRIANGLES in the future, but we allow others for now.
	texture = NULL;
}

Renderable::Renderable(GLenum glBeginMode) {
	this->glBeginMode = glBeginMode;
	texture = NULL;
}

Renderable::Renderable(GLenum glBeginMode, float size) {
	this->glBeginMode = glBeginMode;
	this->size = size;
	texture = NULL;
}

Renderable::~Renderable() {
}

void Renderable::addVertex(const Vertex& vertex) {
	vertices.push_back(vertex);
}

void Renderable::generateMeshVertex(float x, float y, float z, float nx, float ny, float nz, float s, float t) {
	mylib::Vertex vertex;
	vertex.location[0] = x;
	vertex.location[1] = y;
	vertex.location[2] = z;
	vertex.normal[0] = nx;
	vertex.normal[1] = ny;
	vertex.normal[2] = nz;
	vertex.tex[0] = s;
	vertex.tex[1] = t;
	vertex.color[0] = 1.0f;
	vertex.color[1] = 1.0f;
	vertex.color[2] = 1.0f;
	vertex.color[3] = 1.0f;
	vertices.push_back(vertex);
}

void Renderable::generateMeshVertex(float x, float y, float z, float nx, float ny, float nz, const QColor& color) {
	mylib::Vertex vertex;
	vertex.location[0] = x;
	vertex.location[1] = y;
	vertex.location[2] = z;
	vertex.normal[0] = nx;
	vertex.normal[1] = ny;
	vertex.normal[2] = nz;
	vertex.tex[0] = 0.0f;
	vertex.tex[1] = 0.0f;
	vertex.color[0] = color.redF();
	vertex.color[1] = color.greenF();
	vertex.color[2] = color.blueF();
	vertex.color[3] = 1.0f;
	vertices.push_back(vertex);
}

/*void Renderable::generateFace(int face, float x, float y, float z, float w, float d, float h, GLuint texture_id, float s0, float s1, float t0, float t1) {
	switch (face) {
	case 2:	// right face
		generateMeshVertex(x + w, y, z, 1, 0, 0, s0, t0);
		generateMeshVertex(x + w, y + d, z, 1, 0, 0, s1, t0);
		generateMeshVertex(x + w, y + d, z + h, 1, 0, 0, s1, t1);

		generateMeshVertex(x + w, y, z, 1, 0, 0, s0, t0);
		generateMeshVertex(x + w, y + d, z + h, 1, 0, 0, s1, t1);
		generateMeshVertex(x + w, y, z + h, 1, 0, 0, s0, t1);
		break;
	case 4: // left face
		generateMeshVertex(x, y, z, -1, 0, 0, s0, t0);
		generateMeshVertex(x, y + d, z, -1, 0, 0, s1, t0);
		generateMeshVertex(x, y + d, z + h, -1, 0, 0, s1, t1);

		generateMeshVertex(x, y, z, -1, 0, 0, s0, t0);
		generateMeshVertex(x, y + d, z + h, -1, 0, 0, s1, t1);
		generateMeshVertex(x, y, z + h, -1, 0, 0, s0, t1);
		break;
	case 1:	// back face
		generateMeshVertex(x + w, y + d, z, 0, 1, 0, s0, t0);
		generateMeshVertex(x, y + d, z, 0, 1, 0, s1, t0);
		generateMeshVertex(x, y + d, z + h, 0, 1, 0, s1, t1);

		generateMeshVertex(x + w, y + d, z, 0, 1, 0, s0, t0);
		generateMeshVertex(x, y + d, z + h, 0, 1, 0, s1, t1);
		generateMeshVertex(x + w, y + d, z + h, 0, 1, 0, s0, t1);
		break;
	case 3:	// front face
		generateMeshVertex(x, y, z, 0, -1, 0, s0, t0);
		generateMeshVertex(x + w, y, z, 0, -1, 0, s1, t0);
		generateMeshVertex(x + w, y, z + h, 0, -1, 0, s1, t1);

		generateMeshVertex(x, y, z, 0, -1, 0, s0, t0);
		generateMeshVertex(x + w, y, z + h, 0, -1, 0, s1, t1);
		generateMeshVertex(x, y, z + h, 0, -1, 0, s0, t1);
		break;
	case 5:	// top face
		generateMeshVertex(x, y, z + h, 0, 0, 1, s0, t0);
		generateMeshVertex(x + w, y, z + h, 0, 0, 1, s1, t0);
		generateMeshVertex(x + w, y + d, z + h, 0, 0, 1, s1, t1);

		generateMeshVertex(x, y, z + h, 0, 0, 1, s0, t0);
		generateMeshVertex(x + w, y + d, z + h, 0, 0, 1, s1, t1);
		generateMeshVertex(x, y + d, z + h, 0, 0, 1, s0, t1);
		break;
	case 6:	// bottom face
		generateMeshVertex(x, y + d, z, 0, 0, -1, s0, t0);
		generateMeshVertex(x + w, y + d, z, 0, 0, -1, s1, t0);
		generateMeshVertex(x + w, y, z, 0, 0, -1, s1, t1);

		generateMeshVertex(x, y + d, z, 0, 0, -1, s0, t0);
		generateMeshVertex(x + w, y, z, 0, 0, -1, s1, t1);
		generateMeshVertex(x, y, z, 0, 0, -1, s0, t1);
		break;
	}
}

void Renderable::generateFace(int face, float x, float y, float z, float w, float d, float h, const QColor& color) {
	switch (face) {
	case 2:	// right face
		generateMeshVertex(x + w, y, z, 1, 0, 0, color);
		generateMeshVertex(x + w, y + d, z, 1, 0, 0, color);
		generateMeshVertex(x + w, y + d, z + h, 1, 0, 0, color);

		generateMeshVertex(x + w, y, z, 1, 0, 0, color);
		generateMeshVertex(x + w, y + d, z + h, 1, 0, 0, color);
		generateMeshVertex(x + w, y, z + h, 1, 0, 0, color);
		break;
	case 4: // left face
		generateMeshVertex(x, y, z, -1, 0, 0, color);
		generateMeshVertex(x, y + d, z, -1, 0, 0, color);
		generateMeshVertex(x, y + d, z + h, -1, 0, 0, color);

		generateMeshVertex(x, y, z, -1, 0, 0, color);
		generateMeshVertex(x, y + d, z + h, -1, 0, 0, color);
		generateMeshVertex(x, y, z + h, -1, 0, 0, color);
		break;
	case 1:	// back face
		generateMeshVertex(x + w, y + d, z, 0, 1, 0, color);
		generateMeshVertex(x, y + d, z, 0, 1, 0, color);
		generateMeshVertex(x, y + d, z + h, 0, 1, 0, color);

		generateMeshVertex(x + w, y + d, z, 0, 1, 0, color);
		generateMeshVertex(x, y + d, z + h, 0, 1, 0, color);
		generateMeshVertex(x + w, y + d, z + h, 0, 1, 0, color);
		break;
	case 3:	// front face
		generateMeshVertex(x, y, z, 0, -1, 0, color);
		generateMeshVertex(x + w, y, z, 0, -1, 0, color);
		generateMeshVertex(x + w, y, z + h, 0, -1, 0, color);

		generateMeshVertex(x, y, z, 0, -1, 0, color);
		generateMeshVertex(x + w, y, z + h, 0, -1, 0, color);
		generateMeshVertex(x, y, z + h, 0, -1, 0, color);
		break;
	case 5:	// top face
		generateMeshVertex(x, y, z + h, 0, 0, 1, color);
		generateMeshVertex(x + w, y, z + h, 0, 0, 1, color);
		generateMeshVertex(x + w, y + d, z + h, 0, 0, 1, color);

		generateMeshVertex(x, y, z + h, 0, 0, 1, color);
		generateMeshVertex(x + w, y + d, z + h, 0, 0, 1, color);
		generateMeshVertex(x, y + d, z + h, 0, 0, 1, color);
		break;
	case 6:	// bottom face
		generateMeshVertex(x, y + d, z, 0, 0, -1, color);
		generateMeshVertex(x + w, y + d, z, 0, 0, -1, color);
		generateMeshVertex(x + w, y, z, 0, 0, -1, color);

		generateMeshVertex(x, y + d, z, 0, 0, -1, color);
		generateMeshVertex(x + w, y, z, 0, 0, -1, color);
		generateMeshVertex(x, y, z, 0, 0, -1, color);
		break;
	}
}*/

} // namespace mylib