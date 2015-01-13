#include "CircleDetectionScaleInputDialog.h"
#include "MainWindow.h"


CircleDetectionScaleInputDialog::CircleDetectionScaleInputDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditScale->setText("500.0");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void CircleDetectionScaleInputDialog::onOK() {
	scale = ui.lineEditScale->text().toFloat();
	accept();
}

void CircleDetectionScaleInputDialog::onCancel() {
	reject();
}
