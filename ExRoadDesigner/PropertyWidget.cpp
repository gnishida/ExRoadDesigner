#include "PropertyWidget.h"
#include "MainWindow.h"
#include "GraphUtil.h"

PropertyWidget::PropertyWidget(MainWindow* mainWin) : QDockWidget("Property Widget", (QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);

	// register the event handlers
	connect(ui.pushButtonVertexSearch, SIGNAL(clicked()), this, SLOT(searchVertex()));

	// register the event handlers
	hide();
}

void PropertyWidget::setRoadVertex(RoadGraph &roads, RoadVertexDesc vertexDesc, RoadVertexPtr selectedVertex) {
	QString desc("");
	QString location("");
	QString parent("");
	QString uncles("");

	desc.setNum(vertexDesc);

	location = QString("(%1, %2)").arg(roads.graph[vertexDesc]->pt.x(), 0, 'f', 0).arg(roads.graph[vertexDesc]->pt.y(), 0, 'f', 0);

	if (roads.graph[vertexDesc]->properties.contains("parent")) {
		parent.setNum(roads.graph[vertexDesc]->properties["parent"].toUInt());
	}

	QString onBoundary = roads.graph[vertexDesc]->onBoundary ? "Yes" : "No";

	if (roads.graph[vertexDesc]->properties.contains("uncles")) {
		uncles = roads.graph[vertexDesc]->properties["uncles"].toString();
	}

	QString neighbors;
	std::vector<RoadVertexDesc> n = GraphUtil::getNeighbors(roads, vertexDesc);
	for (int i = 0; i < n.size(); i++) {
		QString str;
		str.setNum(n[i]);

		neighbors += str;
		if (i < n.size() - 1) neighbors += ",";
	}

	QString groupId = selectedVertex->properties["group_id"].toString();
	QString generationType = selectedVertex->properties["generation_type"].toString();
	QString exampleDesc;
	QString exampleStreetDesc;
	if (generationType == "example") {
		exampleDesc = selectedVertex->properties["example_desc"].toString();
		if (selectedVertex->properties.contains("example_street_desc")) {
			exampleStreetDesc = selectedVertex->properties["example_street_desc"].toString();
		}
	}

	QString deadend = selectedVertex->properties["deadend"].toBool() ? "Yes" : "No";

	ui.lineEditVertexDesc->setText(desc);
	ui.lineEditVertexPos->setText(location);
	ui.lineEditVertexParent->setText(parent);
	ui.lineEditVertexOnBoundary->setText(onBoundary);
	ui.textEditVertexUncles->setText(uncles);
	ui.textEditVertexNeighbors->setText(neighbors);
	ui.lineEditVertexGroupId->setText(groupId);
	ui.lineEditVertexGenerationType->setText(generationType);
	ui.lineEditVertexExampleDesc->setText(exampleDesc);
	ui.lineEditVertexExampleStreetDesc->setText(exampleStreetDesc);
	ui.lineEditVertexDeadend->setText(deadend);
}

/**
 * Display the selected edge information.
 */
void PropertyWidget::setRoadEdge(RoadEdgePtr selectedEdge) {
	QString type;
	QString numLanes;
	QString oneWay;
	QString link;
	QString roundabout;
	QString groupId;
	QString generationType;

	if (selectedEdge != NULL) {
		switch (selectedEdge->type) {
		case RoadEdge::TYPE_HIGHWAY:
			type = "Highway";
			break;
		case RoadEdge::TYPE_BOULEVARD:
			type = "Boulevard";
			break;
		case RoadEdge::TYPE_AVENUE:
			type = "Avenue";
			break;
		case RoadEdge::TYPE_STREET:
			type = "Street";
			break;
		default:
			type = "";
			break;
		}

		numLanes.setNum(selectedEdge->lanes);

		oneWay = selectedEdge->oneWay ? "Yes" : "No";
		link = selectedEdge->link ? "Yes" : "No";
		roundabout = selectedEdge->roundabout ? "Yes" : "No";

		groupId = selectedEdge->properties["group_id"].toString();
		generationType = selectedEdge->properties["generation_type"].toString();
	}

	ui.lineEditEdgeType->setText(type);
	ui.lineEditEdgeLanes->setText(numLanes);
	ui.lineEditEdgeOneWay->setText(oneWay);
	ui.lineEditEdgeLink->setText(link);
	ui.lineEditEdgeRoundabout->setText(roundabout);
	ui.lineEditEdgeGroupId->setText(groupId);
	ui.lineEditEdgeGenerationType->setText(generationType);
}

void PropertyWidget::resetRoadVertex() {
	ui.lineEditVertexDesc->setText("");
	ui.lineEditVertexPos->setText("");
	ui.lineEditVertexParent->setText("");
	ui.lineEditVertexOnBoundary->setText("");
	ui.textEditVertexUncles->setText("");
	ui.textEditVertexNeighbors->setText("");
	ui.lineEditVertexGroupId->setText("");
	ui.lineEditVertexGenerationType->setText("");
	ui.lineEditVertexExampleDesc->setText("");
	ui.lineEditVertexExampleStreetDesc->setText("");
	ui.lineEditVertexDeadend->setText("");
}

void PropertyWidget::resetRoadEdge() {
	ui.lineEditEdgeType->setText("");
	ui.lineEditEdgeLanes->setText("");
	ui.lineEditEdgeOneWay->setText("");
	ui.lineEditEdgeLink->setText("");
	ui.lineEditEdgeRoundabout->setText("");
	ui.lineEditEdgeGroupId->setText("");
	ui.lineEditEdgeGenerationType->setText("");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void PropertyWidget::searchVertex() {
	/*
	RoadVertexDesc v_desc = ui.lineEditVertexSearch->text().toUInt();

	if (mainWin->urbanGeometry->areas.selectedIndex >= 0) {
		if (v_desc < boost::num_vertices(mainWin->urbanGeometry->areas.selectedArea()->roads.graph)) {
			mainWin->glWidget->selectVertex(mainWin->urbanGeometry->areas.selectedArea()->roads, v_desc);

			mainWin->glWidget->updateGL();
		}
	}
	*/
}
