#include "cvideodlg.h"
#include "qtphonedlg.h"
#include "cvideodevice.h"
#include "myphoneendpoint.h"
#include "global.h"

//PluginLoaderStartup2  PWLibProcess::pluginLoader;
//H323PluginCodecManager *PWLibProcess::plugmgr=NULL;

//PString CMyPhoneEndPoint::VideoInputDriver = "FakeVideo";
PString CMyPhoneEndPoint::VideoInputDriver = "*";
//PString CMyPhoneEndPoint::VideoInputDriver = "V4L";
//PString CMyPhoneEndPoint::VideoInputDriver = "DirectShow";
//PString CMyPhoneEndPoint::VideoInputDriver = "VideoForWindows";

CMyPhoneEndPoint::CMyPhoneEndPoint()
{
	puts("=============PVideoInputDevice::GetDriverNames()================");
	PStringList drvNames = PVideoInputDevice::GetDriverNames();
	for(int i=0;i<drvNames.GetSize();i++)
	{
		puts((const char*)drvNames[i]);
	}
	puts("==========================================================");

	m_dialog = NULL;
	m_vdlg = NULL;
	m_stat.iSecs=0;  m_stat.ibSent=0;  m_stat.ibRcvd=0;  m_stat.iDelay=0;
	m_router = NULL;
	localVideo = FALSE;
}

CMyPhoneEndPoint::~CMyPhoneEndPoint()
{
	delete m_vdlg;
}

bool CMyPhoneEndPoint::Initialise(QtPhoneDlg *dlg)
{
	m_dialog = dlg;
	m_vdlg=new CVideoDlg(m_dialog);

	//////////////////////////// PConfig ////////////////////////////
	SetAudioJitterDelay(50, config.GetInteger(JitterConfigKey, 50)); //GetMaxAudioJitterDelay()));
	SetSoundChannelBufferDepth(config.GetInteger(BufferCountConfigKey, GetSoundChannelBufferDepth()));

	// UserInput mode
    // Backward compatibility configuration entry
    unsigned mode = H323Connection::SendUserInputAsString;
	m_fDtmfAsString=true;
    if (config.HasKey(DtmfAsStringConfigKey))
	{
		if (!config.GetBoolean(DtmfAsStringConfigKey))
		{
			mode = H323Connection::SendUserInputAsTone;
			m_fDtmfAsString=false;
		}
		config.DeleteKey(DtmfAsStringConfigKey);
	}
	mode = config.GetInteger(UserInputModeConfigKey, mode);
	SetSendUserInputMode((H323Connection::SendUserInputModes)mode);

	int i;
	PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
	puts("=============pluginMgr.GetPluginsProviding(PSoundChannel)================");
	//PStringList drvNames = PVideoInputDevice::GetDriverNames();
	PStringList listA = pluginMgr.GetPluginsProviding("PSoundChannel");
	for(i=0;i<listA.GetSize();i++)
	{
		puts((const char*)listA[i]);
		if(i==0)
		{
			SetSoundChannelPlayDriver(listA[i]);
			SetSoundChannelRecordDriver(listA[i]);
		}
	}
	puts("=============pluginMgr.GetPluginsProviding(*)================");
	listA = pluginMgr.GetPluginsProviding("*");
	for(i=0;i<listA.GetSize();i++)
	{
		puts((const char*)listA[i]);
	}
	puts("========================PSoundChannel==================================");
	listA = pluginMgr.GetPluginsDeviceNames("*", "PSoundChannel");
	for(i=0;i<listA.GetSize();i++)
	{
		puts((const char*)listA[i]);
	}
	puts("==========================PVideoInputDevice================================");
	listA = pluginMgr.GetPluginsDeviceNames("*", "PVideoInputDevice");
	for(i=0;i<listA.GetSize();i++)
	{
		puts((const char*)listA[i]);
	}
	puts("==========================PVideoOutputDevice================================");
	listA = pluginMgr.GetPluginsDeviceNames("*", "PVideoOutputDevicel");
	for(i=0;i<listA.GetSize();i++)
	{
		puts((const char*)listA[i]);
	}
	puts("==========================================================");


	//SetSoundChannelPlayDriver("WindowsMutimedia");
	//SetSoundChannelRecordDriver("WindowsMultimedia");

	// set record and play devices
	bool fPlayerAvailable = SetSoundChannelPlayDevice(
		config.GetString(SoundPlayConfigKey,   GetSoundChannelPlayDevice()));
	bool fRecorderAvailable = SetSoundChannelRecordDevice(
		config.GetString(SoundRecordConfigKey, GetSoundChannelRecordDevice()));

	// set some other settings from Config
	m_fNoFastStart =  config.GetBoolean(NoFastStartConfigKey, false);
    DisableFastStart(m_fNoFastStart);
	m_fDoH245Tunnelling = !(config.GetBoolean(NoTunnelingConfigKey, false));
    DisableH245Tunneling(!m_fDoH245Tunnelling);
	m_fSilenceOn = config.GetBoolean(SilenceDetectConfigKey, false);
    SetSilenceDetectionMode(m_fSilenceOn
		? H323AudioCodec::AdaptiveSilenceDetection
		: H323AudioCodec::NoSilenceDetection);
    SetLocalUserName(config.GetString(UsernameConfigKey, GetLocalUserName()));
    SetInitialBandwidth((unsigned)(config.GetReal(BandwidthConfigKey, 10000)*20));
    SetRtpIpTypeofService(config.GetInteger(IpTosConfigKey, GetRtpIpTypeofService()));

	SetRtpIpPorts(config.GetInteger(RTPPortBaseConfigKey, GetRtpIpPortBase()),
		config.GetInteger(RTPPortMaxConfigKey,  GetRtpIpPortBase()));
	if(config.HasKey(RouterConfigKey))
		m_router = config.GetString(RouterConfigKey, m_router.AsString());

	m_fAutoAnswer = config.GetBoolean(AutoAnswerConfigKey, true);

    QString alias, aliases;
	aliases = QString((const char *)config.GetString(AliasConfigKey, ""));
	puts(aliases.toAscii().data());
	aliases.simplified();
	int iPos=0;
	while ((iPos = aliases.indexOf("|"))>0)  // loading user aliases
	{
		alias = aliases.left(iPos);
		aliases = aliases.mid(iPos+1);
		AddAliasName(alias.toAscii().data());
	}

	// The order in which capabilities are added to the capability table
	// determines which one is selected by default.
	LoadCapabilities();

	QString str;
	PString interfaces = config.GetString(ListenerInterfaceConfigKey, "*");
	if(StartListeners(interfaces.Tokenise(',')))
	{
		str = QString("%1 готов к приему...").arg(strProgName);
		emit signal_OutputMsg(str);
		return true;
	}
	else
	{
		str = QString("ОШИБКА! %1 не готов к приему, интерфейс: %2.").arg(strProgName).arg((const char*)interfaces);
		emit signal_OutputMsg(str);
		return false;
	}
	return true;
}

PBoolean CMyPhoneEndPoint::OnIncomingCall(H323Connection &, const H323SignalPDU &, H323SignalPDU &)
{
	puts("CMyPhoneEndPoint::OnIncomingCall");
	return TRUE;
}

H323Connection::AnswerCallResponse CMyPhoneEndPoint::OnAnswerCall(H323Connection &connection, const PString &,const H323SignalPDU &signalPDUconst, H323SignalPDU &)
{
	puts("CMyPhoneEndPoint::OnAnswerCall");

	if(HasConnection(m_dialog->m_token))
	//if(!m_dialog->m_token.IsEmpty())
		return H323Connection::AnswerCallDenied;

	m_dialog->m_token = connection.GetCallToken();

	emit signal_OnAnswerCall(m_dialog->FindContactName(connection));

	// Check for auto answer option
	if (m_fAutoAnswer)
	{
		m_dialog->ringSoundTimer.Stop();
		return H323Connection::AnswerCallNow;
	}

	if (!m_dialog->ringSoundFile)
	{
		PSound::PlayFile(m_dialog->ringSoundFile, FALSE);
		m_dialog->ringSoundTimer.RunContinuous(5000);
	}

	return H323Connection::AnswerCallPending;
}

void CMyPhoneEndPoint::OnConnectionEstablished(H323Connection & connection, const PString & token)
{
	m_dialog->m_token = token;
	const char *goodName = (const char*)connection.GetRemotePartyName();
	// store last number in combobox
	if(!connection.HadAnsweredCall())
	{
		puts("CMyPhoneEndPoint::OnConnectionEstablished - !connection.HadAnsweredCall()");
		//m_dialog->PhoneBookAddCall(1, CString((LPCTSTR)connection.GetRemotePartyName()), m_dialog->GetSimpleAdr(CString((LPCTSTR)connection.GetRemotePartyAddress())));
		//m_dialog->StoreDestonation(goodName /*(const char *)connection.GetRemotePartyName()*/);
	}
	else
	{
		puts("CMyPhoneEndPoint::OnConnectionEstablished - connection.HadAnsweredCall()");
		// store in Address Book
		//m_dialog->PhoneBookAddCall(2, CString((LPCTSTR)connection.GetRemotePartyName()), m_dialog->GetSimpleAdr(CString((LPCTSTR)connection.GetRemotePartyAddress())));
		//if(m_dialog->autoAddInAddr)
		//	m_dialog->StoreDestonation(goodName /*(const char *)connection.GetRemotePartyName()*/, TRUE);
	}
	emit signal_OnConnectionEstablished(m_dialog->FindContactName(connection));
}

void CMyPhoneEndPoint::OnConnectionCleared(H323Connection & connection, const PString & clearedCallToken)
{
	if(connection.GetCallEndReason()==H323Connection::EndedByRemoteBusy ||
		connection.GetCallEndReason()==H323Connection::EndedByRefusal)
	{
		QString msg = QString("У %1 занято...").arg(m_dialog->FindContactName(connection));
		emit signal_OutputMsg(msg);
	}

	if(m_dialog->m_token != connection.GetCallToken())
	//if(HasConnection(m_dialog->m_token))
		return;

	// clearing statistic
	m_stat.iSecs=0;
	m_stat.ibSent=0;
	m_stat.ibRcvd=0;
	m_stat.iDelay=0;

	// UI change
	emit signal_OnConnectionCleared(m_dialog->FindContactName(connection));
}

PBoolean CMyPhoneEndPoint::OpenAudioChannel(H323Connection &connection, PBoolean isEncoding, unsigned bufferSize, H323AudioCodec &codec)
{
	return H323EndPoint::OpenAudioChannel(connection, isEncoding, bufferSize, codec);
}

PBoolean CMyPhoneEndPoint::OpenVideoChannel(H323Connection &connection, PBoolean isEncoding, H323VideoCodec &codec)
{
	PVideoChannel   * channel = new PVideoChannel;
	PVideoDevice * displayDevice = NULL;

	if (isEncoding)
	{
		// Transmitter part
		if(!autoStartTransmitVideo)
			return FALSE;

		int m_Quality = config.GetInteger(VideoQualityConfigKey, 15);
		printf("Quality=%d\n", m_Quality);
		codec.SetTxQualityLevel(m_Quality);
		//codec.SetBackgroundFill(2);

		int videoOutMaxBitRate = config.GetInteger(VideoOutMaxbandWidthKey, 320);
		videoOutMaxBitRate = 1024 * PMAX(16, PMIN(10240, videoOutMaxBitRate));

		H323Channel * lchannel = codec.GetLogicalChannel();
		const H323Capability & capability = lchannel->GetCapability();
		PString cname = capability.GetFormatName();
		PINDEX suffixPos = cname.Find("H.263");
		if(suffixPos == P_MAX_INDEX)
			suffixPos = cname.Find("H.261");

		int videoSize = config.GetInteger(VideoOutSizeConfigKey, 2);
		int width=352, height=288;

		suffixPos = P_MAX_INDEX;
		switch(videoSize)
		{
		case 0: //QCIF
			width = 176; height = 144; break;
		case 1: //QVGA
			if(suffixPos == P_MAX_INDEX) { width = 320; height = 240; break; }
		case 2: //CIF
			width = 352; height = 288; break;
		case 3: //VGA
			if(suffixPos == P_MAX_INDEX) { width = 640; height = 480; break; }
		case 5: //SVGA
			if(suffixPos == P_MAX_INDEX) { width = 800; height = 600; break; }
		case 6: //XVGA
			if(suffixPos == P_MAX_INDEX) { width = 1024; height = 768; break; }
		case 4: //4CIF
			width = 704; height = 576; break;
		case 7: //SXGA
			if(suffixPos == P_MAX_INDEX) { width = 1200; height = 1024; break; }
		case 8: //16CIF
			width = 1408; height = 1152; break;
		case 9: //UXGA
			if(suffixPos == P_MAX_INDEX) { width = 1600; height = 1200; break; }
		default:
			break;
		}

		printf("width=%d, height=%d\n", width, height);

 //       codec.SetGeneralCodecOption(OpalVideoFormat::FrameWidthOption, width);
 //       codec.SetGeneralCodecOption(OpalVideoFormat::FrameHeightOption, height);
		codec.SetFrameSize(width, height);

		width = codec.GetWidth();
		height = codec.GetHeight();

		printf("width=%d, height=%d\n", width, height);

		int curMBR = codec.GetMaxBitRate();
		printf("curMBR=%d, videoOutMaxBitRate=%d\n", curMBR, videoOutMaxBitRate);
		if(curMBR > videoOutMaxBitRate)
		//if(curMBR != videoOutMaxBitRate)
			codec.SetMaxBitRate(videoOutMaxBitRate);

		int videoFramesPS = config.GetInteger(VideoFPSKey, 10);
		codec.SetGeneralCodecOption("Frame Rate",videoFramesPS);

		printf("Frame Rate=%d\n", videoFramesPS);


#if PTLIB_MAJOR<=2 && PTLIB_MINOR<10
		codec.cacheMode = 1;
#endif
		//Create grabber.
		bool NoDevice = false;
		QString devSName(config.GetString(VideoDeviceConfigKey, ""));
		PString deviceName = devSName.toUtf8().data();
		if (deviceName.IsEmpty())
		{
			PStringArray devices = PVideoInputDevice::GetDriversDeviceNames(VideoInputDriver);
			if (!devices.IsEmpty())
				deviceName = devices[0];
			else
				NoDevice = true;
		}
		printf("deviceName=%s\n", (const char*)deviceName);

		PVideoInputDevice * grabber = NULL;
		if (deviceName.Find("fake") == 0)
			NoDevice = true;
		else
			grabber = PVideoInputDevice::CreateDeviceByName(deviceName,VideoInputDriver);

		bool rc1, rc = false;
		if (!NoDevice && grabber)
		{
#ifdef WIN32
			rc1 = grabber->SetColourFormatConverter("YUV420P");
			printf("grabber->SetColourFormatConverter = %d\n", (int)rc1);
#endif
			rc = grabber->Open(deviceName, FALSE);
/*
				if(!rc)
				{
//					rc1 = grabber->SetFrameSize(width, height);
					if(rc1)
					{
//						rc1 = grabber->Open(deviceName, FALSE);
					}
				}
*/
#ifndef WIN32
			rc1 = grabber->SetColourFormatConverter("YUV420P");
			printf("grabber->SetColourFormatConverter = %d\n", (int)rc1);
#endif
			rc1 = grabber->SetFrameSize(width, height);
			printf("grabber->SetFrameSize = %d\n", (int)rc1);

			if(!rc)
				rc = grabber->Open(deviceName, FALSE);
		}

		if (NoDevice || !rc
//			|| !grabber->SetColourFormatConverter("YUV420P")
//			|| !grabber->Open(deviceName, FALSE)
//			|| !grabber->SetFrameSize(width, height)
			)
		{
			if(!NoDevice)
			{
				QString msg = QString("ОШИБКА! Не могу открыть устройство: %1; канал: %2")
					.arg((const char *) deviceName)
					.arg(config.GetInteger(VideoSourceConfigKey,0));
				emit signal_OutputMsg(msg);
				PTRACE(1, "Failed to open or configure the video device \"" << deviceName << '"');
			}

			if(grabber)
				delete grabber;
			grabber = PVideoInputDevice::CreateDevice("FakeVideo");
			grabber->SetColourFormat("YUV420P");
			grabber->SetVideoFormat(PVideoDevice::PAL);
			grabber->SetFrameSize(width, height);
			//grabber->SetVFlipState(localFlip);
			grabber->SetChannel(4);
		}

		if(videoFramesPS >0 && videoFramesPS<30)
			grabber->SetFrameRate(videoFramesPS);

		bool curVFlip = config.GetBoolean(VideoOutVFlipConfigKey, FALSE);
		grabber->SetVFlipState(curVFlip);

		grabber->Start();

		channel->AttachVideoReader(grabber);
		displayDevice = PVideoOutputDevice::CreateDevice("NULLOutput");
	}
	else
	{
		// Receiver part
		if(!autoStartReceiveVideo)
			return FALSE;
		bool curVFlip = config.GetBoolean(VideoInVFlipConfigKey, FALSE);
		bool curHFlip = config.GetBoolean(VideoInHFlipConfigKey, FALSE);

		displayDevice = new CVideoOutputDevice(
			this, curVFlip, curHFlip, FALSE, localVideo);
	}

	if(displayDevice)
	{
		int width = codec.GetWidth();
		int height = codec.GetHeight();
		displayDevice->SetColourFormat("RGB32");
		displayDevice->SetColourFormatConverter("YUV420P");
		displayDevice->SetFrameSize(width, height);
		channel->AttachVideoPlayer((PVideoOutputDevice *)displayDevice);
	}

	return codec.AttachChannel(channel,TRUE);
}

PBoolean CMyPhoneEndPoint::OnStartLogicalChannel(H323Connection &, H323Channel &channel)
{
	OnLogicalChannel(channel, "Отправляем %1 данные %2.", "Получаем %1 данные %2.");
	return TRUE;
}

void CMyPhoneEndPoint::OnClosedLogicalChannel(H323Connection &, const H323Channel &channel)
{
	OnLogicalChannel(channel, "Прекратили отправлять %1 данные %2.", "Прекратили получать %1 данные %2.");
}

void CMyPhoneEndPoint::OnRTPStatistics(const H323Connection &connection, const RTP_Session &session) const
{
	//puts("CMyPhoneEndPoint::OnRTPStatistics");

	m_stat.ibSent = m_stat.ibRcvd = 0;

	if(m_dialog->hideStat)
		return;

    // Getting Audio statistic part
	RTP_Session * asession = connection.GetSession(RTP_Session::DefaultAudioSessionID);
	if (asession != NULL)
	{
		int abSent=0, abRcvd=0;
		// If not sending or receiving Audio, memory read error happens.
		try{	abSent = asession->GetOctetsSent();	} catch(...) {abSent = 0;}
		try{	abRcvd = asession->GetOctetsReceived(); } catch(...) {abRcvd = 0;}
		m_stat.ibSent = abSent;
		m_stat.ibRcvd = abRcvd;
	}

	// Getting Video statistic part
	RTP_Session * vsession = connection.GetSession(RTP_Session::DefaultVideoSessionID);
	if (vsession != NULL)
	{
		int vbSent=0, vbRcvd=0;
		// If not sending or receiving Vidio
		// mamory read error happends.
		try{	vbSent = vsession->GetOctetsSent();	} catch(...) {vbSent = 0;}
		try{	vbRcvd = vsession->GetOctetsReceived(); } catch(...) {vbRcvd = 0;}
		m_stat.ibSent += vbSent;
		m_stat.ibRcvd += vbRcvd;
	}

	m_stat.iDelay = connection.GetRoundTripDelay().GetInterval();

	m_stat.iSecs = (PTime() - connection.GetConnectionStartTime()).GetSeconds();

	m_dialog->ShowStats();
}

void CMyPhoneEndPoint::TranslateTCPAddress(PIPSocket::Address &localAddr, const PIPSocket::Address &remoteAddr)
{
	if (m_router.IsValid() && IsLocalAddress(localAddr) && !IsLocalAddress(remoteAddr))
	{
		puts("CMyPhoneEndPoint::TranslateTCPAddress");
		localAddr = m_router;
	}
}

H323Connection * CMyPhoneEndPoint::CreateConnection(unsigned refID)
{
	puts("CMyPhoneEndPoint::CreateConnection");

	//if(m_dialog->autohideVideoPan && m_dialog->showVideoPan )

	emit signal_CreateConnection();

	return new CMyPhoneConnection(m_dialog, *this, refID, 0);
}

void CMyPhoneEndPoint::OnLogicalChannel(const H323Channel & channel, char *txStrID, char *rxStrID)
{
	puts("CMyPhoneEndPoint::OnLogicalChannel");
	const H323Capability & capability = channel.GetCapability();
	PString name = capability.GetFormatName();
	PString frames;

	if (capability.GetMainType() == H323Capability::e_Video)
	{
		unsigned numFrames = channel.GetDirection() == H323Channel::IsTransmitter
			? capability.GetTxFramesInPacket()
			: capability.GetRxFramesInPacket();
		frames.sprintf(" (%u frames)", numFrames);
	}

	QString msg;

	switch (channel.GetDirection())
	{
    case H323Channel::IsTransmitter :
		msg = QString(txStrID).arg((const char *)name).arg((const char *)frames);
		break;

    case H323Channel::IsReceiver :
		msg = QString(rxStrID).arg((const char *)name).arg((const char *)frames);
		break;

    default:
		break;
	}

	emit signal_OutputMsg(msg);
}

void CMyPhoneEndPoint::LoadCapabilities()
{

	bool sizeChange = FALSE;

	capabilities.RemoveAll();

	// Add the codecs we know about
	AddAllCapabilities(0, 0, "*");


	// Удаляю не поддерживаемые видео кодеки из реестра
	PINDEX videoNum = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++videoNum);
		PString name = config.GetString(VideoCodecsConfigSection, key, "");
		if (name.IsEmpty())
			break;

		PINDEX suffixPos = name.Find(OnCodecSuffix);
		if (suffixPos != P_MAX_INDEX)
			name.Delete(suffixPos, P_MAX_INDEX);
		else
		{
			suffixPos = name.Find(OffCodecSuffix);
			name.Delete(suffixPos, P_MAX_INDEX);
		}

		int res = 0;
		for (PINDEX i = 0; i < capabilities.GetSize(); i++)
		{
			if (capabilities[i].GetMainType() == H323Capability::e_Video)
			{
				if(capabilities[i].GetFormatName() == name)
				{ res = 1; break; }
			}
		}
		if(res == 0)
		{
			PINDEX j = videoNum; videoNum--;
			for (;;)
			{
				PString key1 = psprintf("%u", ++j);
				PString name1 = config.GetString(VideoCodecsConfigSection, key1, "");
				if (name1.IsEmpty()) break;

				config.SetString(VideoCodecsConfigSection, psprintf("%u", j-2), name1);
			}
			config.DeleteKey(VideoCodecsConfigSection, psprintf("%u", j-1));
		}
	}

	// добавляю новые видео кодеки если их нет в конфигурации
	for (PINDEX i = 0; i < capabilities.GetSize(); i++)
	{
		if (capabilities[i].GetMainType() == H323Capability::e_Video)
		{
			PINDEX codecNum=0;
			int res = 0;
			int suffix = 0;
			for (;;)
			{
				PString key = psprintf("%u", ++codecNum);
				PString name = config.GetString(VideoCodecsConfigSection, key, "");
				if (name.IsEmpty()) break;

				suffix = 0;
				PINDEX suffixPos = name.Find(OnCodecSuffix);
				if (suffixPos != P_MAX_INDEX)
					name.Delete(suffixPos, P_MAX_INDEX);
				else
				{
					suffix = 1;
					suffixPos = name.Find(OffCodecSuffix);
					name.Delete(suffixPos, P_MAX_INDEX);
				}

				if(capabilities[i].GetFormatName() == name) { res = 1; break; }
			}
			if(res == 0)
			{
				config.SetString(VideoCodecsConfigSection,
					psprintf("%u", codecNum),
					capabilities[i].GetFormatName() + ((suffix==0)?OnCodecSuffix:OffCodecSuffix));
			}
		}
	}

	PINDEX audioNum = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++audioNum);
		PString name = config.GetString(CodecsConfigSection, key, "");
		if (name.IsEmpty()) break;

		PINDEX suffixPos = name.Find(OnCodecSuffix);
		if (suffixPos != P_MAX_INDEX)
			name.Delete(suffixPos, P_MAX_INDEX);
		else
		{
			suffixPos = name.Find(OffCodecSuffix);
			name.Delete(suffixPos, P_MAX_INDEX);
		}

		int res = 0;
		for (PINDEX i = 0; i < capabilities.GetSize(); i++)
		{
			if (capabilities[i].GetMainType() == H323Capability::e_Audio)
			{
				if(capabilities[i].GetFormatName() == name)
				{ res = 1; break; }
			}
		}
		if(res == 0)
		{
			PINDEX j = audioNum; audioNum--;
			for (;;)
			{
				PString key1 = psprintf("%u", ++j);
				PString name1 = config.GetString(CodecsConfigSection, key1, "");
				if (name1.IsEmpty()) break;

				config.SetString(CodecsConfigSection, psprintf("%u", j-2), name1);
			}
			config.DeleteKey(CodecsConfigSection, psprintf("%u", j-1));
		}
	}

	for (PINDEX i = 0; i < capabilities.GetSize(); i++)
	{
		if (capabilities[i].GetMainType() == H323Capability::e_Audio)
		{
			PINDEX codecNum=0;
			int res = 0;
			int suffix = 0;
			for (;;)
			{
				PString key = psprintf("%u", ++codecNum);
				PString name = config.GetString(CodecsConfigSection, key, "");
				if (name.IsEmpty()) break;

				suffix = 0;
				PINDEX suffixPos = name.Find(OnCodecSuffix);
				if (suffixPos != P_MAX_INDEX)
					name.Delete(suffixPos, P_MAX_INDEX);
				else
				{
					suffix = 1;
					suffixPos = name.Find(OffCodecSuffix);
					name.Delete(suffixPos, P_MAX_INDEX);
				}

				if(capabilities[i].GetFormatName() == name) { res = 1; break; }
			}
			if(res == 0)
			{
				config.SetString(CodecsConfigSection,
					psprintf("%u", codecNum),
					capabilities[i].GetFormatName() + ((suffix==0)?OnCodecSuffix:OffCodecSuffix));
			}
		}
	}

	// Add all the UserInput capabilities
	AddAllUserInputCapabilities(0, 1);

	int videoSizeRx = config.GetInteger(VideoInSizeConfigKey, 2);
	int videoSizeTx = config.GetInteger(VideoOutSizeConfigKey, 2);

	RemoveCapability(H323Capability::e_ExtendVideo);

	autoStartTransmitVideo = config.GetBoolean(AutoTransmitVideoConfigKey, TRUE);
	autoStartReceiveVideo = config.GetBoolean(AutoReceiveVideoConfigKey, TRUE);

	localVideo = config.GetBoolean(VideoLocalConfigKey, FALSE);
	localFlip = config.GetBoolean(VideoFlipLocalConfigKey, FALSE);

	int videoInMaxBitRate = config.GetInteger(VideoInMaxbandWidthKey, 320);
	videoInMaxBitRate = 1024 * PMAX(16, PMIN(10240, videoInMaxBitRate));

	// changing audio codecs
	PStringArray enabledCodecs;
	PINDEX codecNum = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++codecNum);
		PString name = config.GetString(CodecsConfigSection, key, "");
		if (name.IsEmpty()) break;

		PINDEX suffixPos = name.Find(OffCodecSuffix);
		if (suffixPos != P_MAX_INDEX)
		{
			capabilities.Remove(name(0, suffixPos-1));
			continue;
		}

		suffixPos = name.Find(OnCodecSuffix);
		if (suffixPos != P_MAX_INDEX)	name.Delete(suffixPos, P_MAX_INDEX);
		enabledCodecs.AppendString(name);
	}

	codecNum = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++codecNum);
		PString name = config.GetString(VideoCodecsConfigSection, key, "");
		if (name.IsEmpty()) break;
	}

	//tvCaps = (char **)calloc(codecNum+1,sizeof(char *));

	int tvNum = 0;
	codecNum = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++codecNum);
		PString name = config.GetString(VideoCodecsConfigSection, key, "");
		if (name.IsEmpty()) break;

		// удаление отключенных кодеков
		PINDEX suffixPos = name.Find(OffCodecSuffix);
		if (suffixPos != P_MAX_INDEX)
		{
			capabilities.Remove(name(0, suffixPos-1)); continue;
		}
		// удаление суффикса on из имени кодека
		suffixPos = name.Find(OnCodecSuffix);
		if (suffixPos != P_MAX_INDEX)
			name.Delete(suffixPos, P_MAX_INDEX);

		// проверка кодека на соответствие размеру принимаемой картинки
		// (меньше можно, больше нельзя) и удаление из списка локальных кодеков
		suffixPos = P_MAX_INDEX;
		switch(videoSizeRx)
		{
		case 0: //QCIF
			suffixPos = name.Find("-CIF");   if(suffixPos != P_MAX_INDEX) break;
		case 1: //QVGA
		case 2: //CIF
			suffixPos = name.Find("-4CIF");  if(suffixPos != P_MAX_INDEX) break;
		case 3: //VGA
		case 4: //4CIF
			suffixPos = name.Find("-SD");    if(suffixPos != P_MAX_INDEX) break;
		case 5: //SVGA
		case 6: //XVGA
			suffixPos = name.Find("-HD");    if(suffixPos != P_MAX_INDEX) break;
			suffixPos = name.Find("-16CIF"); if(suffixPos != P_MAX_INDEX) break;
		case 7: //SXGA
			suffixPos = name.Find("-FHD"); if(suffixPos != P_MAX_INDEX) break;
		case 8: //16CIF
		default:
			break;
		}
		if(suffixPos == P_MAX_INDEX)
			enabledCodecs.AppendString(name);
		else
			capabilities.Remove(name);

		// проверка кодека на соответствие размеру отправляемой картинки
		// (меньше можно, больше нельзя) и создание списка допустимых кодеков
		suffixPos = P_MAX_INDEX;
		switch(videoSizeTx)
		{
		case 0: //QCIF
			suffixPos = name.Find("-CIF");   if(suffixPos != P_MAX_INDEX) break;
		case 1: //QVGA
		case 2: //CIF
			suffixPos = name.Find("-4CIF");  if(suffixPos != P_MAX_INDEX) break;
		case 3: //VGA
		case 4: //4CIF
			suffixPos = name.Find("-SD");    if(suffixPos != P_MAX_INDEX) break;
		case 5: //SVGA
		case 6: //XVGA
			suffixPos = name.Find("-HD");    if(suffixPos != P_MAX_INDEX) break;
		case 7: //SXGA
			suffixPos = name.Find("-16CIF"); if(suffixPos != P_MAX_INDEX) break;
			suffixPos = name.Find("-FHD"); if(suffixPos != P_MAX_INDEX) break;
		case 8: //16CIF
		default:
			break;
		}
		if(suffixPos == P_MAX_INDEX) // добавляю в список допустимых
		{
			const char *p2pstr=name;
			//tvCaps[tvNum]=strdup(p2pstr);
			tvNum++;
		}
	}

	// Reorder the codecs we have
	capabilities.Reorder(enabledCodecs);

	for (PINDEX i = 0; i < capabilities.GetSize(); i++)
	{
		if (capabilities[i].GetMainType() == H323Capability::e_Video)
		{
			// это влияет на качество получаемой картинки!!!
			capabilities[i].SetMediaFormatOptionInteger(OpalVideoFormat::MaxBitRateOption, videoInMaxBitRate);
			printf("capabilities[i].SetMediaFormatOptionInteger=%d\n", videoInMaxBitRate);
			if(capabilities[i].GetFormatName().Find("H.264")!=P_MAX_INDEX)
			{

				H323GenericVideoCapability *h264cap = (H323GenericVideoCapability *) &capabilities[i];
				h264cap->SetMaxBitRate(videoInMaxBitRate);
				printf("h264cap->SetMaxBitRate=%d\n", videoInMaxBitRate);
			}
		}
	}


	PTRACE(1, "QtPhone\tCapability Table:\n" << setprecision(4) << capabilities);
}
/*
void H323Capability::SetMediaFormatOptionInteger(const PString &name, int val)
{
	mediaFormat.SetOptionInteger(name, val);
}
void H323GenericCapabilityInfo::SetMaxBitRate(unsigned bitrate)
{
	maxBitRate = bitrate;
}
*/
bool CMyPhoneEndPoint::FindGatekeeper()
{
	QString msg;
	if (GetGatekeeper() != NULL)
	{
		if (gatekeeper->IsRegistered()) // If registered, then unregister
		{
			msg = QString("Покидаем Гейткипер: %1").arg((const char *)gatekeeper->GetName());
			emit signal_OutputMsg(msg);
		}
		RemoveGatekeeper();
	}

	if (!config.GetBoolean(UseGatekeeperConfigKey, FALSE))
		return TRUE;

	SetGatekeeperPassword(config.GetString(GatekeeperPassConfigKey));

	PString gkHost = config.GetString(GatekeeperHostConfigKey, "");
	PString gkid = config.GetString(GatekeeperIdConfigKey, "");
	PString iface = "";
	int iMode = config.GetInteger(DiscoverGatekeeperConfigKey, -1);
	switch(iMode)
	{
	case -1:
		return TRUE;
		break;
	case 0:
		gkHost = "";
		gkid = "";
		iface = "";
		break;
	case 1:
		gkid = "";
		iface = "";
		break;
	case 2:
		gkHost = "";
		iface = "";
		break;
	}

	msg = QString("Ищем Гейткипер.... Пожалуйста, подождите!");
	emit signal_OutputMsg(msg);
	if (UseGatekeeper(gkHost, gkid, iface))
	{
		msg = QString("Успешно зарегистрировались на Гейткипере: %1").arg((const char *)gatekeeper->GetName());
		emit signal_OutputMsg(msg);
	}

	bool gkRequired = config.GetBoolean(RequireGatekeeperConfigKey, FALSE);

	msg = "";
	if (GetGatekeeper() != NULL)
	{
		unsigned reason = gatekeeper->GetRegistrationFailReason();
		switch (reason)
		{
		case H323Gatekeeper::InvalidListener :
			msg = QString("ОШИБКА! Гейткипер отказал в слушающем порте.");
			break;
		case H323Gatekeeper::DuplicateAlias :
			msg = QString("ОШИБКА! Ваш псевдоним уже зарегистрирован на Гейткипере.");
			break;
		case H323Gatekeeper::SecurityDenied :
			msg = QString("ОШИБКА! Гейткипер отказал из соображений безопасности, проверьте пароль.");
			break;
		case H323Gatekeeper::TransportError :
			msg = QString("ОШИБКА! Ошибка соединения с Гейткипером.");
			break;
		default :
			if ((reason&H323Gatekeeper::RegistrationRejectReasonMask) != 0)
			{
				msg = QString("Гейткипер отказал в регистрации с кодом %1.")
					.arg((int) reason&(H323Gatekeeper::RegistrationRejectReasonMask-1));
			}
			break;
		}
	}
	else
	{
		if (!gkHost.IsEmpty())
		{
			if (!gkid.IsEmpty())
				msg = QString("ОШИБКА! Не могу найти Гейткипер с ID %1 на %2.")
					.arg((const char *)gkid)
					.arg((const char *)gkHost);
			else
				msg = QString("ОШИБКА! Не могу найти Гейткипер на %2.")
					.arg((const char *)gkHost);
		}
		else
		{
			if (!gkid.IsEmpty())
				msg = QString("ОШИБКА! Не могу найти Гейткипер с ID %1 в сети.")
					.arg((const char *)gkid);
			else
			{
				if (gkRequired)
					msg = QString("ОШИБКА! Не могу найти Гейткипер в сети.");
				else
					msg = QString("Гейткипер не найден.");
			}
		}
	}

	if(!msg.isEmpty())
		emit signal_OutputMsg(msg);

	return !gkRequired;
}

void CMyPhoneEndPoint::OutputMsg(const QString &txt)
{
	emit signal_OutputMsg(txt);
}

void CMyPhoneEndPoint::OutputUsrMsg(const QString &txt)
{
	emit signal_OutputUsrMsg(txt);
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
// Derived Connection
CMyPhoneConnection::CMyPhoneConnection(
		   QtPhoneDlg* wnd, CMyPhoneEndPoint & ep, unsigned callReference, unsigned options)
		   : H323Connection(ep, callReference, options), endpoint(ep), m_dialog(wnd)
{
}

PBoolean CMyPhoneConnection::OnAlerting(const H323SignalPDU &, const PString & user)
{
	QString msg = QString("Вызываем %1...").arg(m_dialog->FindContactName(*this));
	endpoint.OutputMsg(msg);
	return TRUE;
}

void CMyPhoneConnection::OnUserInputString(const PString & value)
{
	QString msg = QString("<-%1: ""%2""").arg(m_dialog->FindContactName(*this)).arg((const char*)value);
	endpoint.OutputUsrMsg(msg);
}
/*
void CMyPhoneConnection::SelectDefaultLogicalChannel(
      unsigned sessionID    ///< Session ID to find default logical channel.
    )
{
	puts("CMyPhoneConnection::SelectDefaultLogicalChannel");
	if (FindChannel (sessionID, FALSE))
		return;

	if(sessionID == RTP_Session::DefaultVideoSessionID)
	{
		if(endpoint.tvCaps==NULL)
		{
			for (PINDEX i = 0; i < remoteCapabilities.GetSize(); i++)
			{
				H323Capability & remoteCapability = remoteCapabilities[i];
				if (remoteCapability.GetDefaultSessionID() == sessionID)
				{
//					if(remoteCapabilities.FindCapability(remoteCapability,sessionID)==TRUE)
					{
//						PTRACE(2, "H245\tOpenLogicalChannel " << remoteCapability);
//						OpenLogicalChannel(remoteCapability, sessionID, H323Channel::IsTransmitter);
//						return;
					}
				}
			}
		}
		else
		{
			H323Capability * remoteCapability = remoteCapabilities.SelectRemoteCapabilty(sessionID,endpoint.tvCaps);
			if(remoteCapability==NULL)
				return;
			PTRACE(2, "H245\tOpenLogicalChannel " << *remoteCapability);
			OpenLogicalChannel(*remoteCapability, sessionID, H323Channel::IsTransmitter);
			return;
		}
		return;
	}

	H323Connection::SelectDefaultLogicalChannel(sessionID);
}
*/
//////////////////////////////////////////////////////////////////////
