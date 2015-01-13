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

#include <QGLWidget>
#include "TextureManager.h"
#include "Texture.h"
#include <qfile.h>
#include <qtextstream.h>
#include <iostream>

namespace mylib {

TextureManager::TextureManager(QGLWidget* glWidget) {
	this->glWidget = glWidget;
	//initializeTextures(glWidget, "data/textures/textures.txt");
}

TextureManager::~TextureManager() {
	clear();
}

TextureManager* TextureManager::getInstance(QGLWidget* glWidget) {
	static TextureManager instance(glWidget);
	return &instance;
}

void TextureManager::clear() {
	for (QHash<QString, Texture*>::iterator it = textures.begin(); it != textures.end(); ++it) {
		delete it.value();
	}
	textures.clear();
}

void TextureManager::initializeTextures(QGLWidget* glWidget, QString filename) {
	clear();

	//read texture file
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		std::cerr << "TextureManager::initializeTexture()... The file is not accessible: " << filename.toUtf8().constData() << endl;
		throw "The file is not accessible: " + filename;
	}

	QTextStream in(&file);
	QString line;
	while ((line = in.readLine()) != NULL) {
		QString filepath = line.split(":").at(0);
		if (!QFile::exists(filepath)) {
			std::cerr << "TextureManager::initializeTexture()... The texture file is not accessible: " << filepath.toUtf8().constData() << endl;
			throw "The texture ile is not accessible: " + filepath;
		}

		unsigned int tex = glWidget->bindTexture(filepath, GL_TEXTURE_2D);
		QVector2D size(line.split(":").at(1).toFloat(), line.split(":").at(2).toFloat());

		textures.insert(filepath, new Texture(tex, filepath, size));
	}
	file.close();
}

Texture* TextureManager::get(QString name) {
	if (!textures.contains(name)) {
		std::cerr << "TextureManager::get()... No such texture is found: " << name.toUtf8().constData() << endl;
		throw "No such texture is found: " + name;
	}
	
	return textures[name];
}

} // namespace mylib