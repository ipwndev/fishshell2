
#ifndef dll_h
#define dll_h

#include "plugin.h"

#define MakeExt32(c0,c1,c2,c3) ( ((u32)(c0)<<24) | ((u32)(c1)<<16) | ((u32)(c2)<<8) | ((u32)(c3)<<0) )

#define PluginHeader_ExtCount (16)

typedef struct {
  u32 ID;
  u8 VersionHigh;
  u8 VersionLow;
  u8 PluginType;
  u8 Dummy0;
  u32 DataStart;
  u32 DataEnd;
  u32 gotStart;
  u32 gotEnd;
  u32 bssStart;
  u32 bssEnd;
  u32 LoadLibrary;
  u32 FreeLibrary;
  u32 QueryInterfaceLibrary;
  u32 Dummy1;
  u32 ext[PluginHeader_ExtCount];
  char info[256-112];
} TPluginHeader;

typedef struct {
  TPluginHeader PluginHeader;
  TMM *pMM;
  void *pData;
  int DataSize;
  void *pbss;
  int bssSize;
  bool (*LoadLibrary)(const TPlugin_StdLib *_pStdLib,TMM *pMM);
  void (*FreeLibrary)(void);
  int (*QueryInterfaceLibrary)(void);
  const TPlugin_ImageLib *pIL;
  const TPlugin_SoundLib *pSL;
} TPluginBody;

extern bool DLL_LoadLibrary(TMM *_pMM,TPluginBody *pPB,void *pbin,int binsize);
extern void DLL_FreeLibrary(TPluginBody *pPB,bool callfree);

extern void DLLList_Regist(char *fn);
extern void DLLList_Init(void);
extern void DLLList_Free(void);
extern EPluginType DLLList_isSupportFormatFromExt(const char *ExtStr);
extern EPluginType DLLList_isSupportFormatFromFilenameUnicode(const UnicodeChar *pFilenameUnicode);
extern EPluginType DLLList_isSupportFormatFromFilenameAlias(const char *pFilenameAlias);
extern EPluginType DLLList_isSupportFormatExt32(u32 Ext32);
extern EPluginType DLLList_GetPluginFilename(const char *extstr,char *resfn);
extern TPluginBody* DLLList_LoadPlugin(EPluginType PluginType,const char *fn);
extern void DLLList_FreePlugin(TPluginBody *pPB);

#define PluginFilenameMax (16)

#endif

