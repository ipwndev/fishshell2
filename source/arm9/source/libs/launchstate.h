
#ifndef launchstate_h
#define launchstate_h

#include "unicode.h"
#include "shell.h"

extern void LaunchState_Clear(void);
extern bool LaunchState_isOpened(void);
extern void LaunchState_Open(void);
extern void LaunchState_Close(void);
extern void LaunchState_Save(void);

enum ELaunchState_Tabs {ELST_Launch=0,ELST_NDS,ELST_Count};

extern void LaunchState_Add(ELaunchState_Tabs Tabs,const UnicodeChar *pFullPathUnicode);

#define LaunchState_TabsCount (ELST_Count)
#define LaunchState_Tab_FilesCountMax (5)

typedef struct {
  u32 FilesCount;
  UnicodeChar FullPathUnicode[LaunchState_Tab_FilesCountMax][MaxFilenameLength];
} TLaunchState_Tab;

typedef struct {
  u32 Version;
  ELaunchState_Tabs LastTab;
  TLaunchState_Tab Tabs[LaunchState_TabsCount];
  u32 Reserved0,Reserved1,Reserved2,Reserved3;
} TLaunchState;

extern TLaunchState *pLaunchState;

#endif

