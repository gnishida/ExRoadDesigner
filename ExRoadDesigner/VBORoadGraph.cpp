/************************************************************************************************
*
*		@desc Class containing the road graph
*		@author igarciad
*
************************************************************************************************/

#include "VBORoadGraph.h"

#include "VBORenderManager.h"
#include "polygon3D.h"
#include "global.h"



void VBORoadGraph::clearVBORoadGraph(VBORenderManager& rendManager){
	rendManager.removeStaticGeometry("3d_roads");
}//


void VBORoadGraph::updateRoadGraph(VBORenderManager& rendManager,RoadGraph &roadGraph){
	//////////////////////////////////////////
	// LINES QUADS
	clearVBORoadGraph(rendManager);
	RoadEdgeIter ei, eiEnd;
	QVector3D p0,p1;
	int numEdges=0;

	std::vector<Vertex> vertROAD;
	std::vector<Vertex> vertINT;
	QVector3D a0,a1,a2,a3;
	float width=3.5f;
	BBox3D roadBB;
	for(boost::tie(ei, eiEnd) = boost::edges(roadGraph.graph);
		ei != eiEnd; ++ei)
	{
		if (!roadGraph.graph[*ei]->valid) continue;
		numEdges++;
		/*QVector3D col;
		if( roadGraph.graph[*ei].numberOfLanes==LC::misctools::Global::global()->cuda_arterial_numLanes){//.isArterial() ){
			glLineWidth(5.0f);
			col=QVector3D(1.0f, 0.99f, 0.54f);//yellow
		} else {
			glLineWidth(2.0f);
			col=QVector3D(0.9f, 0.9f, 0.9f);//white	
		}*/

		RoadEdgePtr edge = roadGraph.graph[*ei];
		
		p0 = edge->polyline.front();//roadGraph.graph[boost::source(*ei, roadGraph.graph)].pt;
		p1 = edge->polyline.back();//roadGraph.graph[boost::target(*ei, roadGraph.graph)].pt;
		p0.setZ(1.0f);
		p1.setZ(1.0f);

		roadBB.addPoint(p0);

		QVector3D dir=(p1-p0);//.normalized();
		float length=dir.length();
		dir/=length;
		QVector3D per=(QVector3D::crossProduct(dir,QVector3D(0,0,1.0f)).normalized());

		int numSeg=0;
		const float intersectionClearance=3.5f;

		/// no room for one line
		if(length<2.0f*intersectionClearance){//no real line
			a0=QVector3D(p0.x()+per.x()*width, p0.y()+per.y()*width, p0.z());
			a1=QVector3D(p1.x()+per.x()*width, p1.y()+per.y()*width, p0.z());

			a2=QVector3D(p1.x()-per.x()*width, p1.y()-per.y()*width, p0.z());
			a3=QVector3D(p0.x()-per.x()*width, p0.y()-per.y()*width, p0.z());

			vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
			vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
			vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
			vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));
		}else{
			QVector3D p0_,p1_;
			// START
			p0_=p0;
			p1_=p0+dir*intersectionClearance;
			a0=QVector3D(p0_.x()+per.x()*width, p0_.y()+per.y()*width, p0_.z());
			a1=QVector3D(p1_.x()+per.x()*width, p1_.y()+per.y()*width, p0_.z());

			a2=QVector3D(p1_.x()-per.x()*width, p1_.y()-per.y()*width, p0_.z());
			a3=QVector3D(p0_.x()-per.x()*width, p0_.y()-per.y()*width, p0_.z());

			vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
			vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
			vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
			vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));

			// MIDDLE

			float middLenght=length-2.0f*intersectionClearance;
			float const maxSegmentLeng=5.0f;
			float segmentLeng;
			if(middLenght>2*maxSegmentLeng){
				int numSegments=ceil(length/5.0f);
				float lengthMoved=0;
				float dW=(2*width);
				for(int nS=0;nS<numSegments;nS++){
					segmentLeng=std::min(maxSegmentLeng,middLenght);
					a0=a1;
					a3=a2;
					a1=a1+dir*(segmentLeng);
					a2=a2+dir*(segmentLeng);
					vertROAD.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,lengthMoved/dW,0.0f)));
					vertROAD.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,(segmentLeng+lengthMoved)/dW,0)));
					vertROAD.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,(segmentLeng+lengthMoved)/dW,0)));
					vertROAD.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,lengthMoved/dW,0)));
					lengthMoved+=segmentLeng;
					middLenght-=segmentLeng;
				}

			}else{
				// JUST ONE MIDDLE SEGMENT
				a0=a1;
				a3=a2;
				a1=a1+dir*(middLenght);
				a2=a2+dir*(middLenght);

				vertROAD.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,0.0f,0)));
				vertROAD.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),QVector3D(0,middLenght/(2*width),0)));
				vertROAD.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,middLenght/(2*width),0)));
				vertROAD.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),QVector3D(1.0f,0.0f,0)));
			}
			// END
			a0=a1;
			a3=a2;

			a1=a1+dir*intersectionClearance;
			a2=a2+dir*intersectionClearance;

			vertINT.push_back(Vertex(a0,QVector3D(),QVector3D(0,0,1.0f),a0));
			vertINT.push_back(Vertex(a1,QVector3D(),QVector3D(0,0,1.0f),a1));
			vertINT.push_back(Vertex(a2,QVector3D(),QVector3D(0,0,1.0f),a2));
			vertINT.push_back(Vertex(a3,QVector3D(),QVector3D(0,0,1.0f),a3));

		}
	}

	

	// add all geometry
	if(G::global()["3d_render_mode"]==0){//normal
		rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1.png",GL_QUADS, 2);//|LC::mode_AdaptTerrain);
		rendManager.addStaticGeometry("3d_roads",vertROAD,"../data/textures/street_0.png",GL_QUADS,2);//|LC::mode_AdaptTerrain);
	}
	if(G::global()["3d_render_mode"]==1){//hatch
		rendManager.addStaticGeometry("3d_roads",vertINT,"../data/textures/street_1b.png",GL_QUADS, 2);//|LC::mode_AdaptTerrain);
		rendManager.addStaticGeometry("3d_roads",vertROAD,"../data/textures/street_0b.png",GL_QUADS,2);//|LC::mode_AdaptTerrain);
		// add bbox
		/*std::vector<Vertex> vertBB;
		vertBB.push_back(Vertex(QVector3D(roadBB.minPt.x(),roadBB.minPt.y(),0),QVector3D(0.9f,0.9f,0.9f),QVector3D(0,0,1.0f),a0));
		vertBB.push_back(Vertex(QVector3D(roadBB.maxPt.x(),roadBB.minPt.y(),0),QVector3D(0.9f,0.9f,0.9f),QVector3D(0,0,1.0f),a3));
		vertBB.push_back(Vertex(QVector3D(roadBB.maxPt.x(),roadBB.maxPt.y(),0),QVector3D(0.9f,0.9f,0.9f),QVector3D(0,0,1.0f),a2));
		vertBB.push_back(Vertex(QVector3D(roadBB.minPt.x(),roadBB.maxPt.y(),0),QVector3D(0.9f,0.9f,0.9f),QVector3D(0,0,1.0f),a1));
		rendManager.addStaticGeometry("3d_roads",vertBB,"",GL_QUADS,1);//|LC::mode_AdaptTerrain);*/
		std::vector<QVector3D> vertBB;
		vertBB.push_back(QVector3D(roadBB.minPt.x(),roadBB.minPt.y(),0));
		vertBB.push_back(QVector3D(roadBB.maxPt.x(),roadBB.minPt.y(),0));
		vertBB.push_back(QVector3D(roadBB.maxPt.x(),roadBB.maxPt.y(),0));
		vertBB.push_back(QVector3D(roadBB.minPt.x(),roadBB.maxPt.y(),0));
		//rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"hatching_array",GL_QUADS,10|mode_TexArray,QVector3D(1.0f,1.0f,1.0f),QVector3D(0,0,0));
		//rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"hatching_array",GL_QUADS,11|mode_TexArray|mode_Lighting,QVector3D(1.0f,1.0f,1.0f),QVector3D(0,0,0));
		//rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"hatching_array",GL_QUADS,11|mode_TexArray|mode_Lighting,QVector3D(1.0f/(roadBB.maxPt.x()-roadBB.minPt.x()),1.0f/(roadBB.maxPt.y()-roadBB.minPt.y()),1.0f),QVector3D(0,0,0));
		rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"hatching_array",GL_QUADS,11|mode_TexArray|mode_Lighting,QVector3D(1.0f/(5.0f),1.0f/(5.0f),1.0f),QVector3D(0,0,0));
		//rendManager.addStaticGeometry2("3d_roads",vertBB,0,false,"../data/textures/LC/hatch/h6b.png",GL_QUADS,2,QVector3D(1.0f,1.0f,1.0f),QVector3D(0,0,0));
	}
	
	

}//


void VBORoadGraph::renderRoadGraphLines(VBORenderManager& rendManager){

	/*//glLineWidth(6.0f);
	glLineWidth(6.0f);
	GLuint vao;
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);
	//
	if(line_wasModified==true){// it was modified
	if(linesVBO!=INT_MAX){//it was already created--> remove
	glDeleteVertexArrays(1, &linesVBO);
	}
	glGenVertexArrays(1,&linesVBO);
	glBindBuffer(GL_ARRAY_BUFFER,linesVBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(Vertex)*line_Vert.size(),line_Vert.data(),GL_STATIC_DRAW);
	line_wasModified=false;
	}else{
	glBindBuffer(GL_ARRAY_BUFFER,linesVBO);
	}

	glUniform1i (glGetUniformLocation (rendManager.program, "terrain_tex"), 7);//tex0: 0
	glUniform4f (glGetUniformLocation (rendManager.program, "terrain_size"), 
	rendManager.vboTerrain.terrainLayer.minPos.x(),
	rendManager.vboTerrain.terrainLayer.minPos.y(),
	(rendManager.vboTerrain.terrainLayer.maxPos.x()-rendManager.vboTerrain.terrainLayer.minPos.x()), 
	(rendManager.vboTerrain.terrainLayer.maxPos.y()-rendManager.vboTerrain.terrainLayer.minPos.y())
	);//tex0: 0

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),0);
	glEnableVertexAttribArray(1);
	VBOUtil::check_gl_error("aa editionMode");
	glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)(9*sizeof(float)));
	glUniform1i (glGetUniformLocation (rendManager.program, "mode"), 1|LC::mode_AdaptTerrain);//MODE: color


	//RendUtil::enableAttributesInShader(programId);
	glDrawArrays(GL_LINES,0,line_Vert.size());
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glBindVertexArray(0);
	glDeleteVertexArrays(1,&vao);*/
}//

