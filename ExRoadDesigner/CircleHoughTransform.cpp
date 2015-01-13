#include "CircleHoughTransform.h"
#include "GraphUtil.h"

std::vector<RoadEdgeDescs> CircleHoughTransform::detect(RoadGraph &roads, float scale) {
	BBox bbox = GraphUtil::bbox(roads);
	int size[3];
	size[0] = bbox.dx() * scale + 1;
	size[1] = bbox.dy() * scale + 1;
	size[2] = std::min(size[0], size[1]) * 0.15;

	// build the radius computation table
	cv::Mat radiusTable = cv::Mat::zeros(size[0], size[1], CV_32F);
	for (int x = 0; x < size[0]; ++x) {
		for (int y = 0; y < size[1]; ++y) {
			radiusTable.at<float>(x, y) = int(sqrtf(SQR(x) + SQR(y)));
		}
	}

	cv::Mat space;
	space.create(3, size, CV_32F);
	space = cv::Scalar(0);

	// 各ピクセルが、訪問済みかどうかチェックするためのテーブル
	cv::Mat visited = cv::Mat::zeros(size[0], size[1], CV_8U);

	{
		int idx[3];

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 10.0f);

			for (int k = 0; k < polyline.size(); ++k) {
				if (!bbox.contains(polyline[k])) continue;

				int x = (polyline[k].x() - bbox.minPt.x()) * scale;
				int y = (polyline[k].y() - bbox.minPt.y()) * scale;

				if (visited.at<uchar>(x, y) > 0) continue;
				visited.at<uchar>(x, y) = 1;

				for (idx[0] = 0; idx[0] < size[0]; ++idx[0]) {
					for (idx[1] = 0; idx[1] < size[1]; ++idx[1]) {
						// compute the radius by using the lookup table
						idx[2] = (int)radiusTable.at<float>((int)abs(idx[0] - x), (int)abs(idx[1] - y));
						if (idx[2] < 3 || idx[2] >= size[2]) continue;

						// vote for the center of a circle
						space.at<float>(idx) ++;
					}
				}
			}
		}
	}

	// normalize the votes according to the radius
	// (votes = votes / radius)
	{
		int idx[3];
		for (idx[0] = 0; idx[0] < size[0]; ++idx[0]) {
			for (idx[1] = 0; idx[1] < size[1]; ++idx[1]) {
				for (idx[2] = 2; idx[2] < size[2]; ++idx[2]) {
					space.at<float>(idx) /= (float)idx[2];
				}
			}
		}
	}

	QMap<RoadEdgeDesc, bool> usedEdges;

	// list up all the centers that have votes more than 100
	std::vector<DetectedCircle> circles;
	for (int loop = 0; loop < 20; ++loop) {
		float max_count = 0;
		int max_x;
		int max_y;
		float max_r;

		int idx[3];
		for (idx[0] = 0; idx[0] < size[0]; ++idx[0]) {
			for (idx[1] = 0; idx[1] < size[1]; ++idx[1]) {
				for (idx[2] = 0; idx[2] < size[2]; ++idx[2]) {
					if (space.at<float>(idx) > max_count) {
						max_count = space.at<float>(idx);
						max_x = idx[0];
						max_y = idx[1];
						max_r = idx[2];
					}
				}
			}
		}

		DetectedCircle circle;
		circle.scaledCenter = QVector2D(max_x, max_y);
		circle.center = QVector2D((float)max_x / scale + bbox.minPt.x(), (float)max_y / scale + bbox.minPt.y());
		circle.scaledRadius = max_r;
		circle.radius = max_r / scale;

		// if the vote is too few, then the detection will stop.
		if (max_count < 1.0f) break;

		// add the corresponding edges to this circle
		{
			RoadEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;
				if (usedEdges.contains(*ei)) continue;

				Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 10.0f);

				int count2 = 0;

				for (int k = 0; k < polyline.size(); ++k) {
					int x = (polyline[k].x() - bbox.minPt.x()) * scale;
					int y = (polyline[k].y() - bbox.minPt.y()) * scale;

					// if the line segment is part of the circle, then count up.
					if (abs(max_r - radiusTable.at<float>((int)abs(x - max_x), (int)abs(y - max_y))) <= 3) {
						count2++;
					}
				}

				if (count2 > (int)polyline.size() * 0.8f) {
					circle.edges.push_back(*ei);
				}
			}
		}

		// compute the total length of this circle
		float length = 0.0;
		for (int j = 0; j < circle.edges.size(); ++j) {
			length += roads.graph[circle.edges[j]]->polyline.length();
		}

		std::cout << "Circle: (" << circle.center.x() << "," << circle.center.y() << ") radius: " << circle.radius << " len: " << length << " cnt: " << max_count << std::endl;

		// if the total length is large enough, add this circle as detected.
		if (length >= circle.radius * 2 * M_PI * 0.9f) {
			circles.push_back(circle);

			for (int j = 0; j < circle.edges.size(); ++j) {
				usedEdges[circle.edges[j]] = true;
			}
		}

		// clear the votes around the detected circle center
		for (idx[0] = max_x - 5; idx[0] <= max_x + 5; ++idx[0]) {
			if (idx[0] < 0 || idx[0] >= size[0]) continue;

			for (idx[1] = max_y - 5; idx[1] <= max_y + 5; ++idx[1]) {
				if (idx[1] < 0 || idx[1] >= size[1]) continue;

				for (idx[2] = max_r - 5; idx[2] <= max_r + 5; ++idx[2]) {
					if (idx[2] < 0 || idx[2] >= size[2]) continue;

					space.at<float>(idx) = 0;
				}
			}
		}
	}

	// remove the circle that does not have any edges
	for (int i = 0; i < circles.size(); ) {
		if (circles[i].edges.size() == 0) {
			circles.erase(circles.begin() + i);
		} else {
			++i;
		}
	}

	std::vector<RoadEdgeDescs> edges;
	for (int i = 0; i < circles.size(); ++i) {
		//std::cout << "Circle: (" << circles[i].center.x() << "," << circles[i].center.y() << ") radius: " << circles[i].radius << std::endl;

		edges.push_back(circles[i].edges);
	}
	
	return edges;
}
