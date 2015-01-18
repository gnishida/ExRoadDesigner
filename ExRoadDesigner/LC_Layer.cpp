
#include "common.h"
#include "LC_Layer.h"
#include <QStringList>
#include "VBOUtil.h"
#include <QImage>
#include <QGLWidget>
	
Layer::Layer() {
	initialized = false;
}

void Layer::init(const QVector3D& _minPos, const QVector3D& _maxPos, int _imgResX, int _imgResY) {
	minPos = _minPos;
	maxPos = _maxPos;
	imgResX = _imgResX;
	imgResY = _imgResY;
		
	layerData = cv::Mat(imgResY, imgResX, CV_32FC1, cv::Scalar(70.0f));

	initialized = true;
}
	
/**
 * ガウシアン分布に基づく山を作成する。
 * 指定した座標を中心としてガウス分布に基づき高さを決定する。
 * ただし、既存の高さの方が高い場所については、既存の高さをそのままキープする。
 *
 * @param u				中心のX座標 [0, 1]
 * @param v				中心のY座標 [0, 1]
 * @param height		ガウス分布の最大高さ
 * @param rad_ratio		半径のサイズ（グリッドサイズに対する比）
 */
void Layer::addGaussian(float u, float v, float height, float rad_ratio) {
	float x0 = u * imgResX;
	float y0 = v * imgResY;
	float sigma = rad_ratio * imgResX;

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
}

/**
 * ガウシアン分布に基づき、高さを上げる。
 *
 * @param u				中心のX座標 [0, 1]
 * @param v				中心のY座標 [0, 1]
 * @param height		ガウス分布の最大高さ
 * @param rad_ratio		半径のサイズ（グリッドサイズに対する比）
 */
void Layer::updateGaussian(float u, float v, float height, float rad_ratio) {
	float x0 = u * imgResX;
	float y0 = v * imgResY;
	float sigma = rad_ratio * imgResX;

	for (int c = 0; c < layerData.cols; c++) {
		for (int r = 0; r < layerData.rows; r++) {
			float x = c + 0.5f;
			float y = r + 0.5f;

			float z = layerData.at<float>(r,c) + height * expf(-(SQR(x - x0) + SQR(y - y0)) / (2 * sigma * sigma));
			if (z < 0) z = 0.0f;
			layerData.at<float>(r,c) = z;
		}
	}
}

/**
 * 指定した位置を中心とする円の範囲の高さを、指定した高さheightにする。
 *
 * @param u				中心のX座標 [0, 1]
 * @param v				中心のY座標 [0, 1]
 * @param height		この高さに設定する
 * @param rad_ratio		半径のサイズ（グリッドサイズに対する比）
 */
void Layer::excavate(float u, float v, float height, float rad_ratio) {
	float x0 = u * imgResX;
	float y0 = v * imgResY;
	float rad = rad_ratio * imgResX;

	for (int c = x0 - rad; c <= x0 + rad + 1; ++c) {
		if (c < 0 || c >= layerData.cols) continue;
		for (int r = y0 - rad; r <= y0 + rad + 1; ++r) {
			if (r < 0 || r >= layerData.rows) continue;

			float x = c + 0.5f;
			float y = r + 0.5f;

			if (SQR(x - x0) + SQR(y - y0) > SQR(rad)) continue;
			
			layerData.at<float>(r, c) = 0.0f;
		}
	}
}

/**
 * Return the interpolated value for a given (u, v) coordinate.
 *
 * @param u		u coordinate [0, 1]
 * @param v		v coordinate [0, 1]
 */
float Layer::getValue(float u, float v) {
	if (u < 0) u = 0.0f;
	if (u >= 1.0f) u = 1.0f;
	if (v < 0) v = 0.0f;
	if (v >= 1.0f) v = 1.0f;

	int c1 = u * (imgResX - 1);
	int c2 = u * (imgResX - 1) + 1;
	int r1 = v * (imgResY - 1);
	int r2 = v * (imgResY - 1) + 1;

	if (c1 < 0) c1 = 0;
	if (c2 >= layerData.cols) c2 = layerData.cols - 1;
	if (r1 < 0) r1 = 0;
	if (r2 >= layerData.rows) r2 = layerData.rows - 1;

	float v1 = layerData.at<float>(r1, c1);
	float v2 = layerData.at<float>(r2, c1);
	float v3 = layerData.at<float>(r1, c2);
	float v4 = layerData.at<float>(r2, c2);

	float v12,v34;
	if (r2 == r1) {
		v12 = v1;
		v34 = v3;
	} else {
		float t = v * imgResY - r1;
		v12 = v1 * (1-t) + v2 * t;
		v34 = v3 * (1-t) + v4 * t;
	}

	if (c2 == c1) {
		return v12;
	} else {
		float s = u * imgResX - c1;
		return v12 * (1-s) + v34 * s;
	}
}

/**
 * 無理やりUCHAR型4チャンネルで保存したデータをFloat型の1チャンネルデータとしてロードする。
 *
 * @param fileName	 file name
 */
void Layer::loadLayer(const QString& fileName) {
	cv::Mat loadImage = cv::imread(fileName.toUtf8().data(), CV_LOAD_IMAGE_UNCHANGED);
	cv::Mat tmp = cv::Mat(loadImage.rows, loadImage.cols, CV_32FC1, loadImage.data);
	tmp.copyTo(layerData);
}

/**
 * Float型の1チャンネルデータを、無理やりUCHAR型4チャンネルで保存する。
 *
 * @param fileName	 file name
 */
void Layer::saveLayer(const QString& fileName) {
	cv::Mat saveImage	= cv::Mat(layerData.rows, layerData.cols, CV_8UC4, layerData.data);
	cv::imwrite(fileName.toUtf8().data(), saveImage);
}

