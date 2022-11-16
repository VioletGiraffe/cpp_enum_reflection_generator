#include "mainwindow.h"

DISABLE_COMPILER_WARNINGS
#include "ui_mainwindow.h"

#include <QFile>
#include <QStringBuilder>
RESTORE_COMPILER_WARNINGS

#include <assert.h>
#include <utility>
#include <vector>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(ui->_sourceText, &QPlainTextEdit::textChanged, this, &MainWindow::generate);
}

MainWindow::~MainWindow()
{
	delete ui;
}

static QStringList preprocess(QString input) noexcept
{
	input.remove('\r').remove(' ').remove('\t');
	auto lines = input.split('\n', Qt::SkipEmptyParts);
	QString clean;
	for (auto& l: lines)
	{
		if (auto commentStart = l.indexOf("//"); commentStart != -1)
			l.truncate(commentStart);

		l = l.trimmed();
	}

	return lines.join(QString{}).split(',');
}

struct EnumNameAndType {
	QString name, type;
};

static EnumNameAndType parseEnumNameAndType(const QString& s) noexcept
{
	static constexpr const char enumClassString[]{ "enum class" };
	int enumPos = s.indexOf(enumClassString);
	int matchLength = (int)std::size(enumClassString);
	if (enumPos == -1)
	{
		static constexpr const char enumString[]{ "enum" };
		enumPos = s.indexOf(enumString);
		matchLength = (int)std::size(enumString);
	}

	const int openingBrace = s.indexOf('{', enumPos);

	EnumNameAndType result;
	const int colonPos = s.indexOf(':', enumPos);
	if (colonPos > 0 && colonPos < openingBrace)
	{
		result.name = s.mid(enumPos + matchLength, colonPos - enumPos - matchLength).trimmed();
		result.type = s.mid(colonPos + 1, openingBrace - colonPos).trimmed();
	}
	else
		result.name = s.mid(enumPos + matchLength, openingBrace - enumPos - matchLength).trimmed();

	return result;
}

void MainWindow::generate()
{
	QString enumText = ui->_sourceText->toPlainText();

	const auto enumNameAndType = parseEnumNameAndType(enumText);
	if (enumNameAndType.name.isEmpty())
	{
		ui->_generatedText->setPlainText("Invalid input!");
		return;
	}

	const auto openingBrace = enumText.indexOf('{');
	const auto closingBrace = enumText.lastIndexOf('}');

	if (openingBrace == -1 || closingBrace == -1)
	{
		ui->_generatedText->setPlainText("Invalid input!");
		return;
	}
	else
		enumText = enumText.mid(openingBrace + 1, closingBrace - openingBrace - 1);

	const auto items = preprocess(std::move(enumText));
	std::vector<std::pair<QString, QString>> parsedItems;
	for (auto&& item : items)
	{
		auto declaration = item.split('=');
		assert(declaration.size() == 0 || declaration.size() == 2);

		QString valueText;
		if (declaration.size() == 2)
			valueText = declaration[1];
		else if (!parsedItems.empty())
		{
			bool ok = false;
			const auto intValue = parsedItems.back().second.toLongLong(&ok);
			if (ok)
				valueText = QString::number(intValue + 1);

			const auto uintValue = parsedItems.back().second.toULongLong(&ok);
			if (ok)
				valueText = QString::number(uintValue + 1);
			else
			{
				ui->_generatedText->setPlainText("Invalid input!");
				return;
			}
		}
		else
			valueText = "0";

		parsedItems.emplace_back(declaration[0], std::move(valueText));
	}

	QFile templFile(":/EnumReflection_template.hpp");
	templFile.open(QFile::ReadOnly);
	QString templ = templFile.readAll();

	QString arrayItems;
	for (auto&& item : parsedItems)
	{
		if (!arrayItems.isEmpty())
			arrayItems += ",\n\t\t\t";

		arrayItems = arrayItems % "{ \"" % item.first % "\", " % item.second % " }";
	}

	templ = templ.arg(enumNameAndType.name).arg(parsedItems.size()).arg(arrayItems);
	ui->_generatedText->setPlainText(templ);
}
