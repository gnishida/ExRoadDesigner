#include "Util.h"
#include "TopNSearch.h"
#include "TensorField.h"

void TensorField::init(const Polygon2D &area, const Polyline2D &hintLine, std::vector<ExFeature> &features, float scale) {
	this->scale = scale;
	this->bbox = area.envelope();

	int numPointsPerEx = hintLine.size() / features.size();

	int rows = bbox.dy() * scale + 1;
	int cols = bbox.dx() * scale + 1;
	//mat = cv::Mat((int)bbox.dy(), (int)bbox.dx(), CV_32F);
	mat = cv::Mat(rows, cols, CV_32F);

	std::vector<float> rotationAngles;

	// compute the knots of B-spline
	Polyline2D bsplineHintLine;
	bsplineHintLine.push_back(hintLine[0]);
	for (int i = 1; i < hintLine.size() - 1; ++i) {
		QVector2D pt1 = (hintLine[i - 1] + hintLine[i]) * 0.5f;
		QVector2D pt2 = (hintLine[i] + hintLine[i + 1]) * 0.5f;
		bsplineHintLine.push_back((pt1 + pt2) * 0.5f);
	}
	bsplineHintLine.push_back(hintLine.last());

	// compute the rotation angle at each hintLine points
	for (int i = 0; i <features.size(); ++i) {
		for (int j = 0; j < numPointsPerEx; ++j) {
			int index = i * numPointsPerEx + j;

			float angle;

			if (index > 0 && index < hintLine.size() - 1) {
				QVector2D ex_vec = features[i].hintLine[j] - features[i].hintLine[j - 1];
				QVector2D vec = hintLine[index] - hintLine[index - 1];

				float angle1 = Util::diffAngle(vec, ex_vec, false);
				QVector2D diffVec1(cosf(angle1), sinf(angle1));

				ex_vec = features[i].hintLine[j + 1] - features[i].hintLine[j];
				vec = hintLine[index + 1] - hintLine[index];

				float angle2 = Util::diffAngle(vec, ex_vec, false);
				QVector2D diffVec2(cosf(angle2), sinf(angle2));

				QVector2D averagedVec = (diffVec1 + diffVec2) * 0.5f;

				angle = atan2f(averagedVec.y(), averagedVec.x());
			} else if (index > 0) {
				QVector2D ex_vec = features[i].hintLine[j] - features[i].hintLine[j - 1];
				QVector2D vec = hintLine[index] - hintLine[index - 1];

				angle = Util::diffAngle(vec, ex_vec, false);
			} else {
				QVector2D ex_vec = features[i].hintLine[j + 1] - features[i].hintLine[j];
				QVector2D vec = hintLine[index + 1] - hintLine[index];

				angle = Util::diffAngle(vec, ex_vec, false);
			}

			rotationAngles.push_back(angle);
		}
	}

	for (int i = 0; i < mat.rows; ++i) {
		for (int j = 0; j < mat.cols; ++j) {
			int x = bbox.minPt.x() + j / scale;
			int y = bbox.minPt.y() + i / scale;

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

			QVector2D vec1(cosf(angle1), sinf(angle1));
			QVector2D vec2(cosf(angle2), sinf(angle2));

			QVector2D averagedVec = (vec1 * dist2 + vec2 * dist1) / (dist1 + dist2);

			//float angle = (angle1 * dist2 + angle2 * dist1) / (dist1 + dist2);
			float angle = atan2f(averagedVec.y(), averagedVec.x());

			mat.at<float>(i, j) = angle;
		}
	}

	// average them
	/*
	std::cout << "tensor field averaging start..." << std::endl;
	bool converged = false;
	while (!converged) {
		converged = true;

		cv::Mat newMat;
		mat.copyTo(newMat);

		for (int r = 0; r < mat.rows; ++r) {
			for (int c = 0; c < mat.cols; ++c) {
				float data = mat.at<float>(r, c);

				float avg = getAverage(r, c);

				if (fabs(data - avg) > 0.01f) {
					converged = false;
					newMat.at<float>(r, c) = avg;
				}
			}
		}

		newMat.copyTo(mat);
	}
	std::cout << "tensor field averaging done." << std::endl;
	*/
}

float TensorField::get(const QVector2D &pt) {
	int x = (pt.x() - bbox.minPt.x()) * scale;
	int y = (pt.y() - bbox.minPt.y()) * scale;
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

float TensorField::getAverage(int row, int col) {
	int count = 0;
	float total = 0.0f;

	for (int r = row - 1; r < row + 1; ++r) {
		for (int c = col - 1; c < col + 1; ++c) {
			if (r < 0 || r >= mat.rows) continue;
			if (c < 0 || c >= mat.cols) continue;

			total += mat.at<float>(r, c);
			count++;
		}
	}

	if (count == 0) return 0.0f;
	else return total / (float)count;
}
