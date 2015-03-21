
#ifndef inifile_h
#define inifile_h

typedef struct {
  bool DetailsDebugLog;
  bool UseGBACartForSwapMemory;
  bool VRAMCacheEnabled;
  bool ARM7_DebugLogFlag;
  bool ChildrenMode;
} TiniSystem;

typedef struct {
  bool SlowDiskAccess;
  bool CheckDiskType1;
  bool Ignore16bitReadTest;
  bool AutoDetect1632bitsReadAccessMode;
  bool AlwaysDisabledCheckDisk;
} TiniDiskAdapter;

typedef struct {
    u32 DelayCount;
    u32 RateCount;
} TiniKeyRepeat;

typedef struct {
    bool IgnoreComplexDecoderFlag;
    bool IgnorePlayMode_AlwaysOne_FromUserRequest;
} TiniPlayList;

typedef struct {
    bool CarSupplyMode;
    bool SwapTopBottomDisplay;
    u32 PowerOffTimerWhileNoInput;
    bool WhenMusicShowMP3Cnt;
} TiniFileList;

enum EGMEPluginSimpleLPF {EGMESimpleLPF_None=0,EGMESimpleLPF_Lite=1,EGMESimpleLPF_Heavy=2};

typedef struct {
	u32 ReverbLevel;
	EGMEPluginSimpleLPF SimpleLPF;
	u32 DefaultLengthSec;
	bool NSF_EnabledMultiTrack;
	bool GBS_EnabledMultiTrack;
	bool AY_EnabledMultiTrack;
	bool KSS_EnabledMultiTrack;
	u32 HES_MaxTrackNumber;
} TiniGMEPlugin;

typedef struct {
    bool ShowEventMessage;
    u32 MaxVoiceCount;
    u32 GenVolume;
    u32 ReverbFactor_ToneMap;
    u32 ReverbFactor_DrumMap;
    bool ShowInfomationMessages;
} TiniMIDPlugin;

typedef struct {
    bool AlwayUseTextEditor;
} TiniTextPlugin;

typedef struct {
  TiniSystem System;
  TiniDiskAdapter DiskAdapter;
  TiniKeyRepeat KeyRepeat;
  TiniPlayList PlayList;
  TiniFileList FileList;
  TiniGMEPlugin GMEPlugin;
  TiniMIDPlugin MIDPlugin;
  TiniTextPlugin TextPlugin;
} TGlobalINI;

extern TGlobalINI GlobalINI;

extern void LoadINI(void);

#endif


