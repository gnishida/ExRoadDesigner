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

#include "../common/common.h"
#include <QtOpenGL>
#include "Noncopyable.h"

namespace mylib {

class TextureManager;
class Texture;

typedef struct {
	float location[3];
	float tex[2];
	float normal[3];
	float color[4];
	char padding[16];
} Vertex;

class Renderable : private Noncopyable {
private:
	// true if the mesh has been modified
	bool modified;

public:
	Texture* texture;
	GLuint vbo;
	std::vector<Vertex> vertices;
	GLenum glBeginMode;
	float size;

public:
	Renderable();
	Renderable(GLenum glBeginMode);
	Renderable(GLenum glBeginMode, float size);
	~Renderable();

	/** getter/setter for texture */
	Texture* getTexture() { return texture; }
	void setTexture(Texture* texture) { this->texture = texture; }

	/** gettere/setter for vbo */
	GLuint getVbo() { return vbo; }

	/** getter/setter for vertices */
	size_t getNumMeshVertices() { return vertices.size(); }
	GLfloat* getMeshVertices() { return (GLfloat*)vertices.data(); }
	Vertex& getMeshVertex(size_t index) { return vertices[index]; }
	void addVertex(const Vertex& vertex);

	GLenum getGlBeginMode() { return glBeginMode; }

	void generateMeshVertices();

	void generateMeshVertex(float x, float y, float z, float nx, float ny, float nz, float s, float t);
	void generateMeshVertex(float x, float y, float z, float nx, float ny, float nz, const QColor& color);
	//void generateFace(int face, float x, float y, float z, float w, float d, float h, GLuint texture_id, float s0, float s1, float t0, float t1);
	//void generateFace(int face, float x, float y, float z, float w, float d, float h, const QColor& color);

protected:
	//virtual void _generateMeshVertices() = 0;
};

typedef boost::shared_ptr<Renderable> RenderablePtr;

} // namespace mylib