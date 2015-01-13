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

#pragma once

#include <QVector3D>
#include <QColor>
#include <vector>
#include <QtOpenGL>
#include "Renderable.h"

namespace mylib {

class TextureManager;

class GeometryObject {
protected:
	// true if the mesh has been modified
	bool modified;

public:
	QVector3D pt;
	float scale;
	float rotateZ;
	QColor color;
	std::vector<RenderablePtr> renderables;
	GLuint dispList;

public:
	GeometryObject();
	GeometryObject(const QVector3D& pt);
	~GeometryObject();

	/** getter for modified */
	bool getModified() { return modified; }

	/** setter for modified */
	void setModified() { this->modified = true; }

	/** getter/setter for pt */
	QVector3D& getPt() { return pt; }
	void setPt(const QVector3D& pt) { this->pt = pt; }

	/** getter/setter for scale */
	float getScale() { return scale; }
	void setScale(float scale) { this->scale = scale; }

	/** getter/setter for rotateZ */
	float getRotateZ() { return rotateZ; }
	void setRotateZ(float rotateZ) { this->rotateZ = rotateZ; }

	/** getter/setter for color */
	const QColor& getColor() const { return color; }
	void setColor(const QColor& color) { this->color = color; }

	/** getter for renderables */
	//std::vector<Renderable*>& getRenderables() { return renderables; }
	int getNumRenderables() { return (int)renderables.size(); }
	RenderablePtr getRenderable(int index);

	GLuint getDispList() { return dispList; }
	void newDispList();

	void clearRenderables();
	void generateMeshVertices(TextureManager* textureManager);

protected:
	virtual void _generateMeshVertices(TextureManager* textureManager) = 0;
};

} // namespace mylib