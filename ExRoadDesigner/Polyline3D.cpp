#include "Polyline3D.h"
#include "Util.h"

const QVector3D & Polyline3D::last() const {
	return at(size() - 1);
}

QVector3D & Polyline3D::last() {
	return at(size() - 1);
}

/**
 * translate this polyline
 */
void Polyline3D::translate(const QVector2D &offset) {
	for (int i = 0; i < size(); ++i) {
		(*this)[i] += offset;
	}
}

/**
 * rotate this polygon clockwise around the given point.
 *
 * @param angle		angle [degree]
 * @param orig		rotation center
 */
void Polyline3D::rotate(float angle, const QVector2D &orig) {
	for (int i = 0; i < size(); ++i) {
		float z = (*this)[i].z();
		(*this)[i] = Util::rotate(QVector2D(at(i).x(), at(i).y()), -Util::deg2rad(angle), orig);
		(*this)[i].setZ(z);
	}
}

/**
 * scale
 */
void Polyline3D::scale(float factor) {
	for (int i = 0; i < size(); ++i) {
		(*this)[i] = QVector3D(at(i).x() * factor, at(i).y() * factor, at(i).z());
	}
}

