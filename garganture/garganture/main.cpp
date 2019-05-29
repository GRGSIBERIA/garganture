#include "Garganture.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	Garganture w;
	w.show();
	return a.exec();
}
