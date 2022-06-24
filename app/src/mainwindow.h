#pragma once
#include "compiler/compiler_warnings_control.h"

DISABLE_COMPILER_WARNINGS
#include <QMainWindow>
RESTORE_COMPILER_WARNINGS

namespace Ui {
class MainWindow;
}

class MainWindow final : public QMainWindow
{
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private:
	void generate();

private:
	Ui::MainWindow *ui;
};
