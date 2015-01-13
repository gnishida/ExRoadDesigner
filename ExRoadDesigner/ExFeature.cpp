#include "ExFeature.h"
#include <QTextStream>
#include "Util.h"
#include "GraphUtil.h"
#include "RoadEdge.h"
#include <fstream>
#include "ShapeDetector.h"

void ExFeature::setArea(const Polygon2D &area) {
	this->area = area;
}

void ExFeature::setHintLine(const Polyline2D &polyline) {
	hintLine = polyline;
}

RoadGraph& ExFeature::roads(int roadType) {
	if (roadType == RoadEdge::TYPE_AVENUE) {
		return avenues;
	} else {
		return streets;
	}
}

RoadGraph& ExFeature::reducedRoads(int roadType) {
	if (roadType == RoadEdge::TYPE_AVENUE) {
		return reducedAvenues;
	} else {
		return streets;
	}
}

/**
 * 指定された角度[degree]だけ、交差点カーネルを時計回りに回転する。
 */
/*
void ExFeature::rotate(float deg) {
	QVector2D centroid = _area.centroid();
	_area.rotate(deg, centroid);

	for (int i = 0; i < _avenueItems.size(); ++i) {
		_avenueItems[i].rotate(deg, centroid);
	}

	for (int i = 0; i < _streetItems.size(); ++i) {
		_streetItems[i].rotate(deg, centroid);
	}
}

void ExFeature::scale(const Polygon2D &area) {
	QVector2D centroid = _area.centroid();

	BBox bboxTarget = area.envelope();
	BBox bboxSource = _area.envelope();

	float scaleX = bboxTarget.dx() / bboxSource.dx() + 0.1f;
	float scaleY = bboxTarget.dy() / bboxSource.dy() + 0.1f;

	for (int i = 0; i < _avenueItems.size(); ++i) {
		_avenueItems[i].scale(scaleX, scaleY, centroid);

		QString str = QString("avenue_kernel_%1.png").arg(i);
		_avenueItems[i].imwrite(str);
	}

	for (int i = 0; i < _streetItems.size(); ++i) {
		_streetItems[i].scale(scaleX, scaleY, centroid);
	}

	for (int i = 0; i < _area.size(); ++i) {
		QVector2D vec = _area[i] - centroid;
		_area[i].setX(centroid.x() + vec.x() * scaleX);
		_area[i].setY(centroid.y() + vec.y() * scaleY);
	}
}
*/

/**
 * PM用にエッジ長を生成する
 */
float ExFeature::length(int roadType) const {
	if (roadType == RoadEdge::TYPE_AVENUE) {
		float length = Util::genRandNormal(avgAvenueLength, varAvenueLength);
		if (length < avgAvenueLength * 0.5f) length = avgAvenueLength * 0.5f;
		return length;
	} else {
		float length = Util::genRandNormal(avgStreetLength, varStreetLength);
		if (length < avgStreetLength * 0.5f) length = avgStreetLength * 0.5f;
		return length;
	}
}

/**
 * PM用にエッジの曲率を生成する
 */
float ExFeature::curvature(int roadType) const {
	float curvature;
	if (roadType == RoadEdge::TYPE_AVENUE) {
		curvature = Util::genRandNormal(avgAvenueCurvature, varAvenueCurvature);
	} else {
		curvature = Util::genRandNormal(avgStreetCurvature, varStreetCurvature);
	}

	if (curvature < 0.0f) curvature = 0.0f;
	if (curvature > M_PI * 0.5f) curvature = M_PI * 0.5f;
	return curvature;
}

/**
 * PM用パラメータを計算する
 */
void ExFeature::computePMParameters() {
	// この結果、各エッジのreduced_numパラメータに、street交差点による分割数が入ったので、ヒストグラムを生成
	/*
	{
		streetSeedNum.clear();

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(avenues.graph); ei != eend; ++ei) {
			if (!avenues.graph[*ei]->valid) continue;
		
			streetSeedNum.getBin(avenues.graph[*ei]->getLength()).vote(avenues.graph[*ei]->properties["reduced_num"].toInt());
		}
	}
	*/

	// extract avenues feature
	GraphUtil::computeStatistics(reducedAvenues, avgAvenueLength, varAvenueLength, avgAvenueCurvature, varAvenueCurvature);

	// extract local streets feature
	GraphUtil::computeStatistics(streets, avgStreetLength, varStreetLength, avgStreetCurvature, varStreetCurvature);
}

void ExFeature::load(QString filepath, bool reduce) {
	// ファイル名からディレクトリ部を取得
	QString dirname;
	int index = filepath.lastIndexOf("/");
	if (index > 0) {
		dirname = filepath.mid(0, index);
	}

	QFile file(filepath);

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	QDomNode node = root.firstChild();
	while (!node.isNull()) {
		if (node.toElement().tagName() == "area") {
			loadArea(node);
		} else if (node.toElement().tagName() == "hintLine") {
			loadHintLine(node);
		} else if (node.toElement().tagName() == "avenues") {
			GraphUtil::loadRoads(avenues, dirname + "/" + node.toElement().attribute("filename"));
		} else if (node.toElement().tagName() == "streets") {
			GraphUtil::loadRoads(streets, dirname + "/" + node.toElement().attribute("filename"));
		}

		node = node.nextSibling();
	}

	GraphUtil::copyRoads(avenues, reducedAvenues);
	if (reduce) {
		GraphUtil::reduce(reducedAvenues);
	}
	GraphUtil::clean(reducedAvenues);

	init();

	computePMParameters();

	avenueShapesDetected = false;
	streetShapesDetected = false;
}

/**
 * Read an area node to the specified Dom document under the parent node.
 */
void ExFeature::loadArea(QDomNode &node) {
	area.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			area.push_back(QVector2D(child.toElement().attribute("x").toFloat(), child.toElement().attribute("y").toFloat()));
		}

		child = child.nextSibling();
	}
}

/**
 * Read an area node to the specified Dom document under the parent node.
 */
void ExFeature::loadHintLine(QDomNode &node) {
	hintLine.clear();

	QDomNode child = node.firstChild();
	while (!child.isNull()) {
		if (child.toElement().tagName() == "point") {
			hintLine.push_back(QVector2D(child.toElement().attribute("x").toFloat(), child.toElement().attribute("y").toFloat()));
		}

		child = child.nextSibling();
	}
}

void ExFeature::save(QString filepath) {
	// ファイル名からディレクトリ部を取得
	QString dirname;
	int index = filepath.lastIndexOf("/");
	if (index > 0) {
		dirname = filepath.mid(0, index);
	}

	// ファイルパスからファイル名を取得
	QString filename;
	index = filepath.lastIndexOf("/");
	if (index >= 0) {
		filename = filepath.mid(index + 1);
	} else {
		filename = filepath;
	}


	QDomDocument doc;

	QDomElement root = doc.createElement("feature");
	doc.appendChild(root);


	// write area node
	saveArea(doc, root);

	saveHintLine(doc, root);

	// write avenues
	QDomElement node_avenues = doc.createElement("avenues");
	node_avenues.setAttribute("filename", filename + ".avenues.gsm");
	GraphUtil::saveRoads(avenues, dirname + "/" + filename + ".avenues.gsm");
	root.appendChild(node_avenues);

	// write streets
	QDomElement node_streets = doc.createElement("streets");
	node_streets.setAttribute("filename", filename + ".streets.gsm");
	GraphUtil::saveRoads(streets, dirname + "/" + filename + ".streets.gsm");
	root.appendChild(node_streets);




	// write the dom to the file
	QFile file(filepath);
	file.open(QIODevice::WriteOnly);

	QTextStream out(&file);
	doc.save(out, 4);
}

/**
 * Write an area node to the specified Dom document under the parent node.
 */
void ExFeature::saveArea(QDomDocument &doc, QDomNode &parent) {
	QDomElement node_area = doc.createElement("area");
	parent.appendChild(node_area);

	for (int i = 0; i < area.size(); ++i) {
		QDomElement node_point = doc.createElement("point");
		node_point.setAttribute("x", area[i].x());
		node_point.setAttribute("y", area[i].y());

		node_area.appendChild(node_point);
	}
}

/**
 * Write an hint line to the specified Dom document under the parent node.
 */
void ExFeature::saveHintLine(QDomDocument &doc, QDomNode &parent) {
	QDomElement node_area = doc.createElement("hintLine");
	parent.appendChild(node_area);

	for (int i = 0; i < hintLine.size(); ++i) {
		QDomElement node_point = doc.createElement("point");
		node_point.setAttribute("x", hintLine[i].x());
		node_point.setAttribute("y", hintLine[i].y());

		node_area.appendChild(node_point);
	}
}

/**
 * 道路avenuesの各Vertexについて、ほぼ同じ座標のlocal streetのVertexがあれば、local streetのVertexの
 * propertyに"avenue_intersected" = trueを設定する。
 */
void ExFeature::init() {
	// エリア境界上の頂点にたいして、onBoundaryフラグをセットする。


	// local streetについて、avenueと交差しているかどうかのフラグをセットする
	RoadVertexIter vi, vend;
	for (boost::tie(vi, vend) = boost::vertices(avenues.graph); vi != vend; ++vi) {
		RoadVertexDesc desc;
		if (GraphUtil::getVertex(streets, avenues.graph[*vi]->pt, 1.0f, desc)) {
			streets.graph[desc]->properties["avenue_intersected"] = true;
		}
	}
}

std::vector<RoadEdgeDescs> ExFeature::shapes(int roadType, float houghScale, float patchDistance) {
	if (roadType == RoadEdge::TYPE_AVENUE) {
		if (!avenueShapesDetected) detectAvenueShapes(houghScale, patchDistance);
		return avenueShapes;
	} else {
		if (!streetShapesDetected) detectStreetShapes(houghScale, patchDistance);
		return streetShapes;
	}
}

void ExFeature::detectAvenueShapes(float houghScale, float patchDistance) {
	if (avenueShapesDetected) return;
	avenueShapesDetected = true;

	avenueShapes = ShapeDetector::detect(reducedAvenues, houghScale, patchDistance);

	for (int j = 0; j < avenueShapes.size(); ++j) {
		//cv::Mat img(15000, 15000, CV_8UC3);

		for (int k = 0 ; k < avenueShapes[j].size(); ++k) {
			reducedAvenues.graph[avenueShapes[j][k]]->properties["shape_id"] = j;
		}

		// 画像に、パッチを描画する
		/*
		for (int j2 = 0; j2 < avenueShapes.size(); ++j2) {
			for (int k = 0 ; k < avenueShapes[j2].size(); ++k) {
				for (int pl = 0; pl < reducedAvenues.graph[avenueShapes[j2][k]]->polyline.size() - 1; ++pl) {
					int x1 = (reducedAvenues.graph[avenueShapes[j2][k]]->polyline[pl].x() + 7500) * 1;
					int y1 = (reducedAvenues.graph[avenueShapes[j2][k]]->polyline[pl].y() + 7500) * 1;
					int x2 = (reducedAvenues.graph[avenueShapes[j2][k]]->polyline[pl+1].x() + 7500) * 1;
					int y2 = (reducedAvenues.graph[avenueShapes[j2][k]]->polyline[pl+1].y() + 7500) * 1;

					if (j2 == j) {
						cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), 3);
					} else {
						cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 255, 255), 3);
					}
				}
			}
		}

		char filename[255];
		sprintf(filename, "patches/avenue_patch_%d.jpg", j);
		cv::flip(img, img, 0);
		cv::imwrite(filename, img);
		*/
	}
}

void ExFeature::detectStreetShapes(float houghScale, float patchDistance) {
	if (streetShapesDetected) return;
	streetShapesDetected = true;

	streetShapes = ShapeDetector::detect(streets, houghScale, patchDistance);

	for (int j = 0; j < streetShapes.size(); ++j) {
		for (int k = 0 ; k < streetShapes[j].size(); ++k) {
			streets.graph[streetShapes[j][k]]->properties["shape_id"] = j;
		}
	}

}
