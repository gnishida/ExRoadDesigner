#include "RotationVideoSettingDialog.h"
#include "MainWindow.h"


RotationVideoSettingDialog::RotationVideoSettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.checkBoxHDImage->setChecked(false);
	ui.lineEditStart->setText("0.0");
	ui.lineEditEnd->setText("1.0");
	ui.lineEditStep->setText("0.01");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void RotationVideoSettingDialog::onOK() {
	useHDImage = ui.checkBoxHDImage->isChecked();
	start = ui.lineEditStart->text().toFloat();
	end = ui.lineEditEnd->text().toFloat();
	step = ui.lineEditStep->text().toFloat();
	accept();
}

void RotationVideoSettingDialog::onCancel() {
	reject();
}
