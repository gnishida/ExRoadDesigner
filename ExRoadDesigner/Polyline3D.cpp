#include "Polyline3D.h"
#include "Util.h"

const QVector3D & Polyline3D::last() const {
	return at(size() - 1);
}

QVector3D & Polyline3D::last() {
	return at(size() - 1);
}

/**
 * 当該ポリラインを移動する。
 */
void Polyline3D::translate(const QVector2D &offset) {
	for (int i = 0; i < size(); ++i) {
		(*this)[i] += offset;
	}
}

/**
 * 指定された点を中心に、指定された角度だけ時計回りに回転したポリゴンを返却する。
 *
 * @param angle		時計回りの回転角度[degree]
 * @param orig		回転中心
 */
void Polyline3D::rotate(float angle, const QVector2D &orig) {
	for (int i = 0; i < size(); ++i) {
		float z = (*this)[i].z();
		(*this)[i] = Util::rotate(QVector2D(at(i).x(), at(i).y()), -Util::deg2rad(angle), orig);
		(*this)[i].setZ(z);
	}
}

/**
 * スケールする
 */
void Polyline3D::scale(float factor) {
	for (int i = 0; i < size(); ++i) {
		(*this)[i] = QVector3D(at(i).x() * factor, at(i).y() * factor, at(i).z());
	}
}

