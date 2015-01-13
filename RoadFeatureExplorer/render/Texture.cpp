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

#include "Texture.h"
#include <QtOpenGL>

namespace mylib {

Texture::Texture(unsigned int id, QString filename, const QVector2D& size) {
	this->id = id;
	this->fileName = filename;
	this->size = size;
}


Texture::~Texture() {
}

/*void Texture::bind(bool repeat) {
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode (GL_MODELVIEW);

	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::bind(float xSize, float ySize, bool repeat) {
	bool scale = false;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	if (xSize != size || ySize != size) {
		scale = true;
		scaleX = size / xSize;
		scaleY = size / ySize;
	}

	glMatrixMode (GL_TEXTURE);
	//glPushMatrix();
	glLoadIdentity();
	if (scale) {
		glScalef(scaleX, scaleY, 1);
	}
	glMatrixMode (GL_MODELVIEW);

	glBindTexture(GL_TEXTURE_2D, id);
}*/

} // namespace mylib