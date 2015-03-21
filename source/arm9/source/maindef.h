
#ifndef maindef_h
#define maindef_h

#include "_console.h"

extern const char ROMTITLE[];
extern const char ROMVERSION[];
extern const char ROMDATE[];
extern const char ROMDATESHORT[];
extern const char ROMDATEVERYSHORT[];
extern const char ROMENV[];
extern const char ROMWEB[];
extern const char OverlayHeader_ID[];
extern const char OverlayHeader_ID_CanNotAccess[];

#include "glib.h"

extern CglFont *pCglFontDefault;

extern void WaitForVBlank(void);

extern CglB15 *pSettingBGBM,*pTimerBodyBGBM;

extern u32 FolderDlg_TargetItemIndex; // (u32)-1==StartBGM

extern bool isExistsROMEO2;

extern bool isCustomFromFileList;

extern bool WaitKeyRelease;

extern bool LongPressRequest;

typedef enum {ETT_AButton=0,ETT_LButton,ETT_RButton,ETT_Touch,ETT_PhoneSwitch} ETriggerType;

typedef struct {
  void (*Start)(void);
  void (*VsyncUpdate)(u32 VsyncCount);
  void (*End)(void);
  void (*KeyPress)(u32 VsyncCount,u32 Keys);
  void (*KeyReleases)(u32 VsyncCount);
  void (*KeyLongPress)(u32 Keys);
  void (*KeySameLRDown)(void);
  void (*KeySameLRUp)(void);
  void (*MouseDown)(s32 x,s32 y);
  void (*MouseMove)(s32 x,s32 y);
  void (*MouseUp)(s32 x,s32 y);
  void (*strpcmRequestStop)(void);
  void (*PanelOpen)(void);
  void (*PanelClose)(void);
  void (*VBlankHandler)(void);
  void (*ExternalPowerAttach)(void);
  void (*ExternalPowerDetach)(void);
  void (*Trigger_ProcStart)(ETriggerType TriggerType);
  void (*Trigger_ProcEnd)(ETriggerType TriggerType);
  void (*Trigger_Down)(ETriggerType TriggerType);
  void (*Trigger_Up)(ETriggerType TriggerType);
  void (*Trigger_LongStart)(ETriggerType TriggerType);
  void (*Trigger_LongEnd)(ETriggerType TriggerType);
  void (*Trigger_SingleClick)(ETriggerType TriggerType);
  void (*Trigger_SingleLongStart)(ETriggerType TriggerType);
  void (*Trigger_SingleLongEnd)(ETriggerType TriggerType);
  void (*Trigger_DoubleClick)(ETriggerType TriggerType);
  void (*Trigger_DoubleLongStart)(ETriggerType TriggerType);
  void (*Trigger_DoubleLongEnd)(ETriggerType TriggerType);
  void (*Trigger_TripleClick)(ETriggerType TriggerType);
  void (*MWin_ProgressShow)(const char *TitleStr,s32 Max);
  void (*MWin_ProgressSetPos)(const char *pTitleStr,s32 pos,s32 max);
  void (*MWin_ProgressDraw)(const char *pstr0,const char *pstr1,s32 pos,s32 max);
  void (*MWin_ProgressHide)(void);
} TCallBack;

extern void CallBack_ExecuteVBlankHandler(void);
extern TCallBack* CallBack_GetPointer(void);

typedef enum {ENP_Loop,ENP_ChkDsk,ENP_Setup,ENP_FileList,ENP_SysMenu,ENP_DPGCustom,ENP_DPGPlay,
	ENP_ImageCustom,ENP_ImageView,ENP_TextCustom,ENP_TextMenu,ENP_TextView,ENP_BinView,ENP_Launch,
	ENP_Custom,ENP_BootROM,ENP_MemoEdit,ENP_MemoList,ENP_AudioPlay,ENP_AudioCustom} ENextProc;
typedef enum {EPFE_None,EPFE_LeftToRight,EPFE_RightToLeft,EPFE_UpToDown,
	EPFE_DownToUp,EPFE_CrossFade,EPFE_FastCrossFade} EProcFadeEffect;

static void SetNextProc(ENextProc _NextProc,EProcFadeEffect _ProcFadeEffect)
{
  extern ENextProc NextProc;
  extern EProcFadeEffect ProcFadeEffect;
  NextProc=_NextProc;
  ProcFadeEffect=_ProcFadeEffect;
}

static ENextProc GetNextProc(void)
{
  extern ENextProc NextProc;
  return(NextProc);
}

static void SetProcFadeEffect(EProcFadeEffect _ProcFadeEffect)
{
  extern EProcFadeEffect ProcFadeEffect;
  ProcFadeEffect=_ProcFadeEffect;
}

extern void ProcChkDsk_SetCallBack(TCallBack *pCallBack);
extern void ProcSetup_SetCallBack(TCallBack *pCallBack);
extern void ProcFileList_SetCallBack(TCallBack *pCallBack);
extern void ProcSysMenu_SetCallBack(TCallBack *pCallBack);
extern void ProcDPGCustom_SetCallBack(TCallBack *pCallBack);
extern void ProcDPGPlay_SetCallBack(TCallBack *pCallBack);
extern void ProcImageCustom_SetCallBack(TCallBack *pCallBack);
extern void ProcImageView_SetCallBack(TCallBack *pCallBack);
extern void ProcTextCustom_SetCallBack(TCallBack *pCallBack);
extern void ProcTextMenu_SetCallBack(TCallBack *pCallBack);
extern void ProcTextView_SetCallBack(TCallBack *pCallBack);
extern void ProcBinView_SetCallBack(TCallBack *pCallBack);
extern void ProcLaunch_SetCallBack(TCallBack *pCallBack);
extern void ProcCustom_SetCallBack(TCallBack *pCallBack);
extern void ProcBootROM_SetCallBack(TCallBack *pCallBack);
extern void ProcMemoEdit_SetCallBack(TCallBack *pCallBack);
extern void ProcMemoList_SetCallBack(TCallBack *pCallBack);
extern void ProcAudioCustom_SetCallBack(TCallBack *pCallBack);

extern void ScreenMain_Flip_ProcFadeEffect(void);

extern void DrawItemsListToScreenSub(bool ShowCursor);

#include "unicode.h"

#define MaxFilenameLength (256)
extern UnicodeChar RelationalFilePathUnicode[MaxFilenameLength];
extern UnicodeChar RelationalFileNameUnicode[MaxFilenameLength];
extern u32 RelationalFilePos;

static inline void RelationalFile_Clear(void)
{
  RelationalFilePathUnicode[0]=0;
  RelationalFileNameUnicode[0]=0;
  RelationalFilePos=0;
}

typedef enum {ETE_Auto=0,ETE_ANSI,ETE_EUC,ETE_UTF16BE,ETE_UTF16LE,ETE_UTF8,ETE_TextEdit} ETextEncode;
extern ETextEncode ManualTextEncode;
extern bool ManualTextEncode_OverrideFlag;
extern bool isExistsTextEditor;

//#define EnableTriggerLog

static inline void CallBack_MWin_ProgressShow(const char *TitleStr,s32 Max)
{
  TCallBack *pcb=CallBack_GetPointer();
  if(pcb->MWin_ProgressShow!=NULL) pcb->MWin_ProgressShow(TitleStr,Max);
}

static inline void CallBack_MWin_ProgressSetPos(const char *pTitleStr,s32 pos,s32 max)
{
  TCallBack *pcb=CallBack_GetPointer();
  if(pcb->MWin_ProgressSetPos!=NULL) pcb->MWin_ProgressSetPos(pTitleStr,pos,max);
}

static inline void CallBack_MWin_ProgressDraw(const char *pstr0,const char *pstr1,s32 pos,s32 max)
{
  TCallBack *pcb=CallBack_GetPointer();
  if(pcb->MWin_ProgressDraw!=NULL) pcb->MWin_ProgressDraw(pstr0,pstr1,pos,max);
}

static inline void CallBack_MWin_ProgressHide(void)
{
  TCallBack *pcb=CallBack_GetPointer();
  if(pcb->MWin_ProgressHide!=NULL) pcb->MWin_ProgressHide();
}

#endif

