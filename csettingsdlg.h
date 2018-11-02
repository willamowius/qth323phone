#ifndef CSETTINGSDLG_H
#define CSETTINGSDLG_H

#include <QDialog>
#include "myphoneendpoint.h"
#include "ui_csettingsdlg.h"

class CMyPhoneEndPoint;

class CSettingsDlg : public QDialog
{
	Q_OBJECT

	CMyPhoneEndPoint *p_endpoint;

	void readIni();
	void saveIni();

public:
	CSettingsDlg(CMyPhoneEndPoint *m_endpoint, QWidget *parent = 0);
	~CSettingsDlg();

private:
	Ui::CSettingsDlg ui;
public slots:
	void slot_ComboBoxChange(int);
	void slot_ApplyChange();
	void slot_VCodecUp();
	void slot_VCodecDown();
	void slot_ACodecUp();
	void slot_ACodecDown();
	void slot_VCodecUse(bool);
	void slot_ACodecUse(bool);
	void slot_VItemChanged(QListWidgetItem *item, QListWidgetItem*);
	void slot_AItemChanged(QListWidgetItem *item, QListWidgetItem*);
	void slot_GetFile();
	void slot_PlayFile();
};

#endif // CSETTINGSDLG_H
