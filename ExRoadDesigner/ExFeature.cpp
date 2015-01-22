#include "ExFeature.h"
#include <QTextStream>
#include "Util.h"
#include "GraphUtil.h"
#include "RoadEdge.h"
#include <fstream>
#include "ShapeDetector.h"
#include "RoadGeneratorHelper.h"

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
	//if (reduce) {
		GraphUtil::reduce(reducedAvenues);
	//}
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
 * パッチを画像として保存する。
 */
void ExFeature::savePatchImages(int roadType, int ex_id, RoadGraph& roads, std::vector<Patch> patches, float scale, bool label) {
	// 画像の大きさを決定
	BBox bbox = GraphUtil::getAABoundingBox(roads, true);
	bbox.minPt -= QVector2D(10.0f, 10.0f);
	bbox.maxPt += QVector2D(10.0f, 10.0f);

	int width = 3 * scale;

	for (int i = 0; i < patches.size(); ++i) {
		cv::Mat img((int)(bbox.dy() * scale), (int)(bbox.dx() * scale), CV_8UC3, cv::Scalar(255, 255, 255));

		// 画像に、全パッチを描画する
		for (int j2 = 0; j2 < patches.size(); ++j2) {
			RoadEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::edges(patches[j2].roads.graph); ei != eend; ++ei) {
				for (int pl = 0; pl < patches[j2].roads.graph[*ei]->polyline.size() - 1; ++pl) {
					int x1 = (patches[j2].roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x()) * scale;
					int y1 = img.rows - (patches[j2].roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y()) * scale;
					int x2 = (patches[j2].roads.graph[*ei]->polyline[pl+1].x() - bbox.minPt.x()) * scale;
					int y2 = img.rows - (patches[j2].roads.graph[*ei]->polyline[pl+1].y() - bbox.minPt.y()) * scale;

					cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(248, 248, 248), width);
				}
			}
		}

		// 画像に、当該パッチを描画する
		{
			RoadEdgeIter ei, eend;
			for (boost::tie(ei, eend) = boost::edges(patches[i].roads.graph); ei != eend; ++ei) {
				for (int pl = 0; pl < patches[i].roads.graph[*ei]->polyline.size() - 1; ++pl) {
					int x1 = (patches[i].roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x()) * scale;
					int y1 = img.rows - (patches[i].roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y()) * scale;
					int x2 = (patches[i].roads.graph[*ei]->polyline[pl+1].x() - bbox.minPt.x()) * scale;
					int y2 = img.rows - (patches[i].roads.graph[*ei]->polyline[pl+1].y() - bbox.minPt.y()) * scale;

					if (patches[i].roads.graph[*ei]->connector) {
						cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 128, 128), width);
					} else {
						cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(255, 0, 0), width);
					}
				}
			}

			// 頂点の描画
			{
				RoadVertexIter vi, vend;
				for (boost::tie(vi, vend) = boost::vertices(patches[i].roads.graph); vi != vend; ++vi) {
					int x = (patches[i].roads.graph[*vi]->pt.x() - bbox.minPt.x()) * scale;
					int y = img.rows - (patches[i].roads.graph[*vi]->pt.y() - bbox.minPt.y()) * scale;

					// 頂点の描画 (青色の円)
					cv::circle(img, cv::Point(x, y), width * 1.5, cv::Scalar(255, 0, 0), -1);
				}
			}

			if (label) {
				// コネクタの描画 (灰色の×)
				for (int ci = 0; ci < patches[i].connectors.size(); ++ci) {
					int x = (patches[i].roads.graph[patches[i].connectors[ci]]->pt.x() - bbox.minPt.x()) * scale;
					int y = img.rows - (patches[i].roads.graph[patches[i].connectors[ci]]->pt.y() - bbox.minPt.y()) *scale;
					cv::line(img, cv::Point(x - 10, y - 10), cv::Point(x + 10, y + 10), cv::Scalar(128, 128, 128), 5);
					cv::line(img, cv::Point(x + 10, y - 10), cv::Point(x - 10, y + 10), cv::Scalar(128, 128, 128), 5);
				}

				RoadVertexIter vi, vend;
				for (boost::tie(vi, vend) = boost::vertices(patches[i].roads.graph); vi != vend; ++vi) {
					int x = (patches[i].roads.graph[*vi]->pt.x() - bbox.minPt.x()) * scale;
					int y = img.rows - (patches[i].roads.graph[*vi]->pt.y() - bbox.minPt.y()) * scale;

					// onBoundaryの描画 (黄色の円)
					if (patches[i].roads.graph[*vi]->onBoundary) {
						cv::circle(img, cv::Point(x, y), 10, cv::Scalar(0, 255, 255), 5);
					}

					// deadendの描画 (赤色の円)
					if (patches[i].roads.graph[*vi]->deadend) {
						cv::circle(img, cv::Point(x, y), 10, cv::Scalar(0, 0, 255), 5);
					}

					// 頂点IDと、元のexampleの頂点IDを描画
					QString str = QString::number(*vi) + "/" + QString::number(patches[i].roads.graph[*vi]->properties["example_desc"].toUInt());
					cv::putText(img, str.toUtf8().data(), cv::Point(x, y), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 0.5, cv::Scalar(0, 128, 0), 1);
				}
			}
		}

		char filename[255];
		if (roadType == RoadEdge::TYPE_AVENUE) {
			sprintf(filename, "patches/avenue%d_patch_%d.jpg", ex_id, i);
		} else {
			sprintf(filename, "patches/street%d_patch_%d.jpg", ex_id, i);
		}
		//cv::flip(img, img, 0);
		cv::imwrite(filename, img);
	}

	// 元のExample道路も描画（各頂点について、属するパッチIDを表示）
	{
		cv::Mat img((int)(bbox.dy() * scale), (int)(bbox.dx() * scale), CV_8UC3, cv::Scalar(255, 255, 255));

		RoadEdgeIter ei, eend;
		for (boost::tie(ei, eend) = boost::edges(roads.graph); ei != eend; ++ei) {
			if (!roads.graph[*ei]->valid) continue;

			for (int pl = 0; pl < roads.graph[*ei]->polyline.size() - 1; ++pl) {
				int x1 = (roads.graph[*ei]->polyline[pl].x() - bbox.minPt.x()) * scale;
				int y1 = img.rows - (roads.graph[*ei]->polyline[pl].y() - bbox.minPt.y()) * scale;
				int x2 = (roads.graph[*ei]->polyline[pl+1].x() - bbox.minPt.x()) * scale;
				int y2 = img.rows - (roads.graph[*ei]->polyline[pl+1].y() - bbox.minPt.y()) * scale;
				cv::line(img, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 0), width);
			}
		}

		// 頂点を描画
		{
			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
				if (!roads.graph[*vi]->valid) continue;

				int x = (roads.graph[*vi]->pt.x() - bbox.minPt.x()) * scale;
				int y = img.rows - (roads.graph[*vi]->pt.y() - bbox.minPt.y()) * scale;

				// 頂点の描画 (黒色の円)
				cv::circle(img, cv::Point(x, y), width * 1.5, cv::Scalar(0, 0, 0), -1);
			}
		}

		if (label) {
			RoadVertexIter vi, vend;
			for (boost::tie(vi, vend) = boost::vertices(roads.graph); vi != vend; ++vi) {
				if (!roads.graph[*vi]->valid) continue;

				int x = (roads.graph[*vi]->pt.x() - bbox.minPt.x()) * scale;
				int y = img.rows - (roads.graph[*vi]->pt.y() - bbox.minPt.y()) * scale;

				// 属するパッチIDを描画
				if (roads.graph[*vi]->patchId < 0 && !roads.graph[*vi]->onBoundary) {
					printf("ERROR!!!!!!!!!!!!!!  patchID should be >= 0.\n");
				}
				QString str = QString::number(roads.graph[*vi]->patchId);
				cv::putText(img, str.toUtf8().data(), cv::Point(x, y), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 1, cv::Scalar(0, 128, 0), 2);
			}
		}

		char filename[255];
		if (roadType == RoadEdge::TYPE_AVENUE) {
			sprintf(filename, "patches/avenue%d_patch_ids.jpg", ex_id);
		} else {
			sprintf(filename, "patches/street%d_patch_ids.jpg", ex_id);
		}
		cv::imwrite(filename, img);
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

std::vector<Patch> ExFeature::patches(int roadType, float houghScale, float patchDistance) {
	if (roadType == RoadEdge::TYPE_AVENUE) {
		if (!avenueShapesDetected) detectAvenueShapes(houghScale, patchDistance);
		return avenuePatches;
	} else {
		if (!streetShapesDetected) detectStreetShapes(houghScale, patchDistance);
		return streetPatches;
	}
}

void ExFeature::detectAvenueShapes(float houghScale, float patchDistance) {
	if (avenueShapesDetected) return;
	avenueShapesDetected = true;

	avenueShapes = ShapeDetector::detect(avenues, houghScale, patchDistance);
	avenuePatches = RoadGeneratorHelper::convertToPatch(RoadEdge::TYPE_AVENUE, avenues, avenues, avenueShapes);

	// save patch images
	//savePatchImages(RoadEdge::TYPE_AVENUE, 1, avenues, avenuePatches, 1.0f, true);
}

void ExFeature::detectStreetShapes(float houghScale, float patchDistance) {
	if (streetShapesDetected) return;
	streetShapesDetected = true;

	streetShapes = ShapeDetector::detect(streets, houghScale, patchDistance);
	streetPatches = RoadGeneratorHelper::convertToPatch(RoadEdge::TYPE_STREET, streets, avenues, streetShapes);

	// save patch images
	//savePatchImages(RoadEdge::TYPE_STREET, 1, streets, streetPatches, 1.0f, true);
}
