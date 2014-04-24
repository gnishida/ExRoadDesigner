#include "TerrainSizeInputDialog.h"
#include "MainWindow.h"


TerrainSizeInputDialog::TerrainSizeInputDialog(MainWindow* mainWin) : QDialog((QWidget*)mainWin) {
	this->mainWin = mainWin;

	// set up the UI
	ui.setupUi(this);
	ui.lineEditWidth->setText("6000");
	ui.lineEditDepth->setText("6000");
	ui.lineEditCellLength->setText("100");

	// register the event handlers
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(onOK()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));

	hide();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event handlers

void TerrainSizeInputDialog::onOK() {
	width = ui.lineEditWidth->text().toInt();
	depth = ui.lineEditDepth->text().toInt();
	cellLength = ui.lineEditCellLength->text().toInt();
	accept();
}

void TerrainSizeInputDialog::onCancel() {
	reject();
}
