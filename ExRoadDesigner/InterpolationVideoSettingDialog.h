#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_InterpolationVideoSettingDialog.h"

class MainWindow;

class InterpolationVideoSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::InterpolationVideoSettingDialog ui;
	MainWindow* mainWin;

public:
	bool useHDImage;
	float start;
	float end;
	float step;

public:
	InterpolationVideoSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

