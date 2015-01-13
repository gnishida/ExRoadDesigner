#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_VideoCaptureSettingDialog.h"

class MainWindow;

class VideoCaptureSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::VideoCaptureSettingDialog ui;
	MainWindow* mainWin;

public:
	bool useHDImage;
	float cameraStep;

public:
	VideoCaptureSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

