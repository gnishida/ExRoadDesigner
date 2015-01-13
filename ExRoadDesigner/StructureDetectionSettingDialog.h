#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_StructureDetectionSettingDialog.h"

class MainWindow;

class StructureDetectionSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::StructureDetectionSettingDialog ui;
	MainWindow* mainWin;

public:
	float scale;
	float distance;

public:
	StructureDetectionSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

