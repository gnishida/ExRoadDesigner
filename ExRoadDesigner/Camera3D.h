#pragma once

#include "glew.h"
#include "Camera.h"
#include "qvector4d.h"
#include "qmatrix4x4.h"

class Camera3D : public Camera{
public:
	QVector3D cam_pos; 
	QVector3D cam_view;

	float sensitivity;
	float walk_speed;

	QVector4D light_dir;

	Camera3D() {
		sensitivity = 0.002f;
		walk_speed = 15.0f;

		resetCamera();
	}

	void resetCamera(){
		//cam_pos=QVector3D(70.0, 500.0f, -900);
		cam_pos=QVector3D(0, 12000.0f, 0);
		//cam_view=QVector3D(-0.5f, -0.5f, 0.7f);
		cam_view=QVector3D(0.0f, -1.0f, 0.001f);

		light_dir=QVector4D(-0.5f,0.4f,-0.77f,0.0f);
		light_dir=QVector4D(-0.38f,0.86f,-0.34f,0.00f);
		light_dir=QVector4D(-0.40f,0.81f,-0.51f,0.00f);
		fovy = 60.0f;
	}//

	void Camera3D::rotate_view(QVector3D& view, float angle, float x, float y, float z){
		float new_x;
		float new_y;
		float new_z;

		float c = cos(angle);
		float s = sin(angle);

		new_x  = (x*x*(1-c) + c)	* view.x();
		new_x += (x*y*(1-c) - z*s)	* view.y();
		new_x += (x*z*(1-c) + y*s)	* view.z();

		new_y  = (y*x*(1-c) + z*s)	* view.x();
		new_y += (y*y*(1-c) + c)	* view.y();
		new_y += (y*z*(1-c) - x*s)	* view.z();

		new_z  = (x*z*(1-c) - y*s)	* view.x();
		new_z += (y*z*(1-c) + x*s)	* view.y();
		new_z += (z*z*(1-c) + c)	* view.z();

		view=QVector3D(new_x,new_y,new_z);
		view.normalize();
	}//

	void motion(int dx, int dy,bool moveLight=false){
		float rot_x, rot_y;
		QVector3D rot_axis;

		rot_x = -dx *sensitivity;//- to invert
		rot_y = dy *sensitivity;

		if(moveLight==true)	{
			printf("1Move light %f %f %f %f\n",light_dir.x(),light_dir.y(),light_dir.z());
			light_dir.setY(light_dir.y()+ rot_y);
			if(light_dir.y() < 0.2f)
				light_dir.setY(0.2f);
			light_dir.normalize();
			QVector3D light_dir3D=light_dir.toVector3D();
			rotate_view(light_dir3D, -rot_x, cam_view.x(), cam_view.y(), cam_view.z());
			light_dir=QVector4D(light_dir3D.x(),light_dir3D.y(),light_dir3D.z(),light_dir.w());
			printf("2Move light %f %f %f %f\n",light_dir.x(),light_dir.y(),light_dir.z());
		}else{
			rotate_view(cam_view, rot_x, 0.0f, 1.0f, 0.0f);
			rot_axis=QVector3D(-cam_view.z(),0,cam_view.x());
			rot_axis.normalize();
			rotate_view(cam_view, rot_y, rot_axis.x(), rot_axis.y(), rot_axis.z());
		}
	}//

	void updateCamMatrix() {

		mvMatrix.setToIdentity();
		QVector3D dirV=cam_pos+ cam_view;
		mvMatrix.lookAt(QVector3D(cam_pos.x(),cam_pos.z(),cam_pos.y()),
			//cam_pos+ cam_view,
			QVector3D(dirV.x(),dirV.z(),dirV.y()),
			QVector3D(0.0f, 0.0f, 1.0f));
		// normal matrix
		normalMatrix=mvMatrix.normalMatrix();
		// mvp
		mvpMatrix=pMatrix*mvMatrix;
	}//

	void Camera3D::moveKey(int typeMode){
		if(typeMode==0){//GetAsyncKeyState('W')){
			cam_pos.setX(cam_pos.x()+ cam_view.x() * walk_speed);
			cam_pos.setY(cam_pos.y()+ cam_view.y() * walk_speed);
			cam_pos.setZ(cam_pos.z()+ cam_view.z() * walk_speed);
		}
		if(typeMode==1){//if(GetAsyncKeyState('S')){
			cam_pos.setX(cam_pos.x() - cam_view.x() * walk_speed);
			cam_pos.setY(cam_pos.y() - cam_view.y() * walk_speed);
			cam_pos.setZ(cam_pos.z() - cam_view.z() * walk_speed);
		}
		if(typeMode==2){//if(GetAsyncKeyState('A')){
			cam_pos.setX(cam_pos.x()+cam_view.z() * walk_speed);
			cam_pos.setZ(cam_pos.z()-cam_view.x() * walk_speed);		
		}
		if(typeMode==3){//if(GetAsyncKeyState('D')){
			cam_pos.setX(cam_pos.x()-cam_view.z() * walk_speed);
			cam_pos.setZ(cam_pos.z()+cam_view.x() * walk_speed);
		}

		if(typeMode==4){//if(GetAsyncKeyState(VK_SPACE)){
			cam_pos.setY( cam_pos.y()+ walk_speed);
		}
	}//
};

