#include "ExFeatureItemEdge.h"

void ExFeatureItemEdge::scale(float scaleX, float scaleY, const QVector2D &orig) {
	for (int i = 0; i < edge.size(); ++i) {
		QVector2D vec = edge[i] - orig;
		edge[i].setX(orig.x() + vec.x() * scaleX);
		edge[i].setY(orig.y() + vec.y() * scaleY);
	}
}