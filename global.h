#ifndef GLOBAL_H
#define GLOBAL_H

#include <ptlib.h>
#include <ptlib/pprocess.h>

#if PTLIB_MAJOR<=2 && PTLIB_MINOR<10
	#define PBoolean		BOOL
#endif

//#include <h323pluginmgr.h>

const char strProgName[]="QtPhone";
const int progMajorVersion = 0;
const int progMinorVersion = 1;
#define MF_RELEASE	//*** Uncomment this when go out of "beta" versioning ***
const int progBuildVersion = 0;
const char strProgPostfix[]="";
const char strProgAuthor[]="H323plus project (support@h323plus.org)";
const char strProgWeb[]="http://h323plus.org/";
const char strProgWebHelp[]="http://h323plus.org/myphone.html";
//
const char strProgRespects[]="\t~~~ Thanks goes to: ~~~\r\n\
							 * All OpenH323/H323plus developers amd mailing list members\r\n\
							 * Franz J Ehrengruber (for testing and hardware support)\r\n\
							 * Guilhem Tardy (for help with the initial H.263 video codec)\r\n\
							 * Simon Horne (for incorporating in h323plus project)\r\n";


// Configuration Keys
const char AddressBookSection[] = "AddressBook";
const char AddressBookRescentKey[] = "RescentEntries";
const char RingSoundFileConfigKey[] = "RingSoundFile";
const char IpTosConfigKey[] = "IpTOS";
const char BandwidthTypeConfigKey[] = "BandwidthType";
const char BandwidthConfigKey[] = "Bandwidth";
const char ListenerInterfaceConfigKey[] = "ListenerInterface";
const char RouterConfigKey[] = "NATRouterAddress";
const char RTPPortBaseConfigKey[] = "RTPPortBase";
const char RTPPortMaxConfigKey[] = "RTPPortMax";
const char NoTunnelingConfigKey[] = "NoTunneling";
const char DtmfAsStringConfigKey[] = "DtmfAsString";
const char AutoAnswerConfigKey[] = "AutoAnswer";
const char UsernameConfigKey[] = "Username";
const char AliasConfigKey[] = "UserAliases";
const char UseGatekeeperConfigKey[] = "UseGatekeeper";
const char GatekeeperPassConfigKey[] = "GatekeeperPassword";
const char RequireGatekeeperConfigKey[] = "RequireGatekeeper";
const char GatekeeperHostConfigKey[] = "GatekeeperHost";
const char GatewayHostConfigKey[] = "GatewayHost";
const char DiscoverGatekeeperConfigKey[] = "DiscoverGatekeeper";
const char DiscoverGatewayConfigKey[] = "DiscoverGateway";
const char SoundPlayConfigKey[] = "SoundPlayDevice";
const char SoundRecordConfigKey[] = "SoundRecordDevice";
const char SilenceDetectConfigKey[] = "SilenceDetect";
const char JitterConfigKey[] = "Jitter";
const char CodecsConfigSection[] = "Codecs";
const char VideoCodecsConfigSection[] = "VideoCodecs";
const char NoFastStartConfigKey[] = "FastStart";
const char BufferCountConfigKey[] = "BufferCount";
const char AutoReceiveVideoConfigKey[] = "AutoReceiveVideo";
const char AutoTransmitVideoConfigKey[] = "AutoTransmitVideo";
const char VideoDeviceConfigKey[] = "VideoDevice";
const char VideoInSizeConfigKey[] = "VideoInSize";
const char VideoOutSizeConfigKey[] = "VideoOutSize";
const char VideoSourceConfigKey[] = "VideoSource";
const char VideoFormatConfigKey[] = "VideoFormat";
const char VideoQualityConfigKey[] = "VideoQuality";
const char VideoLocalConfigKey[] = "VideoLocal";
const char VideoFlipLocalConfigKey[] = "VideoFlipLocal";
const char VideoFPSKey[] = "VideoFramesPerSecond";
const char VideoInMaxbandWidthKey[] = "VideoInMaxbandwidth";
const char VideoOutMaxbandWidthKey[] = "VideoOutMaxbandwidth";
const char VideoPacketSizeConfigKey[] = "VideoPacketSize";
const char VideoInVFlipConfigKey[] = "VideoInVFlip";
const char VideoInHFlipConfigKey[] = "VideoInHFlip";
const char VideoOutVFlipConfigKey[] = "VideoOutVFlip";
const char VideoOutHFlipConfigKey[] = "VideoOutHFlip";
const char OutputVolumeConfigKey[] = "OutputVolume";
const char ColorInMessageConfigKey[] = "ColorInMessage";
const char ColorOutMessageConfigKey[] = "ColorOutMessage";
const char ColorSignalIndicatorConfigKey[] = "ColorActInd";
const char ColorSilentIndicatorConfigKey[] = "ColorSlntInd";
const char AutoVideoHideConfigKey[] = "AutoVidPanHide";
const char HideStatConfigKey[] = "HideStatistic";
const char SysLogMsgHideConfigKey[] = "HideSysLogMessages";
const char AutoAddCallersConfigKey[] = "AutoAddCallerAddr";
const char UserInputModeConfigKey[] = "UserInputMode";
const char OnCodecSuffix[] = " (On)";
const char OffCodecSuffix[] = " (Off)";
const char TimeToLiveConfigKey[] = "TimeToLive";
const char GatekeeperIdConfigKey[] = "GatekeeperId";

/*
class PluginLoaderStartup2 : public PProcessStartup
{
  PCLASSINFO(PluginLoaderStartup2, PProcessStartup);
  public:
    void OnStartup()
    { 
      // load the actual DLLs, which will also load the system plugins
      PStringArray dirs = PPluginManager::GetPluginDirs();
      PPluginManager & mgr = PPluginManager::GetPluginManager();
      PINDEX i;
      for (i = 0; i < dirs.GetSize(); i++) 
        mgr.LoadPluginDirectory(dirs[i]);

      // now load the plugin module managers
      PFactory<PPluginModuleManager>::KeyList_T keyList = PFactory<PPluginModuleManager>::GetKeyList();
      PFactory<PPluginModuleManager>::KeyList_T::const_iterator r;
      for (r = keyList.begin(); r != keyList.end(); ++r) {
        PPluginModuleManager * mgr = PFactory<PPluginModuleManager>::CreateInstance(*r);
        if (mgr == NULL) {
          PTRACE(1, "PLUGIN\tCannot create manager for plugins of type " << *r);
        } else {
          PTRACE(3, "PLUGIN\tCreated manager for plugins of type " << *r);
          managers.push_back(mgr);
        }
      }
    }

    void OnShutdown()
    {
      while (managers.begin() != managers.end()) {
        std::vector<PPluginModuleManager *>::iterator r = managers.begin();
        PPluginModuleManager * mgr = *r;
        managers.erase(r);
        mgr->OnShutdown();
      }
    }

  protected:
    std::vector<PPluginModuleManager *> managers;
};
/*

class PWLibProcess : public PProcess 
{
	PCLASSINFO(PWLibProcess, PProcess);
public:
	PWLibProcess():PProcess(strProgName, strProgName, progMajorVersion, progMinorVersion, ReleaseCode, progBuildVersion) { }
	void Main() { }

	// This is to get the plugins to load in MFC applications
	static void LoadPluginMgr() { plugmgr = new H323PluginCodecManager(); }
	static void RemovePluginMgr() { delete plugmgr; }
	static H323PluginCodecManager * plugmgr;
	//static PluginLoaderStartup2 pluginLoader;
};
*/

#endif // GLOBAL_H
