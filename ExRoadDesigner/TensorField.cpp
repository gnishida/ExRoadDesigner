#include "Util.h"
#include "TopNSearch.h"
#include "TensorField.h"

void TensorField::init(const Polygon2D &area, const Polyline2D &hintLine, const Polyline2D &exampleHintLine) {
	this->area = area;
	this->bbox = area.envelope();

	mat = cv::Mat((int)bbox.dy(), (int)bbox.dx(), CV_32F);

	std::vector<float> rotationAngles;

	// B-splineの位置を計算
	Polyline2D bsplineHintLine;
	bsplineHintLine.push_back(hintLine[0]);
	for (int i = 1; i < hintLine.size() - 1; ++i) {
		QVector2D pt1 = (hintLine[i - 1] + hintLine[i]) * 0.5f;
		QVector2D pt2 = (hintLine[i] + hintLine[i + 1]) * 0.5f;
		bsplineHintLine.push_back((pt1 + pt2) * 0.5f);
	}
	bsplineHintLine.push_back(hintLine.last());

	// hintLine上の各点における、回転角度を計算
	for (int i = 0; i < hintLine.size(); ++i) {
		float angle;

		if (i > 0 && i < hintLine.size() - 1) {
			QVector2D ex_vec = exampleHintLine[i] - exampleHintLine[i - 1];
			QVector2D vec = hintLine[i] - hintLine[i - 1];
			float angle1 = Util::diffAngle(vec, ex_vec, false);

			ex_vec = exampleHintLine[i + 1] - exampleHintLine[i];
			vec = hintLine[i + 1] - hintLine[i];
			float angle2 = Util::diffAngle(vec, ex_vec, false);

			angle = (angle1 + angle2) * 0.5f;
		} else if (i > 0) {
			QVector2D ex_vec = exampleHintLine[i] - exampleHintLine[i - 1];
			QVector2D vec = hintLine[i] - hintLine[i - 1];

			angle = Util::diffAngle(vec, ex_vec, false);
		} else {
			QVector2D ex_vec = exampleHintLine[i + 1] - exampleHintLine[i];
			QVector2D vec = hintLine[i + 1] - hintLine[i];

			angle = Util::diffAngle(vec, ex_vec, false);
		}

		rotationAngles.push_back(angle);
	}

	for (int i = 0; i < mat.rows; ++i) {
		for (int j = 0; j < mat.cols; ++j) {
			int x = bbox.minPt.x() + j;
			int y = bbox.minPt.y() + i;

			//if (!area.contains(QVector2D(x, y))) continue;

			TopNSearch<int> tns;
			for (int k = 0; k < bsplineHintLine.size(); ++k) {
				float dist = (bsplineHintLine[k] - QVector2D(x, y)).lengthSquared();
				tns.add(dist, k);
			}

			QList<int> nearestPts = tns.topN(2, TopNSearch<int>::ORDER_ASC);

			float angle1 = rotationAngles[nearestPts[0]];
			float angle2 = rotationAngles[nearestPts[1]];

			float dist1 = (hintLine[nearestPts[0]] - QVector2D(x, y)).length();
			float dist2 = (hintLine[nearestPts[1]] - QVector2D(x, y)).length();

			float angle = (angle1 * dist2 + angle2 * dist1) / (dist1 + dist2);

			mat.at<float>(i, j) = angle;
		}
	}
}

float TensorField::get(const QVector2D &pt) {
	int x = pt.x() - bbox.minPt.x();
	int y = pt.y() - bbox.minPt.y();
	if (x < 0) x = 0;
	if (x >= mat.cols) x = mat.cols - 1;
	if (y < 0) y = 0;
	if (y >= mat.rows) y = mat.rows - 1;
	return mat.at<float>(y, x);
}

void TensorField::save(const QString &filename) {
	cv::Mat img((int)(mat.rows / 10), (int)(mat.cols / 10), CV_8U, cv::Scalar(0));
	for (int i = 0; i < (int)(img.rows/10); ++i) {
		for (int j = 0; j < (int)(img.cols/10); ++j) {
			float totalAngle = 0.0f;
			int num = 0;

			for (int v = 0; v < 100; ++v) {
				for (int u = 0; u < 100; ++u) {
					int x = j * 100 + u;
					int y = i * 100 + v;
					if (x >= mat.cols) continue;
					if (y >= mat.rows) continue;

					totalAngle += mat.at<float>(y, x);
					num++;
				}
			}

			if (num > 0) totalAngle /= (float)num;

			int x1 = j * 10 + 5 - cosf(totalAngle) * 4.0f;
			int y1 = i * 10 + 5 - sinf(totalAngle) * 4.0f;
			int x2 = j * 10 + 5 + cosf(totalAngle) * 4.0f;
			int y2 = i * 10 + 5 + sinf(totalAngle) * 4.0f;
			cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255), 1);
		}
	}

	cv::flip(img, img, 0);
	cv::imwrite(filename.toUtf8().data(), img);
}
