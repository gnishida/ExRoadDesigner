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

#include "GeometryObject.h"

namespace mylib {

GeometryObject::GeometryObject() {
	scale = 1.0f;
	rotateZ = 0.0f;
	color = QColor(255, 255, 255);
	dispList = 0;
}

GeometryObject::GeometryObject(const QVector3D& pt) {
	this->pt = pt;
	scale = 1.0f;
	rotateZ = 0.0f;
	color = QColor(255, 255, 255);
	dispList = 0;
}

GeometryObject::~GeometryObject() {
	glDeleteLists(dispList, 1);
	clearRenderables();
}

void GeometryObject::clearRenderables() {
	for (int i = 0; i < renderables.size(); i++) {
		//delete renderables[i];
	}
	renderables.clear();
}

RenderablePtr GeometryObject::getRenderable(int index) {
	if (index < 0 || index >= renderables.size()) {
		int a = 0;
		return NULL;
	}

	return renderables[index];
}

void GeometryObject::newDispList() {
	glDeleteLists(dispList, 1);
	dispList = glGenLists(1);
	glNewList(dispList, GL_COMPILE_AND_EXECUTE);
}

void GeometryObject::generateMeshVertices(TextureManager* textureManager) {
	if (!modified) return;

	clearRenderables();
	_generateMeshVertices(textureManager);
	modified = false;
}

} // namespace mylib