#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_BlendingVideoSettingDialog.h"

class MainWindow;

class BlendingVideoSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::BlendingVideoSettingDialog ui;
	MainWindow* mainWin;

public:
	bool useHDImage;
	float start;
	float end;
	float step;

public:
	BlendingVideoSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

