#ifndef CVIDEODLG_H
#define CVIDEODLG_H

#include <QDialog>
#include <QMutex>
#include "ui_cvideodlg.h"

class QtPhoneDlg;

class CVideoDlg : public QDialog
{
	Q_OBJECT
	
	bool eventFilter(QObject *obj, QEvent *event);
	void closeEvent(QCloseEvent *ev);

	QtPhoneDlg *p_dlg;

public:
	CVideoDlg(QtPhoneDlg *dlg);
	~CVideoDlg();

	QMutex	m_mutex;
	QImage	m_image;

private:
	Ui::CVideoDlg ui;

};

#endif // CVIDEODLG_H
