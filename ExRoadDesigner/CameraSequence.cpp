#include "CameraSequence.h"
#include <QFile>
#include <QDomNode>
#include <QTextStream>
#include "FlyThroughCamera.h"

void CameraSequence::load(const QString &filename, float step) {
	QFile file(filename);

	QDomDocument doc;
	doc.setContent(&file, true);
	QDomElement root = doc.documentElement();

	std::vector<FlyThroughCamera> controlCameras;

	QDomNode node = root.firstChild();
	while (!node.isNull()) {
		if (node.toElement().tagName() == "camera") {
			FlyThroughCamera camera;

			camera.xrot = node.toElement().attribute("xrot").toFloat();
			camera.yrot = node.toElement().attribute("yrot").toFloat();
			camera.zrot = node.toElement().attribute("zrot").toFloat();
			camera.dx = node.toElement().attribute("dx").toFloat();
			camera.dy = node.toElement().attribute("dy").toFloat();
			camera.dz = node.toElement().attribute("dz").toFloat();
			camera.lookAtX = node.toElement().attribute("lookAtX").toFloat();
			camera.lookAtY = node.toElement().attribute("lookAtY").toFloat();
			camera.lookAtZ = node.toElement().attribute("lookAtZ").toFloat();

			controlCameras.push_back(camera);
			cameras.push_back(camera);
		}

		node = node.nextSibling();
	}

	cameras.clear();

	for (int i = 0; i < controlCameras.size() - 1; ++i) {
		float dist = sqrtf(SQR(controlCameras[i].dx - controlCameras[i + 1].dx) + SQR(controlCameras[i].dy - controlCameras[i + 1].dy) + SQR(controlCameras[i].dz - controlCameras[i + 1].dz));

		int num = 0;
		if (dist > 0.0f) {
			num = dist / step;
		} else {
			num = 200;
		}
		num = 100;

		cameras.push_back(controlCameras[i]);
		
		for (int j = 1; j < num; ++j) {
			float dx = controlCameras[i].dx + (controlCameras[i + 1].dx - controlCameras[i].dx) / (float)num * (float)j;
			float dy = controlCameras[i].dy + (controlCameras[i + 1].dy - controlCameras[i].dy) / (float)num * (float)j;
			float dz = controlCameras[i].dz + (controlCameras[i + 1].dz - controlCameras[i].dz) / (float)num * (float)j;
			float xrot = controlCameras[i].xrot + (controlCameras[i + 1].xrot - controlCameras[i].xrot) / (float)num * (float)j;
			float yrot = controlCameras[i].yrot + (controlCameras[i + 1].yrot - controlCameras[i].yrot) / (float)num * (float)j;
			float zrot = controlCameras[i].zrot + (controlCameras[i + 1].zrot - controlCameras[i].zrot) / (float)num * (float)j;
			float lookAtX = controlCameras[i].lookAtX + (controlCameras[i + 1].lookAtX - controlCameras[i].lookAtX) / (float)num * (float)j;
			float lookAtY = controlCameras[i].lookAtY + (controlCameras[i + 1].lookAtY - controlCameras[i].lookAtY) / (float)num * (float)j;
			float lookAtZ = controlCameras[i].lookAtZ + (controlCameras[i + 1].lookAtZ - controlCameras[i].lookAtZ) / (float)num * (float)j;

			FlyThroughCamera camera;
			camera.dx = dx;
			camera.dy = dy;
			camera.dz = dz;
			camera.xrot = xrot;
			camera.yrot = yrot;
			camera.zrot = zrot;
			camera.lookAtX = lookAtX;
			camera.lookAtY = lookAtY;
			camera.lookAtZ = lookAtZ;
			cameras.push_back(camera);
		}
	}

	cameras.push_back(controlCameras.back());
}

