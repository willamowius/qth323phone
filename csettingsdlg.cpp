#include <QFileDialog>
#include "csettingsdlg.h"
#include "global.h"

#include <ptlib/sound.h>
#include <ptlib/video.h>

const char *StandartSizes[10] = {
	"QCIF (176 x 144)",
	"QVGA (320 x 240)",
	"CIF (352 x 288)",
	"VGA (640 x 480)",
	"4CIF (704 x 576)",
	"SVGA (800 x 600)",
	"XVGA (1024 x 768)",
	"SXGA (1280 x 1024)",
	"16CIF (1408 x 1152)",
	"UXGA (1600 x 1200)"};

#define COUNT_STANDART_SIZE	10

CSettingsDlg::CSettingsDlg(CMyPhoneEndPoint *m_endpoint, QWidget *parent)
	: QDialog(parent)
{
	p_endpoint = m_endpoint;

	ui.setupUi(this);

	readIni();

	connect(ui.IDS_DEVSTR, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_ComboBoxChange(int)));
	connect(ui.IDS_INDEVSTR, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_ComboBoxChange(int)));
	connect(ui.IDS_OUTDEVSTR, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_ComboBoxChange(int)));
	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(slot_ApplyChange()));
	connect(ui.IDS_VCSUPSTR, SIGNAL(clicked()), this, SLOT(slot_VCodecUp()));
	connect(ui.IDS_VCSDOWNSTR, SIGNAL(clicked()), this, SLOT(slot_VCodecDown()));
	connect(ui.IDS_CSUPSTR, SIGNAL(clicked()), this, SLOT(slot_ACodecUp()));
	connect(ui.IDS_CSDOWNSTR, SIGNAL(clicked()), this, SLOT(slot_ACodecDown()));
	connect(ui.IDS_VCSENBLSTR, SIGNAL(clicked(bool)), this, SLOT(slot_VCodecUse(bool)));
	connect(ui.IDS_CSENBLSTR, SIGNAL(clicked(bool)), this, SLOT(slot_ACodecUse(bool)));
	connect(ui.IDS_VCADECSSTR, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slot_VItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.IDS_CADECSSTR, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(slot_AItemChanged(QListWidgetItem*, QListWidgetItem*)));
	connect(ui.IDS_RINGFCMD, SIGNAL(clicked()), this, SLOT(slot_GetFile()));
	connect(ui.IDS_RINGFCMDP, SIGNAL(clicked()), this, SLOT(slot_PlayFile()));
}

CSettingsDlg::~CSettingsDlg()
{

}

void CSettingsDlg::readIni()
{
	int i;
	QString t_str;
	PConfig& config = p_endpoint->config;

	//////////////////////////////////// General /////////////////////////////////////////////

	ui.IDS_UNAMESTR->setText(QString((const char*)p_endpoint->GetLocalUserName()));
	ui.IDS_RINGFSTR->setText(QString((const char*)config.GetString(RingSoundFileConfigKey, "call.wav")));

	ui.IDS_AANSWERSTR->setChecked(p_endpoint->m_fAutoAnswer);
	ui.IDS_DTMFSTRSTR->setChecked(p_endpoint->m_fDtmfAsString);
	ui.IDS_D242STR->setChecked(p_endpoint->DisableH245Tunnelling());
	ui.IDS_DFSTARTSTR->setChecked(p_endpoint->m_fNoFastStart);

	//////////////////////////////////// Net /////////////////////////////////////////////

	ui.IDS_BANDSTR->setCurrentIndex(config.GetInteger(BandwidthTypeConfigKey, 5));
	ui.IDS_KBSSTR->setText(QString::number(p_endpoint->GetInitialBandwidth()/20));
	ui.IDS_IPTSTR->setText(QString::number((int)p_endpoint->GetRtpIpTypeofService(),16));
	ui.IDS_RTPBSTR->setValue(config.GetInteger(RTPPortBaseConfigKey, p_endpoint->GetRtpIpPortBase()));
	ui.IDS_RTPMSTR->setValue(config.GetInteger(RTPPortMaxConfigKey, p_endpoint->GetRtpIpPortBase()));
	ui.IDS_NATRSTR->setText((config.HasKey(RouterConfigKey))?
		(const char*)config.GetString(RouterConfigKey, p_endpoint->m_router.AsString()):
		"");

	PString lifaces = config.GetString(ListenerInterfaceConfigKey, "*");
	lifaces = lifaces.IsEmpty()?"*":lifaces;
	ui.IDS_LOCISTR->setText(QString((const char*)lifaces));

	//////////////////////////////////// Gatekeeper /////////////////////////////////////////////

	ui.IDS_GKUSESTR->setChecked(config.GetBoolean(UseGatekeeperConfigKey, FALSE));
	ui.IDS_GKRECSTR->setChecked(config.GetBoolean(RequireGatekeeperConfigKey, FALSE));
	QString gkhost = QString((const char*)config.GetString(GatekeeperHostConfigKey, ""));
	ui.IDS_GKHOSTLOCSTR_STR->setText(gkhost);
	ui.IDS_GKPASSSTR->setText(QString((const char*)config.GetString(GatekeeperPassConfigKey, "")));

	if(gkhost.isEmpty())
		ui.IDS_GKAUTOLOCSTR->setChecked(true);

	const PStringList aliases = p_endpoint->GetAliasNames();
	for (i = 1; i < aliases.GetSize(); i++)
		ui.IDS_UALIASESSTR->addItem(QString((const char*)(aliases[i])));

	//////////////////////////////////// Audio /////////////////////////////////////////////
	PStringArray playDevices = PSoundChannel::GetDeviceNames(PSoundChannel::Player);
	for (i = 0; i < playDevices.GetSize(); i++)
		ui.IDS_OUTDEVSTR->addItem(QString((const char*)playDevices[i]));

	t_str = QString((const char*)config.GetString(SoundPlayConfigKey, ""));
	if(!t_str.isEmpty())
		ui.IDS_OUTDEVSTR->setCurrentIndex(ui.IDS_OUTDEVSTR->findText(t_str));

	PStringArray recordDevices = PSoundChannel::GetDeviceNames(PSoundChannel::Recorder);
	for (i = 0; i < recordDevices.GetSize(); i++)
		ui.IDS_INDEVSTR->addItem(QString((const char*)recordDevices[i]));

	t_str = QString((const char*)config.GetString(SoundRecordConfigKey, ""));
	if(!t_str.isEmpty())
		ui.IDS_INDEVSTR->setCurrentIndex(ui.IDS_INDEVSTR->findText(t_str));

	ui.IDS_SLSTR->setChecked(p_endpoint->m_fSilenceOn);

	i = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++i);
		PString name = config.GetString(CodecsConfigSection, key, "");
		if (name.IsEmpty())
			break;
		ui.IDS_CADECSSTR->addItem(QString((const char*)name));
	}

	ui.IDS_JITSTR->setValue(p_endpoint->GetMaxAudioJitterDelay());
	ui.IDS_BUFFSTR->setValue(p_endpoint->GetSoundChannelBufferDepth());

	//////////////////////////////////// video /////////////////////////////////////////////

	PStringArray vdevices = PVideoInputDevice::GetDriversDeviceNames(CMyPhoneEndPoint::VideoInputDriver);
	for (i = 0; i < vdevices.GetSize(); i++)
	{
		//PString *pstr = (PString *)vdevices.GetAt(i);
		ui.IDS_DEVSTR->addItem(QString::fromUtf8((const char*)vdevices[i]));
	}

	PStringArray fakedevice = PVideoInputDevice::GetDriversDeviceNames("FakeVideo");
	for (i = 0; i < fakedevice.GetSize(); i++)
		ui.IDS_DEVSTR->addItem(QString((const char*)fakedevice[i]));

	t_str = QString((const char*)config.GetString(VideoDeviceConfigKey, "")); //YK: Should really be search in list
	if(!t_str.isEmpty())
		ui.IDS_DEVSTR->setCurrentIndex(ui.IDS_DEVSTR->findText(t_str));

	i = 0;
	for (;;)
	{
		PString key = psprintf("%u", ++i);
		PString name = config.GetString(VideoCodecsConfigSection, key, "");
		if (name.IsEmpty())
			break;
		ui.IDS_VCADECSSTR->addItem(QString((const char*)name));
	}

	for(i=0;i<COUNT_STANDART_SIZE;i++)
	{
		ui.IDC_VSIZE->addItem(StandartSizes[i]);
		ui.IDC_VSIZE2->addItem(StandartSizes[i]);
	}

	ui.IDS_VSENDSTR->setChecked(config.GetBoolean(AutoTransmitVideoConfigKey, TRUE));
	ui.IDS_VRCVDSTR->setChecked(config.GetBoolean(AutoReceiveVideoConfigKey, TRUE));
	ui.IDS_VLOCSTR->setChecked(config.GetBoolean(VideoLocalConfigKey, TRUE));

	ui.IDS_LVFLIPSTR->setChecked(config.GetBoolean(VideoOutVFlipConfigKey, TRUE));
	ui.IDS_RVFLIPSTR->setChecked(config.GetBoolean(VideoInVFlipConfigKey, TRUE));
	//videoPage.m_localFlip = config.GetBoolean(VideoFlipLocalConfigKey, FALSE);

	ui.IDS_QTYSTR->setValue(config.GetInteger(VideoQualityConfigKey, 15));
	ui.IDC_FPSSTATIC->setValue(config.GetInteger(VideoFPSKey, 10));

	ui.IDC_BPSSTATIC->setValue(config.GetInteger(VideoInMaxbandWidthKey, 320));
	ui.IDC_BPSSTATIC2->setValue(config.GetInteger(VideoOutMaxbandWidthKey, 320));

	//videoPage.m_RecDevSrc = config.GetInteger(VideoSourceConfigKey, -1)+1;

	ui.IDC_VSIZE->setCurrentIndex(config.GetInteger(VideoInSizeConfigKey, 2));
	ui.IDC_VSIZE2->setCurrentIndex(config.GetInteger(VideoOutSizeConfigKey, 2));
}


void CSettingsDlg::saveIni()
{
	int i, n, val;
	QString t_str;
	PConfig& config = p_endpoint->config;

	p_endpoint->m_fAutoAnswer = ui.IDS_AANSWERSTR->isChecked();
	config.SetBoolean(AutoAnswerConfigKey, p_endpoint->m_fAutoAnswer);

	config.SetString(RingSoundFileConfigKey, ui.IDS_RINGFSTR->text().toAscii().data());

	p_endpoint->m_fSilenceOn = ui.IDS_SLSTR->isChecked();
	p_endpoint->SetSilenceDetectionMode(p_endpoint->m_fSilenceOn ?
		H323AudioCodec::AdaptiveSilenceDetection :
		H323AudioCodec::NoSilenceDetection);
	config.SetBoolean(SilenceDetectConfigKey, p_endpoint->m_fSilenceOn);

	val = ui.IDS_JITSTR->value();
	p_endpoint->SetAudioJitterDelay(val, val);
	config.SetInteger(JitterConfigKey, val);

	val = ui.IDS_BUFFSTR->value();
	p_endpoint->SetSoundChannelBufferDepth(val);
	config.SetInteger(BufferCountConfigKey, val);

	// refreshing Audio Codecs
	PString defaultSection = config.GetDefaultSection();
	config.SetDefaultSection(CodecsConfigSection);
	config.DeleteSection();
	n = ui.IDS_CADECSSTR->count();
	for (i = 0; i < n; i++)
		config.SetString(psprintf("%u", i+1), PString(ui.IDS_CADECSSTR->item(i)->text().toAscii().data()));
	config.SetDefaultSection(defaultSection);

	// Codecs in separate section
	defaultSection = config.GetDefaultSection();
	config.SetDefaultSection(VideoCodecsConfigSection);
	config.DeleteSection();
	n = ui.IDS_VCADECSSTR->count();
	for (i = 0; i < n; i++)
		config.SetString(psprintf("%u", i+1), PString(ui.IDS_VCADECSSTR->item(i)->text().toAscii().data()));
	config.SetDefaultSection(defaultSection);

	config.SetBoolean(VideoOutVFlipConfigKey, ui.IDS_LVFLIPSTR->isChecked());
	config.SetBoolean(VideoInVFlipConfigKey, ui.IDS_RVFLIPSTR->isChecked());

	config.SetInteger(VideoInMaxbandWidthKey, ui.IDC_BPSSTATIC->value());
	config.SetInteger(VideoOutMaxbandWidthKey, ui.IDC_BPSSTATIC2->value());

	config.SetInteger(VideoQualityConfigKey, ui.IDS_QTYSTR->value());
	config.SetInteger(VideoFPSKey, ui.IDC_FPSSTATIC->value());

	config.SetInteger(VideoInSizeConfigKey, ui.IDC_VSIZE->currentIndex());
	config.SetInteger(VideoOutSizeConfigKey, ui.IDC_VSIZE2->currentIndex());
}

void CSettingsDlg::slot_ApplyChange()
{
	saveIni();
	// Reload Endpoint capabilities
	p_endpoint->LoadCapabilities();
}

void CSettingsDlg::slot_ComboBoxChange(int state)
{
	QString t_str;
	PConfig& config = p_endpoint->config;

	if(sender()==ui.IDS_DEVSTR)
	{
		t_str = ui.IDS_DEVSTR->currentText();
		if(!t_str.isEmpty())
		{
			printf("VideoDeviceConfigKey - %s\n", t_str.toAscii().data());
			config.SetString(VideoDeviceConfigKey, t_str.toAscii().data());
		}
	}
	if(sender()==ui.IDS_INDEVSTR)
	{
		t_str = ui.IDS_INDEVSTR->currentText();
		if(!t_str.isEmpty())
		{
			printf("SetSoundChannelRecordDevice - %s\n", t_str.toAscii().data());
			config.SetString(SoundRecordConfigKey, t_str.toAscii().data());
			//p_endpoint->SetSoundChannelRecordDevice(t_str.toAscii().data());
		}
	}
	if(sender()==ui.IDS_OUTDEVSTR)
	{
		t_str = ui.IDS_OUTDEVSTR->currentText();
		if(!t_str.isEmpty())
		{
			printf("SetSoundChannelPlayDevice - %s\n", t_str.toAscii().data());
			config.SetString(SoundPlayConfigKey, t_str.toAscii().data());
			//p_endpoint->SetSoundChannelPlayDevice(t_str.toAscii().data());
		}
	}
}

void CSettingsDlg::slot_VCodecUp()
{
	QListWidgetItem *item = ui.IDS_VCADECSSTR->currentItem();
	if(!item)
		return;
	int indx = ui.IDS_VCADECSSTR->currentRow();
	if(indx==0)
		return;
	ui.IDS_VCADECSSTR->takeItem(indx);
	ui.IDS_VCADECSSTR->insertItem(indx-1, item);
	ui.IDS_VCADECSSTR->setCurrentRow(indx-1);
}

void CSettingsDlg::slot_VCodecDown()
{
	QListWidgetItem *item = ui.IDS_VCADECSSTR->currentItem();
	if(!item)
		return;
	int indx = ui.IDS_VCADECSSTR->currentRow();
	if(indx==ui.IDS_VCADECSSTR->count()-1)
		return;
	ui.IDS_VCADECSSTR->takeItem(indx);
	ui.IDS_VCADECSSTR->insertItem(indx+1, item);
	ui.IDS_VCADECSSTR->setCurrentRow(indx+1);
}

void CSettingsDlg::slot_ACodecUp()
{
	QListWidgetItem *item = ui.IDS_CADECSSTR->currentItem();
	if(!item)
		return;
	int indx = ui.IDS_CADECSSTR->currentRow();
	if(indx==0)
		return;
	ui.IDS_CADECSSTR->takeItem(indx);
	ui.IDS_CADECSSTR->insertItem(indx-1, item);
	ui.IDS_CADECSSTR->setCurrentRow(indx-1);
}

void CSettingsDlg::slot_ACodecDown()
{
	QListWidgetItem *item = ui.IDS_CADECSSTR->currentItem();
	if(!item)
		return;
	int indx = ui.IDS_CADECSSTR->currentRow();
	if(indx==ui.IDS_CADECSSTR->count()-1)
		return;
	ui.IDS_CADECSSTR->takeItem(indx);
	ui.IDS_CADECSSTR->insertItem(indx+1, item);
	ui.IDS_CADECSSTR->setCurrentRow(indx+1);
}

void CSettingsDlg::slot_VCodecUse(bool fUse)
{
	QListWidgetItem *item = ui.IDS_VCADECSSTR->currentItem();
	if(!item)
		return;
	QString value = item->text();
	if (fUse)
		value.replace(OffCodecSuffix, OnCodecSuffix);
	else
		value.replace(OnCodecSuffix, OffCodecSuffix);
	item->setText(value);
}

void CSettingsDlg::slot_ACodecUse(bool fUse)
{
	QListWidgetItem *item = ui.IDS_CADECSSTR->currentItem();
	if(!item)
		return;
	QString value = item->text();
	if (fUse)
		value.replace(OffCodecSuffix, OnCodecSuffix);
	else
		value.replace(OnCodecSuffix, OffCodecSuffix);
	item->setText(value);
}

void CSettingsDlg::slot_VItemChanged(QListWidgetItem *item, QListWidgetItem*)
{
	int indx = (item) ? ui.IDS_VCADECSSTR->currentRow() : -1;

	ui.IDS_VCSUPSTR->setEnabled(item && indx>0);
	ui.IDS_VCSENBLSTR->setEnabled(item);
	ui.IDS_VCSDOWNSTR->setEnabled(item && indx<ui.IDS_VCADECSSTR->count()-1);

	if(item)
	{
		ui.IDS_VCSENBLSTR->setChecked(item->text().indexOf(OnCodecSuffix)>0);
	}
}

void CSettingsDlg::slot_AItemChanged(QListWidgetItem *item, QListWidgetItem*)
{
	int indx = (item) ? ui.IDS_CADECSSTR->currentRow() : -1;

	ui.IDS_CSUPSTR->setEnabled(item && indx>0);
	ui.IDS_CSENBLSTR->setEnabled(item);
	ui.IDS_CSDOWNSTR->setEnabled(item && indx<ui.IDS_CADECSSTR->count()-1);

	if(item)
	{
		ui.IDS_CSENBLSTR->setChecked(item->text().indexOf(OnCodecSuffix)>0);
	}
}

void CSettingsDlg::slot_GetFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
		QObject::tr("Выбор аудио файла"),
		ui.IDS_RINGFSTR->text(),
		QObject::tr("аудио файлы (*.wav);;все файлы (*.*)")
		);

	if (fileName.isEmpty())
		return;

	ui.IDS_RINGFSTR->setText(fileName);
}

void CSettingsDlg::slot_PlayFile()
{
	QString fileName = ui.IDS_RINGFSTR->text();
	if (fileName.isEmpty())
		return;
	PSound::PlayFile(fileName.toAscii().data(), FALSE);
}
