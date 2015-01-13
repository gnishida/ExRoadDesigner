#include "HoughTransform.h"

#define SQR(x)		((x) * (x))

HoughTransform::HoughTransform(const Polygon2D& area, float scale) {
	this->area = area;
	this->scale = scale;

	bbox = area.envelope();
	htSpace = cv::Mat::zeros(bbox.dy() * scale, bbox.dx() * scale, CV_32F);

	std::cout << "HoughTransform: (" << htSpace.rows << " x " << htSpace.cols << ")" << std::endl;
}

/**
 * オリジナルの座標系での頂点座標v1とv2を通る直線を、縮尺された座標系のhtSpace上に描画する。
 */
void HoughTransform::line(const QVector2D& p1, const QVector2D& p2, float sigma) {
	QVector2D v1 = (p1 - bbox.minPt) * scale;
	QVector2D v2 = (p2 - bbox.minPt) * scale;

	QVector2D vl, vr;
	if (v1.x() < v2.x()) {
		vl = v1;
		vr = v2;
	} else {
		vl = v2;
		vr = v1;
	}

	QVector2D vu, vd;
	if (v1.y() < v2.y()) {
		vd = v1;
		vu = v2;
	} else {
		vd = v2;
		vu = v1;
	}

	QVector2D dir = v2 - v1;
	float len = dir.length();

	sigma *= scale;
	float sigma2 = SQR(sigma);

	if (fabs(dir.x()) > fabs(dir.y())) {
		for (int x = 0; x < htSpace.cols; x++) {
			int y = dir.y() * ((float)x - v1.x()) / dir.x() + v1.y() + 0.5f;
			if (y < 0 || y >= htSpace.rows) continue;

			float h = 0;
			if (x >= vl.x() && x <= vr.x()) {
				h = len;
			} else if (x < vl.x()) {
				h = len * expf(-(SQR(x - vl.x()) + SQR(y - vl.y())) * 0.5f / sigma2);
			} else {
				h = len * expf(-(SQR(x - vr.x()) + SQR(y - vr.y())) * 0.5f / sigma2);
			}

			htSpace.at<float>(y, x) += h;
		}
	} else {
		for (int y = 0; y < htSpace.rows; y++) {
			int x = dir.x() * ((float)y - v1.y()) / dir.y() + v1.x() + 0.5f;
			if (x < 0 || x >= htSpace.cols) continue;

			float h = 0;
			if (y >= vd.y() && y <= vu.y()) {
				h = len;
			} else if (y < vd.y()) {
				h = len * expf(-(SQR(x - vd.x()) + SQR(y - vd.y())) * 0.5f / sigma2);
			} else {
				h = len * expf(-(SQR(x - vu.x()) + SQR(y - vu.y())) * 0.5f / sigma2);
			}

			htSpace.at<float>(y, x) += h;
		}
	}
}
/*
void HoughTransform::line(const QVector2D& p1, const QVector2D& p2, float sigma) {
	QVector2D v1 = (p1 - bbox.minPt) * scale;
	QVector2D v2 = (p2 - bbox.minPt) * scale;

	QVector2D vl, vr;
	if (v1.x() < v2.x()) {
		vl = v1;
		vr = v2;
	} else {
		vl = v2;
		vr = v1;
	}

	QVector2D vu, vd;
	if (v1.y() < v2.y()) {
		vd = v1;
		vu = v2;
	} else {
		vd = v2;
		vu = v1;
	}

	QVector2D dir = v2 - v1;
	float len = dir.length();

	float sigma2 = SQR(sigma);

	if (fabs(dir.x()) > fabs(dir.y())) {
		for (int x = 0; x < htSpace.cols; x++) {
			int y = dir.y() * ((float)x - v1.x()) / dir.x() + v1.y() + 0.5f;
			if (y < 0 || y >= htSpace.rows) continue;

			float h = 0;
			if (x >= vl.x() && x <= vr.x()) {
				h = len;
			} else if (SQR(x - v1.x()) + SQR(y - v1.y()) <= sigma2 || SQR(x - v2.x()) + SQR(y - v2.y()) <= sigma2) {
				h = len;
			}

			htSpace.at<float>(y, x) += h;
		}
	} else {
		for (int y = 0; y < htSpace.rows; y++) {
			int x = dir.x() * ((float)y - v1.y()) / dir.y() + v1.x() + 0.5f;
			if (x < 0 || x >= htSpace.cols) continue;

			float h = 0;
			if (y >= vd.y() && y <= vu.y()) {
				h = len;
			} else if (SQR(x - v1.x()) + SQR(y - v1.y()) <= sigma2 || SQR(x - v2.x()) + SQR(y - v2.y()) <= sigma2) {
				h = len;
			}

			htSpace.at<float>(y, x) += h;
		}
	}
}
*/
/**
 * オリジナルの座標系での頂点座標v1とv2を通る直線を、縮尺された座標系のhtSpace上に描画する。
 */
void HoughTransform::circle(const QVector2D& p1, const QVector2D& p2, float sigma) {
	QVector2D v1 = (p1 - bbox.minPt) * scale;
	QVector2D v2 = (p2 - bbox.minPt) * scale;


	sigma *= scale;
	float sigma2 = SQR(sigma);
	
	int size[3];
	size[0] = bbox.dy() * scale;
	size[1] = bbox.dx() * scale;
	size[2] = (int)sigma;
	cv::Mat htCircle = cv::Mat(3, size, CV_32F, cv::Scalar(0));

	float len = (v2 - v1).length();
	if (len > sigma) len = sigma;
	
	int x0 = std::max((int)(v1.x() - sigma), 0);
	int x1 = std::min((int)(v1.x() + sigma), htSpace.cols - 1);
	int y0 = std::max((int)(v1.y() - sigma), 0);
	int y1 = std::min((int)(v1.y() + sigma), htSpace.rows - 1);

	for (int x = x0; x <= x1; x++) {
		for (int y = y0; y <= y1; y++) {
			float r = sqrtf(SQR(x - v1.x()) + SQR(y - v1.y()));
			if (r >= (int)sigma) continue;

			htCircle.at<float>(y, x, (int)r) += 1;
		}
	}
	std::cout << "voting ok" << std::endl;

	// project the maximum voting for each (x, y)
	//cv::reduce(htCircle, htCircle, 2, CV_REDUCE_MAX);
	//htSpace += htCircle;
	for (int v = 0; v < size[0]; ++v) {
		for (int u = 0; u < size[1]; ++u) {
			float max_value = 0.0f;
			for (int w = 0; w < size[2]; ++w) {
				if (htCircle.at<float>(v, u, w) > max_value) {
					max_value = htCircle.at<float>(v, u, w);
				}
			}

			htSpace.at<float>(v, u) += max_value;
		}
	}
}

QVector2D HoughTransform::maxPoint() const {
	QVector2D ret;

	float max_value = 0.0f;

	for (int v = 0; v < htSpace.rows; v++) {
		for (int u = 0; u < htSpace.cols; u++) {
			if (htSpace.at<float>(v, u) > max_value) {
				max_value = htSpace.at<float>(v, u);
				ret.setX(u + 0.5f);
				ret.setY(v + 0.5f);
			}
		}
	}

	std::cout << "max_value: " << max_value << std::endl;

	// 投票結果を画像として保存する
	cv::Mat m;
	cv::flip(htSpace, m, 0);
	m /= (max_value / 255.0f);
	m.convertTo(m, CV_8U);
	cv::imwrite(QString("result%1.jpg").arg(scale).toUtf8().data(), m);


	ret /= scale;
	ret += bbox.minPt;

	return ret;
}

std::vector<QVector2D> HoughTransform::points(float threshold) const {
	maxPoint();

	std::vector<QVector2D> ret;

	for (int v = 0; v < htSpace.rows; v++) {
		for (int u = 0; u < htSpace.cols; u++) {
			if (htSpace.at<float>(v, u) > threshold) {
				QVector2D pt(u + 0.5f, v + 0.5f);
				pt /= scale;
				pt += bbox.minPt;

				ret.push_back(pt);
			}
		}
	}

	return ret;
}
