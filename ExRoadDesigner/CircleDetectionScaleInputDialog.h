#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_CircleDetectionScaleInputDialog.h"

class MainWindow;

class CircleDetectionScaleInputDialog : public QDialog {
Q_OBJECT

private:
	Ui::CircleDetectionScaleInputDialog ui;
	MainWindow* mainWin;

public:
	float scale;

public:
	CircleDetectionScaleInputDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

