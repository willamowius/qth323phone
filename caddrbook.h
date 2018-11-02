#ifndef CADDRBOOK_H
#define CADDRBOOK_H

#include <QDialog>
#include <QCloseEvent>
#include "ui_caddrbook.h"

class CAddrBook : public QDialog
{
	Q_OBJECT

	void closeEvent(QCloseEvent *e);
	void accept();
	int saveIni();

public:
	CAddrBook(QWidget *parent = 0);
	~CAddrBook();
	int getAddr(QString &addr);

public slots:
	void addAddr();
	void delAddr();

private:
	Ui::CAddrBook ui;
};

#endif // CADDRBOOK_H
