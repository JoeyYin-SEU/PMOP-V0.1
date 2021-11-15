#pragma once

#include <QWidget>
#include "ui_Camera_op_opt.h"

class Camera_op_opt : public QWidget
{
	Q_OBJECT

public:
	Camera_op_opt(QWidget *parent = Q_NULLPTR);
	~Camera_op_opt();

private:
	Ui::Camera_op_opt ui;
};
