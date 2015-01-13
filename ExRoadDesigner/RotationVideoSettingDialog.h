#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_RotationVideoSettingDialog.h"

class MainWindow;

class RotationVideoSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::RotationVideoSettingDialog ui;
	MainWindow* mainWin;

public:
	bool useHDImage;
	float start;
	float end;
	float step;

public:
	RotationVideoSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

