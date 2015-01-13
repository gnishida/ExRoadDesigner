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

#include <qstring.h>
#include <qvector2d.h>

namespace mylib {

class Texture {
private:
	QString fileName;
	//float size;
	QVector2D size;
	unsigned int id;

public:
	Texture(unsigned int id, QString filename, const QVector2D& size);
	~Texture();

	QString getFileName() { return fileName; }
	//void setFileName(QString fileName) { this->fileName = fileName; }
	float getSizeX() { return size.x(); }
	float getSizeY() { return size.y(); }
	void setSize(float x, float y) { this->size.setX(x); this->size.setY(y); }
	unsigned int getId() { return id; }
	//void setId(unsigned int urn id) { this->urn id = urn id; }

	//void bind(bool repeat = false);
	//void bind(float xSize, float ySize, bool repeat = false);
};

} // namespace mylib