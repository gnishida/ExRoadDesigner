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
#include "Renderable.h"

namespace mylib {

class RenderableCircleList : public Renderable {
public:
	RenderableCircleList();
	RenderableCircleList(Texture* texture);
	~RenderableCircleList() {}

	void addCircle(const QVector3D& o, float r, int slices, const QColor& color);
	void addCircle(const QVector3D& o, float r, int slices, const QColor& color, float hightOffset);
	void addCircle(const QVector3D& o, float r, int slices);
	void addCircle(const QVector3D& o, float r, int slices, float heightOffset);
};

} // namespace mylib