#ifndef QTPHONEDLG_H
#define QTPHONEDLG_H

#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QLabel>
#include <QMenu>
#include <QComboBox>
#include <QtGui/QMainWindow>
#include "ui_qtphone.h"
#include "myphoneendpoint.h"
#include "global.h"

class QtPhoneDlg : public QMainWindow
{
	Q_OBJECT

	enum states
	{
		STATE_READY,
		STATE_WORK,
		STATE_CALL
	};
	states				m_state;
	QMenu				*trayIconMenu;
	QSystemTrayIcon		*trayIcon;
	QComboBox			*iconComboBox;

	CMyPhoneEndPoint	*m_endpoint;

	void closeEvent(QCloseEvent *ev);

public:
	QtPhoneDlg(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QtPhoneDlg();

	PString			m_token;
	QLabel			*m_pLblStat;

   
	PTimer indTimer;
    PDECLARE_NOTIFIER(PTimer, QtPhoneDlg, OnUpdateIndicators);

    PTimer ringSoundTimer;
    PDECLARE_NOTIFIER(PTimer, QtPhoneDlg, OnRingSoundAgain);

	void OnUpdateIndicators();
	void OnRingSoundAgain();
  
	PString ringSoundFile;
 
	QString FindContactName(const H323Connection & connection);
	
	bool hidePnP;
	bool autoAddInAddr;
	bool autohideVideoPan;
	bool showVideoPan;
	bool hideStat;  // can show stat
	
	PConfig adrbook;  // PhoneBook file
	
	void ShowStats() const;
	int Close();
 
#if PTRACING	// If Compiled with PTlib Tracing support (like in OpenH323)
	bool Tracing;
    bool OpenTraceFile(PConfig & config);
	int TraceLevel;	// set -1 for no trace, or set trace detalization level (0-3).
    PTextFile * myTraceFile;
#endif

private:
	Ui::QtPhoneClass ui;
	void SendUserInput(const QString &strusermsg);

public slots:
	void slot_SendMsg();
	void slot_OnCall();
	void slot_OnRefuse();
	void slot_OnSettings();
	void slot_MuteMicrophoneCmd(bool);
	void slot_MuteSpeakerCmd(bool);
	void slot_RecvStats(const QString &txt);
	void slot_OutputStatus(const QString &text);
	void slot_OnConnectionEstablished(const QString &remotename);
	void slot_OnConnectionCleared(const QString &remotename);
	void slot_OnAnswerCall(const QString &remotename);
	void slot_ShowVideoPanels(bool show = true);
	void slot_SetLevelVolume(unsigned int val);
	void slot_SetLevelMic(unsigned int val);
	void slot_CreateConnection();
	void slot_OutputUserMsg(const QString &text);
	void slot_IconActivated(QSystemTrayIcon::ActivationReason reason);
	void slot_SetIcon(int index);
	void slot_Close();
	void about();
	void openContacts();

signals:
	void signal_SetLevelVolume(unsigned int val);
	void signal_SetLevelMic(unsigned int val);
	void signal_SetIcon(int index);
};

#endif // QTPHONEDLG_H
