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

static auto parseNumber(const QString& s) noexcept
{
	int base = 10;
	if (s.startsWith(QLatin1String("0x"), Qt::CaseInsensitive))
		base = 16;
	else if (s.startsWith(QLatin1String("0b"), Qt::CaseInsensitive))
		base = 2;
	else if (s.startsWith(QLatin1String("0"), Qt::CaseInsensitive))
		base = 8;

	[[maybe_unused]] bool ok = false;
	const auto n = s.toLongLong(&ok, base);
	assert(ok);
	return n;
}

static QString parseEnumName(const QString& s) noexcept
{
	const auto openingBrace = s.indexOf('{');
	static constexpr const char enumClassString[]{ "enum class" };
	auto enumPos = s.indexOf(enumClassString);
	int matchLength = (int)std::size(enumClassString);
	if (enumPos == -1)
	{
		static constexpr const char enumString[]{ "enum" };
		enumPos = s.indexOf(enumString);
		matchLength = (int)std::size(enumString);
	}

	QString enumName = s.mid(enumPos + matchLength, openingBrace - enumPos - matchLength);
	return enumName.trimmed();
}

void MainWindow::generate()
{
	QString enumText = ui->_sourceText->toPlainText();

	const auto enumName = parseEnumName(enumText);
	if (enumName.isEmpty())
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
	using EnumType = int64_t;
	std::vector<std::pair<QString, EnumType>> parsedItems;
	for (auto&& item : items)
	{
		auto declaration = item.split('=');
		assert(declaration.size() == 1 || declaration.size() == 2);

		EnumType value;
		if (declaration.size() == 2)
			value = parseNumber(declaration[1]);
		else
			value = parsedItems.empty() ? 0 : parsedItems.back().second + 1;

		parsedItems.emplace_back(declaration[0], value);
	}

	QFile templFile(":/EnumReflection_template.hpp");
	templFile.open(QFile::ReadOnly);
	QString templ = templFile.readAll();

	QString arrayItems;
	for (auto&& item : parsedItems)
	{
		if (!arrayItems.isEmpty())
			arrayItems += ",\n\t\t\t";

		arrayItems = arrayItems % "{ \"" % item.first % "\", " % QString::number(item.second) % " }";
	}

	templ = templ.arg(enumName).arg(parsedItems.size()).arg(arrayItems);
	ui->_generatedText->setPlainText(templ);
}
