#include "StructureDetectionSettingDialog.h"
#include "MainWindow.h"


StructureDetectionSettingDialog::StructureDetectionSettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditScale->setText("500.0");
	ui.lineEditDistance->setText("100");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void StructureDetectionSettingDialog::onOK() {
	scale = ui.lineEditScale->text().toFloat();
	distance = ui.lineEditDistance->text().toFloat();
	accept();
}

void StructureDetectionSettingDialog::onCancel() {
	reject();
}
