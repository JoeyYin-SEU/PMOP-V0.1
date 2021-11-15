#pragma once

#include <QDialog>
#include "ui_DiaCamSel.h"

class DiaCamSel : public QDialog
{
	Q_OBJECT

public:
	DiaCamSel(QWidget *parent = Q_NULLPTR);
	~DiaCamSel();
	int modu=-1;

private:
	Ui::DiaCamSel ui;

private slots:
	void Ok_clicked();
	void Cancel_clicked();
};
