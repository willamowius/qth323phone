#ifndef MYPHONEENDPOINT_H
#define MYPHONEENDPOINT_H

#include <QObject>

#include "global.h"
#include <ptlib/sound.h>
#include <ptlib/video.h>

#include <h323.h>



class QtPhoneDlg;
class CVideoDlg;

// Structure for stored connection statistics
typedef struct
{
	int iSecs;		// talk duration
	int ibSent;	// bytes sent
	int ibRcvd;	// bytes received
	int iDelay;	// rount trip delay
} MyStat;

class CMyPhoneEndPoint : public QObject, public H323EndPoint
{
	Q_OBJECT
public:
	CMyPhoneEndPoint();
	~CMyPhoneEndPoint();
	
	QtPhoneDlg		*m_dialog;
	CVideoDlg		*m_vdlg;
	PConfig			config;

	mutable MyStat	m_stat;

	//char **tvCaps;

    PBoolean OnIncomingCall(H323Connection &, const H323SignalPDU &, H323SignalPDU &);
	virtual H323Connection::AnswerCallResponse OnAnswerCall(H323Connection &, const PString & caller,const H323SignalPDU &,H323SignalPDU &);
	virtual void OnConnectionEstablished(H323Connection & connection, const PString & token);
	virtual void OnConnectionCleared(H323Connection & connection, const PString & clearedCallToken);
    PBoolean OpenAudioChannel(H323Connection &, PBoolean, unsigned, H323AudioCodec &);
    PBoolean OpenVideoChannel(H323Connection &, PBoolean, H323VideoCodec &);
    PBoolean OnStartLogicalChannel(H323Connection &, H323Channel &);
    void OnClosedLogicalChannel(H323Connection &, const H323Channel &);
	void OnRTPStatistics(const H323Connection &, const RTP_Session &) const;
    void TranslateTCPAddress(PIPSocket::Address &localAddr, const PIPSocket::Address &remoteAddr);
	virtual H323Connection * CreateConnection(unsigned refID);
	void OnLogicalChannel(const H323Channel & channel, char *txStrID, char *rxStrID);
	bool FindGatekeeper();


	bool Initialise(QtPhoneDlg *dlg);
	void LoadCapabilities(); 


	bool autoStartTransmitVideo;
	bool autoStartReceiveVideo;
	bool localVideo;
	bool localFlip;
	PIPSocket::Address m_router;
	static PString VideoInputDriver;

	bool m_fSilenceOn;
	bool m_fAutoAnswer;
	bool m_fNoFastStart;
	bool m_fDoH245Tunnelling;
	bool m_fDtmfAsString;

	inline void DisableH245Tunnelling(bool bDisable) { m_fDoH245Tunnelling = !bDisable; }
	inline bool DisableH245Tunnelling() { return !m_fDoH245Tunnelling; }

	friend class QtPhoneDlg;
	friend class CMyPhoneConnection;
	
	void OutputMsg(const QString &);
	void OutputUsrMsg(const QString &);

signals:
	void signal_RecvStats(const QString &);
	void signal_OutputMsg(const QString &);
	void signal_OutputUsrMsg(const QString &);
	void signal_OnConnectionEstablished(const QString &);
	void signal_OnConnectionCleared(const QString &);
	void signal_OnAnswerCall(const QString &);
	void signal_ShowVideoPanels(bool);
	void signal_CreateConnection();
};

class CMyPhoneConnection : public H323Connection
{
  PCLASSINFO(CMyPhoneConnection, H323Connection);

  public:
    CMyPhoneConnection(
		QtPhoneDlg* wnd,
		CMyPhoneEndPoint & ep,
		unsigned callReference,
		unsigned options);

    // overrides from H323Connection
    PBoolean OnAlerting(const H323SignalPDU &, const PString &);
    void OnUserInputString(const PString &);

    //virtual void SelectDefaultLogicalChannel( unsigned sessionID   ); 
  protected:
    CMyPhoneEndPoint		&endpoint;
    QtPhoneDlg			*m_dialog;
};

#endif // MYPHONEENDPOINT_H
