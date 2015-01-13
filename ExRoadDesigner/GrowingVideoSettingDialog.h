#pragma once

#include "glew.h"

#include <QDockWidget>
#include "ui_GrowingVideoSettingDialog.h"

class MainWindow;

class GrowingVideoSettingDialog : public QDialog {
Q_OBJECT

private:
	Ui::GrowingVideoSettingDialog ui;
	MainWindow* mainWin;

public:
	QString generationMethod;
	bool useHDImage;
	int frequency;
	int numAvenueIterations;
	int numStreetIterations;

public:
	GrowingVideoSettingDialog(MainWindow* mainWin);

public slots:
	void onOK();
	void onCancel();
};

