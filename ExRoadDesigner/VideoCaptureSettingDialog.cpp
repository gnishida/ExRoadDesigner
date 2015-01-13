#include "VideoCaptureSettingDialog.h"
#include "MainWindow.h"


VideoCaptureSettingDialog::VideoCaptureSettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.checkBoxHDImage->setChecked(false);
	ui.lineEditCameraStep->setText("15");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void VideoCaptureSettingDialog::onOK() {
	useHDImage = ui.checkBoxHDImage->isChecked();
	cameraStep = ui.lineEditCameraStep->text().toFloat();
	accept();
}

void VideoCaptureSettingDialog::onCancel() {
	reject();
}
