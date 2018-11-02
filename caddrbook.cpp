#include "caddrbook.h"
#include <QFile>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QDomDocument>
#include <QTextStream>

#define FILE_NAME	"contacts.ini"

CAddrBook::CAddrBook(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.tbl->verticalHeader()->setDefaultSectionSize(20);

	connect(ui.cmdAdd, SIGNAL(clicked()), this, SLOT(addAddr()));
	connect(ui.cmdDel, SIGNAL(clicked()), this, SLOT(delAddr()));

	QFile file(FILE_NAME);
	if (!file.open(QFile::ReadOnly | QFile::Text))
		return;

	QString text;
	QIODevice *device = &file;
	int errorLine;
	int errorColumn;
	QDomDocument document;
	if (!document.setContent(device, true, &text, &errorLine,
		&errorColumn))
	{
		QMessageBox::information(this, tr("Ошибка чтения файла"),
			QString("Файл: %1, строка: %2; %3").arg(text).arg(errorLine).arg(text),
			QMessageBox::Ok);
		return;
	}
	file.close();

	QTableWidgetItem *newItem;
	QDomElement elem = document.documentElement().firstChildElement("contact");
	int row = document.documentElement().elementsByTagName("contact").size();
	ui.tbl->setRowCount(row);

	row = 0;

	while (!elem.isNull())
	{
		newItem = new QTableWidgetItem(elem.attribute("name"));
		ui.tbl->setItem(row, 0, newItem);

		newItem = new QTableWidgetItem(elem.attribute("addr"));
		ui.tbl->setItem(row, 1, newItem);

		row++;

		elem = elem.nextSiblingElement("contact");
	}
}

CAddrBook::~CAddrBook()
{
}

void CAddrBook::closeEvent(QCloseEvent *e)
{
	if(!saveIni())
		e->ignore();
}

void CAddrBook::accept()
{
	if(saveIni())
		QDialog::accept();
}

int CAddrBook::saveIni()
{
	QDomDocument document;
	QDomProcessingInstruction processingInstruction = document.createProcessingInstruction("xml",
		"version=\"1.0\" encoding=\"UTF-8\"");
	document.appendChild(processingInstruction);
	QDomElement rootElement = document.createElement("Contacts");
	rootElement.setAttribute("version", "1.0");
	document.appendChild(rootElement);

	QDomElement elem;
	int i, n = ui.tbl->rowCount();
	for(i=0;i<n;i++)
	{
		elem = document.createElement("contact");
		elem.setAttribute("name", ui.tbl->item(i, 0)->text());
		elem.setAttribute("addr", ui.tbl->item(i, 1)->text());
		rootElement.appendChild(elem);
	}

	QFile file(FILE_NAME);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		QMessageBox::information(this, tr("Внимание!"),
			tr("Невозможно создать файл"), QMessageBox::Ok);
		return 0;
	}
	QIODevice *device = &file;
	QTextStream out(device);
	document.save(out, 4);
	file.close();
	return 1;
}

int CAddrBook::getAddr(QString &addr)
{
	int rc;

	rc = exec();
	if(rc)
	{
		QTableWidgetItem *p_item = ui.tbl->currentItem();
		if(!p_item)
			return 0;
		p_item = ui.tbl->item(p_item->row(), 1);
		if(!p_item)
			return 0;
		addr = p_item->text();
	}

	return rc;
}

void CAddrBook::addAddr()
{
	QTableWidgetItem *p_item = ui.tbl->currentItem();
	int row = 0;
	if(p_item)
		row = p_item->row()+1;
	ui.tbl->insertRow(row);
}

void CAddrBook::delAddr()
{
	int row = ui.tbl->currentRow();
	QTableWidgetItem *p_item;
	QString name;

	p_item = ui.tbl->item(row, 0);
	if(p_item)
		name = p_item->text();

	if(QMessageBox::question(this, tr("Удаление контакта"),
		tr("Вы действительно хотите удалить \"%1\"?").arg(name),
		tr("Да"), tr("Нет"), tr("Отмена"))!=0)
		return;

	ui.tbl->removeRow(row);
}
