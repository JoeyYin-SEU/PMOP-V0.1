#include "DiaCamSel.h"

DiaCamSel::DiaCamSel(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.okButton, SIGNAL(clicked()), this, SLOT(Ok_clicked()));
	connect(ui.cancelButton, SIGNAL(clicked()), this, SLOT(Cancel_clicked()));
}

DiaCamSel::~DiaCamSel()
{
}


void DiaCamSel::Ok_clicked()
{
	modu = ui.comboBox->currentIndex();
	this->close();
}

void DiaCamSel::Cancel_clicked()
{
	modu = -1;
	this->close();
}