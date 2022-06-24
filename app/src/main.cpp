#include "mainwindow.h"

DISABLE_COMPILER_WARNINGS
#include <QApplication>
RESTORE_COMPILER_WARNINGS

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	Q_INIT_RESOURCE(resources);

	MainWindow mw;
	mw.show();

	return app.exec();
}
