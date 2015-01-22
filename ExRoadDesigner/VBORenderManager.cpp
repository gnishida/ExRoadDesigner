#include "VBORenderManager.h"
#include "global.h"

using namespace boost::polygon::operators;

VBORenderManager::VBORenderManager() {
	editionMode=false;
	side=5000.0f;
	minPos=QVector3D(-side/2.0f, -side/2.0f, 0);
	maxPos=QVector3D(side/2.0f, side/2.0f, 0);
	//initializedStreetElements=false;
}

VBORenderManager::~VBORenderManager() {
	Shader::cleanShaders();
}

void VBORenderManager::init() {
	// init program shader
	program=Shader::initShader("../data/shaders/lc_vertex_sk.glsl", "../data/shaders/lc_fragment_sk.glsl");
	glUseProgram(program);

	vboTerrain.init(this, 200);
	vboSkyBox.init(*this);

	nameToTexId[""]=0;

	printf("VBORenderManager\n");
}

GLuint VBORenderManager::loadTexture(const QString fileName,bool mirrored){
	GLuint texId;
	if(nameToTexId.contains(fileName)){
		texId=nameToTexId[fileName];
	}else{
		texId=VBOUtil::loadImage(fileName,mirrored);
		nameToTexId[fileName]=texId;
	}
	return texId;
}//

GLuint VBORenderManager::loadArrayTexture(QString texName,std::vector<QString> fileNames){
	GLuint texId;
	if(nameToTexId.contains(texName)){
		texId=nameToTexId[texName];
	}else{
		texId=VBOUtil::loadImageArray(fileNames);
		nameToTexId[texName]=texId;
	}
	return texId;
}

// ATRIBUTES
// 0 Vertex
// 1 Color
// 2 Normal
// 3 UV

// UNIFORMS
// 0 mode
// 1 tex0

bool VBORenderManager::createVAO(std::vector<Vertex>& vert,GLuint& vbo,GLuint& vao,int& numVertex){
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);
	// Crete VBO
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vert.size(), vert.data(), GL_STATIC_DRAW);
	
	// Configure the attributes in the VAO.
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(4*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(8*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(12*sizeof(float)));
	
	// Bind back to the default state.
	glBindVertexArray(0); 
	glBindBuffer(GL_ARRAY_BUFFER,0);
	
	// clean space
	numVertex=vert.size();
	
	return true;
}

void VBORenderManager::renderVAO(RenderSt& renderSt,bool cleanVertex){
	//printf("renderVAO numVert %d texNum %d vao %d numVertVert %d\n",renderSt.numVertex,renderSt.texNum,renderSt.vao,renderSt.vertices.size());
	// 1. Create if necessary
	if(renderSt.numVertex!=renderSt.vertices.size()&&renderSt.vertices.size()>0){
		if(renderSt.numVertex!=-1){
			cleanVAO(renderSt.vbo,renderSt.vao);
		}
		// generate vao/vbo
		createVAO(renderSt.vertices,renderSt.vbo,renderSt.vao,renderSt.numVertex);
		if(cleanVertex)
			renderSt.vertices.clear();
	}
	// 2. Render
	// 2.1 TEX
	int mode=renderSt.shaderMode;
	if((mode&mode_TexArray)==mode_TexArray){
		// MULTI TEX
		mode=mode&(~mode_TexArray);//remove tex array bit
		glActiveTexture(GL_TEXTURE8);

		glBindTexture(GL_TEXTURE_2D,0); 
		glBindTexture(GL_TEXTURE_2D_ARRAY, renderSt.texNum);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i (glGetUniformLocation (program, "tex_3D"), 8);
	}else{
		glBindTexture(GL_TEXTURE_2D, renderSt.texNum);
	}
	// 2.2 mode
	//if(renderSt.texNum==0){
		//glUniform1i (glGetUniformLocation (program, "mode"), 1|(renderSt.shaderMode&0xFF00));//MODE: same modifiers but just color (renderSt.shaderMode&0xFF00)
	//}else{
		glUniform1i (glGetUniformLocation (program, "mode"), mode);
	//}

	glUniform1i (glGetUniformLocation (program, "tex0"), 0);//tex0: 0

	glBindVertexArray(renderSt.vao);
	glDrawArrays(renderSt.geometryType,0,renderSt.numVertex);
	glBindVertexArray(0);
}

void VBORenderManager::renderAll(bool cleanVertex){
	//vboTerrain.render(editionMode,mousePos3D);
	//vboWater.render(*this);
		
	////////////////////////
	// 1. STATIC
	QHash<QString,renderGrid>::iterator statcIt;
	// 1.1 iterate over geoNames
	for (statcIt = geoName2StaticRender.begin(); statcIt != geoName2StaticRender.end(); ++statcIt){
		printf("Render: %s\n",statcIt.key().toAscii().constData());
		// 1.1.1 iterate over textures
		renderGrid::iterator i;
		for (i = statcIt.value().begin(); i != statcIt.value().end(); ++i){
			renderVAO(i.value(),cleanVertex);
		}
	}

}//

void VBORenderManager::cleanVAO(GLuint vbo,GLuint vao){
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}//

/**
 * Return the terrain elevation.
 *
 * @param x			x coordinate [-side/2, side/2]
 * @param y			y coordinate [-side/2, side/2]
 */
float VBORenderManager::getTerrainHeight(float x, float y) {
	float u = x / side + 0.5f;
	float v = y / side + 0.5f;
	if (u < 0) u = 0.0f;
	if (u > 1.0f) u = 1.0f;
	if (v < 0) v = 0.0f;
	if (v > 1.0f) v = 1.0f;

	return vboTerrain.getTerrainHeight(u, v);
}

/**
 * Return the minimum terrain elevation around the given poing.
 *
 * @param x			x coordinate [-side/2, side/2]
 * @param y			y coordinate [-side/2, side/2]
 * @param radius	radius
 */
float VBORenderManager::getMinTerrainHeight(float x, float y, float radius) {
	float u = x / side + 0.5f;
	float v = y / side + 0.5f;
	float r = radius / side;

	float min_z = std::numeric_limits<float>::max();
	for (int i = -1; i <= 1; ++i) {
		float uu = u + i * r;
		if (uu < 0) uu = 0.0f;
		if (uu > 1.0f) uu = 1.0f;

		for (int j = -1; j <= 1; ++j) {
			float vv = v + j * r;
			if (vv < 0) vv = 0.0f;
			if (vv > 1.0f) vv = 1.0f;

			float z = vboTerrain.getTerrainHeight(uu, vv);
			if (z < min_z) min_z = z;
		}
	}

	return min_z;
}

void VBORenderManager::changeTerrainDimensions(float terrainSide,int resolution) {
	side = terrainSide;
	minPos = QVector3D(-side/2.0f, -side/2.0f, 0);
	maxPos = QVector3D(side/2.0f, side/2.0f, 0);
	vboTerrain.init(this, resolution);
	vboSkyBox.init(*this);
}

void VBORenderManager::changeTerrainShader(int newMode){
	glUniform1i(glGetUniformLocation(program, "terrainMode"),newMode);//newMode
}

///////////////////////////////////////////////////////////////////
// STATIC
bool VBORenderManager::addStaticGeometry(QString geoName,std::vector<Vertex>& vert,QString texName,GLenum geometryType,int shaderMode){
	if(vert.size() <= 0)
		return false;
	GLuint texId;
	if(nameToTexId.contains(texName)){
		texId=nameToTexId[texName];
	}else{
		printf("load img %s\n",texName.toAscii().constData());
		texId=VBOUtil::loadImage(texName);
		nameToTexId[texName]=texId;
	}
		
	if(geoName2StaticRender.contains(geoName)){
		// 1.1 If already in manager
		if(geoName2StaticRender[geoName].contains(texId)){
			if(geoName2StaticRender[geoName][texId].vertices.size()==0){
				//1.1.1 if also contains texture and the number of vertex=0--> vao created--> remove
				cleanVAO(geoName2StaticRender[geoName][texId].vbo,geoName2StaticRender[geoName][texId].vao);
				geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
			}else{
				//1.1.1 if also contains texture and the number of vertex!=0--> vao no created--> just append
				if(geometryType==GL_TRIANGLE_STRIP){
					//vert.insert(vert.begin(),vert.front());
					vert.insert(vert.begin(),geoName2StaticRender[geoName][texId].vertices.back());
					vert.insert(vert.begin(),geoName2StaticRender[geoName][texId].vertices.back());
				}
				geoName2StaticRender[geoName][texId].vertices.insert(geoName2StaticRender[geoName][texId].vertices.end(),vert.begin(),vert.end());
			}
		}else{
			geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
		}
		//printf("--> YES in manager %s\n",geoName.toAscii().constData());
	}else{
		// 1.2 No yet in manager
		geoName2StaticRender[geoName][texId]=RenderSt(texId,vert,geometryType,shaderMode);
		//renderStaticGeometry(geoName);
		//printf("--> It was not yet in manager %s\n",geoName.toAscii().constData());
	}
	return true;
}//

/**
	* 指定されたポリゴンに基づいて、ジオメトリを生成する。
	* 凹型のポリゴンにも対応するよう、ポリゴンは台形にtessellateする。
	*/
bool VBORenderManager::addStaticGeometry2(QString geoName,std::vector<QVector3D>& pos,float zShift,bool inverseLoop,QString textureName,GLenum geometryType,int shaderMode,QVector3D texScale,QColor color){
	if(pos.size()<3){
		return false;
	}
	PolygonSetP polySet;
	polygonP tempPolyP;

	std::vector<pointP> vP;
	vP.resize(pos.size());
	float minX=FLT_MAX,minY=FLT_MAX;
	float maxX=-FLT_MAX,maxY=-FLT_MAX;

	for(int pN=0;pN<pos.size();pN++){
		vP[pN]=boost::polygon::construct<pointP>(pos[pN].x(),pos[pN].y());
		minX=std::min<float>(minX,pos[pN].x());
		minY=std::min<float>(minY,pos[pN].y());
		maxX=std::max<float>(maxX,pos[pN].x());
		maxY=std::max<float>(maxY,pos[pN].y());
	}
	if(pos.back().x()!=pos.front().x()&&pos.back().y()!=pos.front().y()){
		vP.push_back(vP[0]);
	}

	boost::polygon::set_points(tempPolyP,vP.begin(),vP.end());
	polySet+=tempPolyP;
	std::vector<polygonP> allP;
	boost::polygon::get_trapezoids(allP,polySet);
		
	std::vector<Vertex> vert;

	for(int pN=0;pN<allP.size();pN++){
		//glColor3ub(qrand()%255,qrand()%255,qrand()%255);
		boost::polygon::polygon_with_holes_data<double>::iterator_type itPoly=allP[pN].begin();

		Polygon3D points;
		//std::vector<QVector3D> points;
		std::vector<QVector3D> texP;
		while(itPoly!=allP[pN].end()){
			pointP cP=*itPoly;
			if(inverseLoop==false)
				points.push_back(QVector3D(cP.x(),cP.y(),pos[0].z()+zShift));
			else
				points.contour.insert(points.contour.begin(),QVector3D(cP.x(),cP.y(),pos[0].z()+zShift));

			//if(texZeroToOne==true){
				//texP.push_back(QVector3D((cP.x()-minX)/(maxX-minX),(cP.y()-minY)/(maxY-minY),0.0f));
			//}else{
				texP.push_back(QVector3D((cP.x()-minX)*texScale.x(),(cP.y()-minY)*texScale.y(),0.0f));
			//}
			itPoly++;
		}
		if(points.contour.size()==0)continue;
		while(points.contour.size()<4)
			points.push_back(points.contour.back());

		// GEN 
		// Sometimes, the polygon is formed in CW order, so we have to reorient it in CCW order.
		if(inverseLoop==false) {
			points.correct();
		}

		/*if(points.size()==4){//last vertex repited
			addTexTriang(texInd,points,texP,col,norm);
		}
		if(points.size()==5){
			addTexQuad(texInd,points,texP,col,norm);

		}
		if(points.size()==6){
			//addTexQuad(texInd,std::vector<QVector3D>(&points[0],&points[3]),std::vector<QVector3D>(&texP[0],&texP[3]),col,norm);

			addTexQuad(texInd,points,texP,col,norm);
			//addTexTriang(texInd,std::vector<QVector3D>(&points[3],&points[6]),std::vector<QVector3D>(&texP[3],&texP[6]),col,norm);
			//addTexTriang(texInd,std::vector<QVector3D>(&points[4],&points[6]),std::vector<QVector3D>(&texP[4],&texP[6]),col,norm);
		}*/
		vert.push_back(Vertex(points[0],color,QVector3D(0,0,1),texP[0]));//texScale is a hack to define a color when it is not texture
		vert.push_back(Vertex(points[1],color,QVector3D(0,0,1),texP[1]));
		vert.push_back(Vertex(points[2],color,QVector3D(0,0,1),texP[2]));
		vert.push_back(Vertex(points[3],color,QVector3D(0,0,1),texP[3]));
	}

	return addStaticGeometry(geoName,vert,textureName,geometryType,shaderMode);
}//

bool VBORenderManager::removeStaticGeometry(QString geoName){
	if(geoName2StaticRender.contains(geoName)){
		// iterate and remove
		renderGrid::iterator i;
		for (i = geoName2StaticRender[geoName].begin(); i != geoName2StaticRender[geoName].end(); ++i){
			cleanVAO(geoName2StaticRender[geoName][i.key()].vbo,geoName2StaticRender[geoName][i.key()].vao);
		}
		geoName2StaticRender[geoName].clear();
		geoName2StaticRender.remove(geoName);
	}else{
		//printf("ERROR: Remove Geometry %s but it did not exist\n",geoName.toAscii().constData());
		return false;
	}

	return true;
}//

void VBORenderManager::renderStaticGeometry(QString geoName){

	if(geoName2StaticRender.contains(geoName)){
		// iterate and remove
		renderGrid::iterator i;
		for (i = geoName2StaticRender[geoName].begin(); i != geoName2StaticRender[geoName].end(); ++i){
			renderVAO(i.value(),false);
		}
	}else{
		//printf("ERROR: Render Geometry %s but it did not exist\n",geoName.toAscii().constData());
		return;
	}
}

void VBORenderManager::addPoint(const QString &name, const QVector2D& pt, const QColor& color, float height) {
	std::vector<Vertex> vertP;

	Vertex v(pt.x(), pt.y(), height, color, 0, 0, 1, 0, 0, 0);//pos color normal tex
	vertP.push_back(v);

	addStaticGeometry(name, vertP, "", GL_POINTS, 1);
}

void VBORenderManager::addPolyline(const QString &name, const Polyline3D& polyline, const QColor& color) {
	std::vector<Vertex> vertP;
	std::vector<Vertex> vertL;

	int num = polyline.size();
	if (num == 0) return;

	std::vector<Vertex> vert(num - 1);

	for (int i = 0; i < num; ++i) {
		Vertex v = Vertex(polyline[i].x(), polyline[i].y(), polyline[i].z(), color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertP.push_back(v);
	}

	// add lines
	for (int i = 0; i < num - 1; i++) {
		Vertex v = Vertex(polyline[i].x(), polyline[i].y(), polyline[i].z(), color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		Vertex v2 = Vertex(polyline[i+1].x(), polyline[i+1].y(), polyline[i+1].z(), color, 0, 0, 1, 0, 0, 0);//pos color normal tex
		vertL.push_back(v);
		vertL.push_back(v2);
	}

	addStaticGeometry(name, vertL, "", GL_LINES, 1);
	addStaticGeometry(name, vertP, "dummy", GL_POINTS, 1);
}

///////////////////////////////////////////////////////////////////
// GRID
bool VBORenderManager::addGridGeometry(QString geoName,std::vector<Vertex>& vert,QString textureName){
	return false;
}//
bool VBORenderManager::removeGridGeometry(QString geoName){
	return false;
}//
	
void VBORenderManager::renderGridGeometry(QString geoName){
		
}//
////////////////////////////////////////////////////////////////////
// MODEL
void VBORenderManager::addStreetElementModel(QString name,ModelSpec mSpec){
	nameToVectorModels[name].push_back(mSpec);
}//
void VBORenderManager::renderAllStreetElementName(QString name){
	for(int i=0;i<nameToVectorModels[name].size();i++){
		VBOModel_StreetElements::renderOneStreetElement(program,nameToVectorModels[name][i]);
	}
	//printf("name %s --> %d\n",name.toAscii().constData(),nameToVectorModels[name].size());
}//
void VBORenderManager::removeAllStreetElementName(QString name){
	nameToVectorModels[name].clear();
}
	
