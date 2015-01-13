#include "GrowingVideoSettingDialog.h"
#include "MainWindow.h"


GrowingVideoSettingDialog::GrowingVideoSettingDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.checkBoxHDImage->setChecked(false);
	ui.lineEditFrequency->setText("10");
	ui.lineEditNumAvenueIterations->setText("650");
	ui.lineEditNumStreetIterations->setText("1200");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void GrowingVideoSettingDialog::onOK() {
	generationMethod = ui.comboBoxGenerationMethod->currentText();
	useHDImage = ui.checkBoxHDImage->isChecked();
	frequency = ui.lineEditFrequency->text().toInt();
	numAvenueIterations = ui.lineEditNumAvenueIterations->text().toInt();
	numStreetIterations = ui.lineEditNumStreetIterations->text().toInt();
	accept();
}

void GrowingVideoSettingDialog::onCancel() {
	reject();
}
