#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Garganture.h"

class Garganture : public QMainWindow
{
	Q_OBJECT

public:
	Garganture(QWidget *parent = Q_NULLPTR);

private:
	Ui::GargantureClass ui;
};
