#include <QtGui/QApplication>
#include <common/Util.h>
#include <common/ConvexHull.h>
#include "MainWindow.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();
}
