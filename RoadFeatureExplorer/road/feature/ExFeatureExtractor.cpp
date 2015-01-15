#include <time.h>
#include "../../common/global.h"
#include "../../common/Util.h"
#include "../GraphUtil.h"
#include "ExFeatureExtractor.h"


/**
 * Example-basedの特徴量を抽出する。
 */
void ExFeatureExtractor::extractFeature(RoadGraph& roads, Polygon2D& area, Polyline2D& hintLine, ExFeature& feature) {
	QVector2D center = area.envelope().midPt();

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// エリア内の道路網を保存する
	GraphUtil::copyRoads(roads, feature.original);
	GraphUtil::extractRoads2(feature.original, area);
	GraphUtil::clean(feature.original);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Avenueのみを抽出する
	RoadGraph temp_roads;
	GraphUtil::copyRoads(roads, feature.avenues);
	GraphUtil::extractRoads2(feature.avenues, area, RoadEdge::TYPE_AVENUE | RoadEdge::TYPE_BOULEVARD);
	GraphUtil::clean(feature.avenues);
	//GraphUtil::reduce(feature.avenues);  <- reduceは、特徴量を利用する側が、適宜実施すること！

	// roundaboutを削除する
	//GraphUtil::removeRoundabout(temp_roads);

	// linkを削除する
	//GraphUtil::removeLinkEdges(feature.avenues);
	//GraphUtil::reduce(feature.avenues);
	//GraphUtil::removeIsolatedVertices(feature.avenues);
	//GraphUtil::clean(feature.avenues);

	// 中心が(0,0)となるよう座標変換する
	//GraphUtil::translate(feature.avenues, center * -1.0f);

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// streetのみを抽出する
	GraphUtil::copyRoads(roads, feature.streets);
	GraphUtil::extractRoads2(feature.streets, area, RoadEdge::TYPE_STREET);
	//GraphUtil::reduce(temp_roads);  <- わざとreduceしない

	// linkを削除する
	//GraphUtil::removeLinkEdges(feature.streets);
	//GraphUtil::removeIsolatedVertices(feature.streets);
	GraphUtil::clean(feature.streets);

	// 中心が(0,0)となるよう座標変換する
	//GraphUtil::translate(feature.streets, center * -1.0f);

	// エリアを設定
	Polygon2D translatedArea;
	//area.translate(-center.x(), -center.y(), translatedArea);
	//feature.setArea(translatedArea);
	feature.setArea(area);

	// HintLineを設定
	Polyline2D translatedHintLine;
	//hintLine.translate(-center.x(), -center.y(), translatedHintLine);
	//feature.setHintLine(translatedHintLine);
	feature.setHintLine(hintLine);
}

