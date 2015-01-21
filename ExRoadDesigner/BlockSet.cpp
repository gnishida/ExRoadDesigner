#include "BlockSet.h"
#include <QFile>
#include <QTextStream>
#include "Polygon2D.h"

int BlockSet::selectBlock(const QVector2D& pos) {
	for (int i = 0; i < blocks.size(); ++i) {
		Polygon2D polygon;
		for (int j = 0; j < blocks[i].blockContour.contour.size(); ++j) {
			polygon.push_back(QVector2D(blocks[i].blockContour[j]));
		}
		polygon.correct();

		if (polygon.contains(pos)) {
			selectedBlockIndex = i;
			return i;
		}
	}

	selectedBlockIndex = -1;

	return -1;
}

std::pair<int, int> BlockSet::selectParcel(const QVector2D& pos) {
	for (int i = 0; i < blocks.size(); ++i) {
		Block::parcelGraphVertexIter vi, viEnd;
		for (boost::tie(vi, viEnd) = boost::vertices(blocks[i].myParcels); vi != viEnd; ++vi) {
			Polygon2D polygon;
			for (int j = 0; j < blocks[i].myParcels[*vi].parcelContour.contour.size(); ++j) {
				polygon.push_back(QVector2D(blocks[i].myParcels[*vi].parcelContour[j]));
			}
			polygon.correct();

			if (polygon.contains(pos)) {
				selectedBlockIndex = i;
				selectedParcelIndex = *vi;

				return std::make_pair(selectedBlockIndex, selectedParcelIndex);
			}
		}
	}

	selectedBlockIndex = -1;
	selectedParcelIndex = -1;

	return std::make_pair(-1, -1);
}

void BlockSet::removeSelectedBlock() {
	if (selectedBlockIndex < 0 || selectedBlockIndex >= blocks.size()) return;

	blocks.erase(blocks.begin() + selectedBlockIndex);

	selectedBlockIndex = -1;
	selectedParcelIndex = -1;
	//modified = true;
}

void BlockSet::clear() {
	blocks.clear();

	selectedBlockIndex = -1;
	selectedParcelIndex = -1;
	//modified = true;
}