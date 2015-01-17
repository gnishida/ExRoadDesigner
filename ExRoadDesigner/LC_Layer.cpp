
#include "common.h"
#include "LC_Layer.h"
#include <QStringList>
#include "VBOUtil.h"
#include <QImage>
#include <QGLWidget>
	
Layer::Layer() {
	initialized = false;
	texData=0;
}

void Layer::init(const QVector3D& _minPos, const QVector3D& _maxPos, int _imgResX, int _imgResY) {
	minPos = _minPos;
	maxPos = _maxPos;
	imgResX = _imgResX;
	imgResY = _imgResY;
		
	//layerData=cv::Mat::zeros(imgResY,imgResX,CV_8UC1);
	//layerData = cv::Mat(imgResY,imgResX,CV_8UC1,10);
	layerData = cv::Mat(imgResY, imgResX, CV_32FC1, cv::Scalar(70.0f));

	updateTexFromData();

	initialized=true;
}

//////////////////////////////////////////////////////////////
//dreamincode.net/forums/topic/66480-perlin-noise/
inline double findnoise2(double x,double y){
	int n=(int)x+(int)y*57;
	n=(n<<13)^n;
	int nn=(n*(n*n*60493+19990303)+1376312589)&0x7fffffff;
	return 1.0-((double)nn/1073741824.0);
}//
inline double interpolate(double a,double b,double x){
	double ft=x * 3.1415927;
	double f=(1.0-cos(ft))* 0.5;
	return a*(1.0-f)+b*f;
}//

double noise(double x,double y){
	double floorx=(double)((int)x);//This is kinda a cheap way to floor a double integer.
	double floory=(double)((int)y);
	double s,t,u,v;//Integer declaration
	s=findnoise2(floorx,floory); 
	t=findnoise2(floorx+1,floory);
	u=findnoise2(floorx,floory+1);//Get the surrounding pixels to calculate the transition.
	v=findnoise2(floorx+1,floory+1);
	double int1=interpolate(s,t,x-floorx);//Interpolate between the values.
	double int2=interpolate(u,v,x-floorx);//Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
	return interpolate(int1,int2,y-floory);//Here we use y-floory, to get the 2nd dimension.
}//

void Layer::randomPerlineNoise(cv::Mat& perlinNoise){
	if(perlinNoise.rows==0||perlinNoise.cols==0){
		perlinNoise=cv::Mat::zeros(imgResY,imgResX,CV_8UC1);
	}
	int octaves=6;
	double zoom=75.0;//zoom in and out on it
	double p=0.6f;//roughness of the picture
	for(int r=0;r<perlinNoise.rows;r++){//Loops to loop trough all the pixels
		for(int c=0;c<perlinNoise.cols;c++){
			/*// flat center
			uchar centerH=60;//30;
			float rad=0.02;
			if(r>=perlinNoise.cols/2-perlinNoise.cols*rad&&r<=perlinNoise.cols/2+perlinNoise.cols*rad&&
				c>=perlinNoise.rows/2-perlinNoise.rows*rad&&c<=perlinNoise.rows/2+perlinNoise.rows*rad){
					
				perlinNoise.at<uchar>(r,c)=centerH;	
				continue;
			}*/
			double getnoise =0;
			for(int a=0;a<octaves-1;a++){//This loops trough the octaves.
				double frequency = pow(2.0,a);//This increases the frequency with every loop of the octave.
				double amplitude = pow(p,a);//This decreases the amplitude with every loop of the octave.
				getnoise += noise(((double)c)*frequency/zoom,((double)r)/zoom*frequency)*amplitude;//This uses our perlin noise functions. It calculates all our zoom and frequency and amplitude
			}//											It gives a decimal value, you know, between the pixels. Like 4.2 or 5.1
			int color= (int)((getnoise*128.0)+128.0)-40;//Convert to 0-256 values. //MAGIC 40 to decrease the average
			if(color>255)
				color=255;
			if(color<0)
				color=0;
			//SetPixel(ret,c,r,(int)((r/255.0)*(double)color),(int)((g/255.0)*(double)color),(int)((b/255.0)*(double)color));//This colours the image with the RGB values 
			perlinNoise.at<uchar>(r,c)=(uchar)(color);
		}//														   given at the beginning in the function.
	}
}
//////////////////////////////////////////////////////////////

	
// GEN: 1/16/2015. Change to not use a temporary file.
void Layer::updateTexFromData() {
	/*QImage img = QImage((uchar*)layerData.data, layerData.cols, layerData.rows, layerData.step, QImage::Format_Mono);
	if (img.isNull()) {
		printf("ERROR: GL_formatted_image\n");
		return;
	}*/

	if(texData!=0){
		glDeleteTextures(1,&texData);
		texData=0;
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	glGenTextures(1, &texData);
	glBindTexture(GL_TEXTURE_2D, texData);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, img.width(), img.height(), 0, GL_RED, GL_UNSIGNED_BYTE, img.bits());
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, layerData.cols, layerData.rows, 0, GL_RED, GL_UNSIGNED_BYTE, layerData.data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, layerData.cols, layerData.rows, 0, GL_RED, GL_FLOAT, layerData.data);

	glGenerateMipmap(GL_TEXTURE_2D);
}

/**
 * ガウシアン分布に基づく山を作成する。既存の高さの方が高い場所については、高さをそのままキープする。
 *
 * @param coordX			X座標 [0, 1]
 * @param coordY			Y座標 [0, 1]
 * @param height			ガウス分布の最大高さ
 * @param sigma				標準偏差
 */
void Layer::addGaussian(float coordX, float coordY, float height, float rad) {
	float x0 = coordX * imgResX;
	float y0 = coordY * imgResY;
	float sigma = rad * imgResX;

	for (int c = 0; c < layerData.cols; c++) {
		for (int r = 0; r < layerData.rows; r++) {
			float x = c + 0.5f;
			float y = r + 0.5f;

			float z = height * expf(-(SQR(x - x0) + SQR(y - y0)) / (2 * sigma * sigma));
			if (height > 0) {
				if (z > layerData.at<float>(r, c)) {
					layerData.at<float>(r,c) = z;
				}
			} else {
				if (z < layerData.at<float>(r, c)) {
					layerData.at<float>(r,c) = z;
				}
			}
		}
	}

	// update image
	updateTexFromData();
}

/**
 * ガウシアン分布に基づき、高さを上げる。
 *
 * @param coordX			X座標 [0, 1]
 * @param coordY			Y座標 [0, 1]
 * @param height			ガウス分布の最大高さ
 * @param sigma				標準偏差
 */
void Layer::updateGaussian(float coordX, float coordY, float height, float rad) {
	float x0 = coordX * imgResX;
	float y0 = coordY * imgResY;
	float sigma = rad * imgResX;

	for (int c = 0; c < layerData.cols; c++) {
		for (int r = 0; r < layerData.rows; r++) {
			float x = c + 0.5f;
			float y = r + 0.5f;

			float z = layerData.at<float>(r,c) + height * expf(-(SQR(x - x0) + SQR(y - y0)) / (2 * sigma * sigma));
			if (z < 0) z = 0.0f;
			layerData.at<float>(r,c) = z;
		}
	}

	// update image
	updateTexFromData();
}

void Layer::updateLayer(float coordX,float coordY,float change,float rad){
	float sigmaX,sigmaY;
	float x,y,x0,y0,A;

	x0=coordX*imgResX;//0-imgRes
	y0=coordY*imgResY;
	A=change;
	sigmaX=imgResX*rad/2.0f;
	sigmaY=imgResY*rad/2.0f;

	if(change==FLT_MAX){
		printf("Hack, flat area\n");
		// HACK Flat area
		//int flatValue=(int)layerData.at<uchar>(y0,x0);
		int flatValue=(int)layerData.at<float>(y0,x0);
		int radImgX=rad*imgResX;
		int radImgY=rad*imgResY;
		for(int c=0;c<layerData.cols;c++){
			for(int r=0;r<layerData.rows;r++){
				if(abs(c-x0)<radImgX&&abs(r-y0)<radImgY){
					layerData.at<float>(r,c)=flatValue;
				}
			}
		}
	}else{
		// Normal
		float diff;
		for(int c=0;c<layerData.cols;c++){
			for(int r=0;r<layerData.rows;r++){

				x=(0.5f+c);
				y=(0.5f+r);
				diff=(A*qExp(-( (((x-x0)*(x-x0))/(2*sigmaX*sigmaX))+ (((y-y0)*(y-y0))/(2*sigmaY*sigmaY)) )))*255.0f;//0-255
				int newV=std::floor((int)layerData.at<float>(r,c)+diff+0.5f);//floor +0.5--> round
				if(newV<0){
					newV=0;
				}
				layerData.at<float>(r,c)=newV;
			}
		}
	}
	// update image
	updateTexFromData();
}//

void Layer::updateLayerNewValue(float coordX,float coordY,float newValue,float rad){
	float sigmaX,sigmaY;
	float x,y,x0,y0,A;

	x0=coordX*imgResX;//0-imgRes
	y0=coordY*imgResY;
	sigmaX=imgResX*rad/2.0f;
	sigmaY=imgResY*rad/2.0f;
		

	int flatValue=(int)newValue;
	int radImg=rad*std::max<int>(imgResX,imgResY);

	printf("updateLayerNewValue x0 y0 %f %f \n");
	for(int c=0;c<layerData.cols;c++){
		for(int r=0;r<layerData.rows;r++){
			//if(abs(c-x0)<radImgX&&abs(r-y0)<radImgY){
			if(((c-x0)*(c-x0)+(r-y0)*(r-y0))<=radImg*radImg){
				layerData.at<float>(r,c)=flatValue;
			}
		}
	}
	// update image
	updateTexFromData();
}//

/*
float Layer::getValue(float xM,float yM){
	int c=xM*imgResX;//0-imgRes
	int r=yM*imgResY;
	if (c < 0) c = 0;
	if (c >= layerData.cols) c = layerData.cols - 1;
	if (r < 0) r = 0;
	if (r >= layerData.rows) r = layerData.rows - 1;
	return float((int)layerData.at<uchar>(r,c));
}
*/

// return the interpolated value
float Layer::getValue(float xM,float yM){
	int c1=floor(xM*imgResX);//0-imgRes
	int c2=c1+1;
	int r1=floor(yM*imgResY);
	int r2=r1+1;
	if (c1 < 0) c1 = 0;
	if (c1 >= layerData.cols) c1 = layerData.cols - 1;
	if (c2 < 0) c2 = 0;
	if (c2 >= layerData.cols) c2 = layerData.cols - 1;
	if (r1 < 0) r1 = 0;
	if (r1 >= layerData.rows) r1 = layerData.rows - 1;
	if (r2 < 0) r2 = 0;
	if (r2 >= layerData.rows) r2 = layerData.rows - 1;

	float v1 = layerData.at<float>(r1,c1);
	float v2 = layerData.at<float>(r2,c1);
	float v3 = layerData.at<float>(r1,c2);
	float v4 = layerData.at<float>(r2,c2);

	float v12,v34;
	if (yM*imgResY<=r1){
		v12 = v1;
		v34 = v3;
	} else if (yM*imgResY>=r2){
		v12 = v2;
		v34 = v4;
	} else {
		float s = yM*imgResY - r1;
		float t = r2 - yM*imgResY;
		v12 = (v1 * t + v2 * s) / (s + t);
		v34 = (v3 * t + v4 * s) / (s + t);
	}

	if (xM*imgResX<=c1){
		return v12;
	} else if (xM*imgResX>=c2){
		return v34;
	} else {
		float s = xM*imgResX - c1;
		float t = c2 - xM*imgResX;
		return (v12 * t + v34 * s) / (s + t);
	}
}

void Layer::loadLayer(QString& fileName){
	layerData=cv::imread(fileName.toAscii().constData(),0);//load one channel
	// update image
	updateTexFromData();
}//
	
void Layer::saveLayer(QString& fileName){
	cv::imwrite( fileName.toAscii().constData(), layerData );
}//

void Layer::smoothLayer(){
	int smoothV=std::max(imgResX/80,3);
	if(smoothV%2==0)smoothV++;
	printf("smoothV %d\n",smoothV);
	cv::blur(layerData,layerData,cv::Size(smoothV,smoothV));
	updateTexFromData();
}//

