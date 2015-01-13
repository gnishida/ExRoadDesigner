#include "CircleHoughTransform.h"
#include "GraphUtil.h"

/**
 * Hough Transformにより円を検出する。
 * houghThreshold は、デフォルトは100だが、50ぐらいにしないと、小さい円を検知しない。
 */
std::vector<RoadEdgeDescs> CircleHoughTransform::detect(RoadGraph &roads, float maxRadius, float houghThreshold) {
	BBox bbox = GraphUtil::bbox(roads);

	cv::Mat img(bbox.dy(), bbox.dx(), CV_8U, cv::Scalar(0));
	RoadEdgeIter ei, eend;
	for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
		if (!roads.graph[*ei]->valid) continue;

		for (int pl = 0; pl < roads.graph[*ei]->polyline.size() - 1; ++pl) {
			int x1 = roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x();
			int y1 = roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y();
			int x2 = roads.graph[*ei]->polyline[pl + 1].x() - bbox.minPt.x();
			int y2 = roads.graph[*ei]->polyline[pl + 1].y() - bbox.minPt.y();
			cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255), 3);
		}
	}

	cv::GaussianBlur(img, img, cv::Size(9, 9), 2, 2);

	std::vector<cv::Vec3f> circles;

	// Apply the Hough Transform to find the circles
	cv::HoughCircles(img, circles, CV_HOUGH_GRADIENT, 1, img.rows/8, 200, houghThreshold, 0, maxRadius);

	QMap<RoadEdgeDesc, bool> usedEdges;
	std::vector<RoadEdgeDescs> edges;

	// list up all the centers that have votes more than 100
	for (int i = 0; i < circles.size(); ++i) {
		float cx = circles[i][0] + bbox.minPt.x();
		float cy = circles[i][1] + bbox.minPt.y();
		float cr = circles[i][2];

		RoadEdgeDescs circle_edges;

		// add the corresponding edges to this circle
		{
			RoadEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
				if (!roads.graph[*ei]->valid) continue;
				if (usedEdges.contains(*ei)) continue;

				//Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 1.0f / scale);
				Polyline2D polyline = GraphUtil::finerEdge(roads.graph[*ei]->polyline, 10.0f);

				int count = 0;

				for (int k = 0; k < polyline.size(); ++k) {
					// if the line segment is part of the circle, then count up.
					float error = abs(sqrt((polyline[k].x() - cx) * (polyline[k].x() - cx) + (polyline[k].y() - cy) * (polyline[k].y() - cy)) - cr);
					if (error < 20.0f) {
						count++;
					}
				}

				if (count > (int)polyline.size() * 0.8f) {
					circle_edges.push_back(*ei);
				}
			}
		}

		// compute the total length
		float length = 0.0f;
		for (int i = 0; i < circle_edges.size(); ++i) {
			length += roads.graph[circle_edges[i]]->polyline.length();
		}

		if (length >= cr * 2.0f * 3.14159265f * 0.7f) {
			edges.push_back(circle_edges);
			for (int i = 0; i < circle_edges.size(); ++i) {
				usedEdges[circle_edges[i]] = true;
			}
		}
	}
	
	return edges;
}
