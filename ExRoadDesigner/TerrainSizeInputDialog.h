#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_TerrainSizeInputDialog.h"

class MainWindow;

class TerrainSizeInputDialog : public QDialog {
Q_OBJECT

private:
	Ui::TerrainSizeInputDialog ui;
	MainWindow* mainWin;

public:
	int side;
	int cellResolution;

public:
	TerrainSizeInputDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

