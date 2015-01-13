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
#include <qhash.h>
//#include <vector>

class QGLWidget;

namespace mylib {

class Texture;

class TextureManager {
protected:
	QGLWidget* glWidget;
	//std::vector<Texture*> textures;
	QHash<QString, Texture*> textures;

protected:
	TextureManager(QGLWidget* glWidget);
	~TextureManager();
	TextureManager(const TextureManager& ref);
	TextureManager& operator=(const TextureManager& ref);

public:
	static TextureManager* getInstance(QGLWidget* glWidget);
	void clear();
	void initializeTextures(QGLWidget* glWidget, QString filename);
	Texture* get(QString name);
};

} // namespace mylib