
#ifndef procstate_h
#define procstate_h

#include "unicode.h"
#include "shell.h"

extern void ProcState_Init(void);
extern void ProcState_Free(void);
extern void ProcState_Clear(void);
extern void ProcState_Load(void);
extern void ProcState_Save(void);

enum EProcStateScreenSaver {EPSSS_Normal,EPSSS_Digital,EPSSS_Extend};
enum EProcStateScreenSaverBG {EPSSSBG_SkinDefault,EPSSSBG_TopScreen,EPSSSBG_BottomScreen};

enum ELastState {ELS_FileList,ELS_Launch};

enum ELastPageTab {ELPT_Global,ELPT_FileList,ELPT_ScreenSaver,ELPT_Music};
enum ELRKeyLockType {ELRLT_AlwayOff,ELRLT_RelationalPanel,ELRLT_AlwayOn};
typedef struct {
    ELastPageTab LastPageTab;
    bool SkipSetup;
    bool BootCheckDisk;
    bool ClickSound;
    bool EnableFadeEffect;
    bool AutoLastState;
    ELastState LastState;
    u32 AudioVolume64;
    u32 VideoVolume64;
    u32 BacklightLevel;
    UnicodeChar SkinFilenameUnicode[MaxFilenameLength];
    ELRKeyLockType LRKeyLockType;
    bool EnableResumeFunction;
    bool EnableScreenCapture;
    bool SpeakerPowerOffWhenPanelClosed;
} TProcState_System;

enum EProcStateFileListMode {EPSFLM_Single,EPSFLM_Double};

enum EProcStateFileListPlayMode {EPSFLPM_Repeat,EPSFLPM_AllRep,EPSFLPM_Shuffle};

typedef struct {
    UnicodeChar CurrentPathUnicode[MaxFilenameLength];
    UnicodeChar SelectFilenameUnicode[MaxFilenameLength];
    s32 SelectWindowTopOffset;
    EProcStateFileListMode Mode;
    EProcStateFileListPlayMode PlayMode;
    bool LRClickLongSeek;
    bool MoveFolderLocked;
    bool HiddenFilenameExt;
    bool HiddenNotSupportFileType;
    bool HiddenNDSROMFile;
    bool EnableFileInfo;
    bool ShowOnlineHelp_LRButton;
    bool BButtonToFolderUp;
    bool EasyDeleteKey;
    bool UseJpegExifThumbnail;
    bool ShowCoverImage;
    
    bool HideAttribute_Archive;
    bool HideAttribute_Hidden;
    bool HideAttribute_System;
    bool HideAttribute_ReadOnly;
} TProcState_FileList;
typedef struct {
    bool EnabledScreenSaver;
    bool Use24hFormat;
    bool ShowClock;
    bool ShowID3Tag;
    bool ShowLyric;
    bool BacklightOffTimeout;
    u32 BacklightTimeoutSec;
    bool HideScreenSaverCustom;
    bool EnabledTouchPanelCombination;
    EProcStateScreenSaver ScreenSaver;
    EProcStateScreenSaverBG ScreenSaverBG;
} TProcState_ScreenSaver;

enum EPlayListEnd {EPLE_Loop,EPLE_Stop,EPLE_Off};
typedef struct {
    EPlayListEnd PlayListEnd;
} TProcState_Music;

typedef struct {
    bool AlwaysUsedAudioPlayer;
    bool PauseWhenStopped;
    bool DrawPlayList;
    bool DrawSpectrumAnalyzer;
} TProcState_Audio;

enum EDPGPlayMode {EDPM_Repeat,EDPM_AllRep,EDPM_Random};

typedef struct {
    bool EnabledFastStart;
    EDPGPlayMode PlayMode;
    bool BacklightFlag;
    bool PauseWhenPanelClosed;
    bool EnableIndividualResume;
    bool ControlPowerLED;
    bool InverseVisual_PlayPause;
    bool EverytimeStopOfEnd;
    bool WhenLateDecodingWaitPause;
    u32  BacklightTimeOutSec;
} TProcState_DPG;

enum EImageStartPos {EISP_LeftTop,EISP_RightTop,EISP_LeftBottom,EISP_RightBottom};

typedef struct {
    s32 MultipleFix8;
    bool ShowInfomation;
    bool ShowControlIcons;
    bool DoubleSpeedKey;
    bool DoubleSpeedTouch;
    bool MultipleResume;
    bool AutoFitting;
    EImageStartPos StartPos;
    bool EffectHeightPadding;
    bool EffectPastelForTopBG;
    bool EffectPastelForBottomBG;
} TProcState_Image;

enum ETextTopScrMode {ETTSM_LightOff,ETTSM_Text,ETTSM_Clock};
enum ETextLineSpace {ETLS_Small,ETLS_Middle,ETLS_Large};

#define Text_FontSize_Small (12)
#define Text_FontSize_Middle (14)
#define Text_FontSize_Large (16)

enum ETextClearTypeFont {ETCTF_None,ETCTF_Lite,ETCTF_Normal,ETCTF_Heavy};

typedef struct {
    ETextTopScrMode TopScrMode;
    bool isSwapDisp;
    bool BacklightFlag;
    EProcStateScreenSaver ScreenSaver;
    u32 FontSize;
    ETextClearTypeFont ClearTypeFont;
    ETextLineSpace LineSpace;
    u32 DefaultCodePage;
    bool UseSmoothScroll;
    bool BButtonToExitText;
    bool LockScrollBar;
    bool DetectCharCode_ANSI,DetectCharCode_EUC,DetectCharCode_UTF16BE,DetectCharCode_UTF16LE,DetectCharCode_UTF8;
} TProcState_Text;

enum EMemoEditPenColor {EMEPC_Black,EMEPC_Blue,EMEPC_Green,EMEPC_Red};

typedef struct {
    bool TickLine;
    EMemoEditPenColor PenColor;
} TProcState_MemoEdit;

#define ProcState_ReadWriteSize (2048)

typedef struct {
    u32 Version;
    TProcState_System System;
    TProcState_FileList FileList;
    TProcState_ScreenSaver ScreenSaver;
    TProcState_Music Music;
    TProcState_Audio Audio;
    TProcState_DPG DPG;
    TProcState_Image Image;
    TProcState_Text Text;
    TProcState_MemoEdit MemoEdit;
    u8 PaddingForSectorAlign[ProcState_ReadWriteSize];
} TProcState;

extern TProcState ProcState;

extern bool ProcState_RequestSave;

extern void ApplyCurrentBacklightLevel(void);
extern void ChangePrevBacklightLevel(void);
extern void ChangeNextBacklightLevel(void);

enum EBGBMPType {EBGBT_None=0,EBGBT_8bit=1,EBGBT_15bit=2};

#endif
