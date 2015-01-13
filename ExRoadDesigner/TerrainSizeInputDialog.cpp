#include "TerrainSizeInputDialog.h"
#include "MainWindow.h"


TerrainSizeInputDialog::TerrainSizeInputDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditSide->setText("6000");
	ui.lineEditCellResolution->setText("200");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void TerrainSizeInputDialog::onOK() {
	side = ui.lineEditSide->text().toInt();
	cellResolution = ui.lineEditCellResolution->text().toInt();
	accept();
}

void TerrainSizeInputDialog::onCancel() {
	reject();
}
