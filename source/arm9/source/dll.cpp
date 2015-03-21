
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "fat2.h"
#include "memtool.h"
#include "strtool.h"
#include "shell.h"
#include "splash.h"
#include "maindef.h"
#include "procstate.h"

#include "arm9tcm.h"

#include "dll.h"

#include "internaldrivers.h"

#include "dll_StdLib.h"

//#define ShowPluginInfo
//#define ShowPluginInfo_Deep

bool DLL_LoadLibrary(TMM *_pMM,TPluginBody *pPB,const TPlugin_StdLib *pStdLib,void *pbin,int binsize)
{
  memset(pPB,0,sizeof(TPluginBody));
  pPB->pMM=_pMM;
  
  TPluginHeader *pPH=&pPB->PluginHeader;
  
  memmove(pPH,pbin,sizeof(TPluginHeader));
  
#ifdef ShowPluginInfo
  _consolePrint("MSP Header\n");
  _consolePrintf("ID=%x ver%d.%d\n",pPH->ID,pPH->VersionHigh,pPH->VersionLow);
  {
    char *pts;
    switch((EPluginType)(pPH->PluginType)){
      case EPT_None: pts="NULL"; break;
      case EPT_Image: pts="Image"; break;
      case EPT_Sound: pts="Sound"; break;
      default: pts="undefined"; break;
    }
    _consolePrintf("PluginType=%x %s\n",pPH->PluginType,pts);
  }
  _consolePrintf("Data 0x%x-0x%x\n",pPH->DataStart,pPH->DataEnd);
  _consolePrintf("got  0x%x-0x%x\n",pPH->gotStart,pPH->gotEnd);
  _consolePrintf("bss  0x%x-0x%x\n",pPH->bssStart,pPH->bssEnd);
  {
    char *str=pPH->info;
    _consolePrintf("Name=%s\n",str);
    while(*str!=0){
      *str++;
    }
    *str++;
    _consolePrintf("Author=%s\n",str);
  }
  _consolePrint("\n");
#endif
  
  pPB->DataSize=binsize;
  pPB->pData=pbin;
  
#ifdef ShowPluginInfo
  _consolePrintf("Plugin LoadAddress=0x%08x\n",(u32)pPB->pData);
#endif
  
#ifdef ShowPluginInfo
  {
    u32 crc=0;
    u32 size=pPB->DataSize/4;
    u32 *pdata=(u32*)pPB->pData;
    for(u32 idx=0;idx<size;idx++){
      crc^=*pdata++;
    }
    //tta 0xbf0df25d
    //ogg 0x7df2d889
    _consolePrintf("BodyCRC= 0x%08x.\n",crc);
  }
#endif

  pPB->bssSize=pPH->bssEnd-pPH->bssStart;
  pPB->pbss=safemalloc_chkmem(pPB->pMM,pPB->bssSize);
  MemSet8CPU(0,pPB->pbss,pPB->bssSize);
  _consolePrintf("BSS: ptr=0x%08x, size=%dbyte.\n",pPB->pbss,pPB->bssSize);
  
#ifdef ShowPluginInfo
  u32 plug_got_bssbaseofs=pPH->bssStart;
#endif
  u32 plug_got_ofs=pPH->gotStart-pPH->DataStart;
  u32 plug_got_size=pPH->gotEnd-pPH->gotStart;
  
#ifdef ShowPluginInfo
  _consolePrintf("allocbss 0x%08x (0x%xbyte)\n",(u32)pPB->pbss,pPB->bssSize);
  _consolePrintf("got_bssbaseofs=0x%x\n",plug_got_bssbaseofs);
  _consolePrintf("got_ofs=0x%x got_size=0x%x\n",plug_got_ofs,plug_got_size);
#endif
  
  {
    u32 *padr=(u32*)pPB->pData;
    for(u32 idx=64/4;idx<plug_got_ofs/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)pPB->pData;
    for(u32 idx=(plug_got_ofs+plug_got_size)/4;idx<((u32)pPH->DataEnd-(u32)pPH->DataStart)/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
  {
    u32 *padr=(u32*)((u32)pPB->pData+plug_got_ofs);
    
    for(u32 idx=0;idx<plug_got_size/4;idx++){
      u32 adr=padr[idx];
      if((pPH->bssStart<=adr)&&(adr<pPH->bssEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
        _consolePrintf("b%x:%x ",idx*4,adr);
#endif
#endif
        padr[idx]=(u32)pPB->pbss+(adr-pPH->bssStart);
        }else{
        if((pPH->DataStart<=adr)&&(adr<pPH->DataEnd)){
#ifdef ShowPluginInfo
#ifdef ShowPluginInfo_Deep
          _consolePrintf("d%x:%x ",idx*4,adr);
#endif
#endif
          padr[idx]=(u32)pPB->pData+(adr-pPH->DataStart);
        }
      }
    }
  }
  
#ifdef ShowPluginInfo
  {
    u32 crc=0;
    u32 size=pPB->DataSize/4;
    u32 *pdata=(u32*)pPB->pData;
    for(u32 idx=0;idx<size;idx++){
      crc^=*pdata++;
    }
    //tta 0xbf0df25d
    //ogg 0x7df2d889
    _consolePrintf("BodyCRC= 0x%08x.\n",crc);
  }
#endif
  
  {
    u32 src;
    u32 *pdst;
    
    src=pPH->LoadLibrary;
    if(src==0){
      _consolePrint("LoadLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->LoadLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->FreeLibrary;
    if(src==0){
      _consolePrint("FreeLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->FreeLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
    
    src=pPH->QueryInterfaceLibrary;
    if(src==0){
      _consolePrint("QueryInterfaceLibrary:BootStrap function is NULL.\n");
      DLL_FreeLibrary(pPB,false);
      return(false);
    }
    pdst=(u32*)&pPB->QueryInterfaceLibrary;
    *pdst=(u32)pPB->pData+(src-pPH->DataStart);
  }
  
#ifdef ShowPluginInfo
  {
    u32 crc=0;
    u32 size=pPB->DataSize/4;
    u32 *pdata=(u32*)pPB->pData;
    for(u32 idx=0;idx<size;idx++){
      crc^=*pdata++;
    }
    //tta 0xbf0df25d
    //ogg 0x7df2d889
    _consolePrintf("BodyCRC= 0x%08x.\n",crc);
  }
#endif

#ifdef ShowPluginInfo
  _consolePrintf("0x%08x LoadLibrary\n",(u32)pPB->LoadLibrary);
  _consolePrintf("0x%08x FreeLibrary\n",(u32)pPB->FreeLibrary);
  _consolePrintf("0x%08x QueryInterfaceLib.\n",(u32)pPB->QueryInterfaceLibrary);
#endif
  
  if(pPB->LoadLibrary==NULL){
    _consolePrint("LoadLibrary:LoadLibrary() is NULL.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
#ifdef ShowPluginInfo
  {
    u32 *p=(u32*)pPB->LoadLibrary;
    _consolePrintf("pPB->LoadLibrary: %08x, %08x, %08x, %08x\n",p[0],p[1],p[2],p[3]);
  }
#endif

  bool res=pPB->LoadLibrary(pStdLib,pPB->pMM);
  
  if(res==false){
    _consolePrint("LoadLibrary:LoadLibrary() false.\n");
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
#ifdef ShowPluginInfo
  {
    u32 crc=0;
    u32 size=pPB->DataSize/4;
    u32 *pdata=(u32*)pPB->pData;
    for(u32 idx=0;idx<size;idx++){
      crc^=*pdata++;
    }
    //tta 0xbf0df25d
    //ogg 0x7df2d889
    _consolePrintf("BodyCRC= 0x%08x.\n",crc);
  }
#endif

#ifdef ShowPluginInfo
  {
    u32 *p=(u32*)pPB->QueryInterfaceLibrary;
    _consolePrintf("pPB->QueryInterfaceLibrary: %08x, %08x, %08x, %08x\n",p[0],p[1],p[2],p[3]);
  }
#endif

  pPB->pSL=NULL;
  pPB->pIL=NULL;
  
  switch((EPluginType)(pPH->PluginType)){
    case EPT_None: {
#ifdef ShowPluginInfo
      _consolePrint("LoadLibrary:PluginType == None.\n");
#endif
      DLL_FreeLibrary(pPB,false);
      return(false);
    } break;
    case EPT_Image: {
      pPB->pIL=(TPlugin_ImageLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf("ImageInterfacePtr 0x%08x\n",(u32)pPB->pIL);
#endif
    } break;
    case EPT_Sound: {
      pPB->pSL=(TPlugin_SoundLib*)pPB->QueryInterfaceLibrary();
#ifdef ShowPluginInfo
      _consolePrintf("SoundInterfacePtr 0x%08x\n",(u32)pPB->pSL);
#endif
    } break;
  }
  
  if((pPB->pIL==NULL)&&(pPB->pSL==NULL)){
    _consolePrintf("LoadLibrary:not found function list error. (%d)\n",pPH->PluginType);
    DLL_FreeLibrary(pPB,false);
    return(false);
  }
  
#ifdef ShowPluginInfo
  {
    u32 crc=0;
    u32 size=pPB->DataSize/4;
    u32 *pdata=(u32*)pPB->pData;
    for(u32 idx=0;idx<size;idx++){
      crc^=*pdata++;
    }
    //tta 0xbf0df25d
    //ogg 0x7df2d889
    _consolePrintf("BodyCRC= 0x%08x.\n",crc);
  }
#endif

#ifdef ShowPluginInfo
  {
    u32 *p=(u32*)pPB->FreeLibrary;
    _consolePrintf("pPB->FreeLibrary: %08x, %08x, %08x, %08x\n",p[0],p[1],p[2],p[3]);
  }
#endif
  
  MWin_CurrentPluginType=(EPluginType)(pPH->PluginType);
  
#ifdef ShowPluginInfo
  _consolePrint("LoadLibrary:Initialized.\n");
#endif
  
  return(true);
}

void DLL_FreeLibrary(TPluginBody *pPB,bool callfree)
{
  MWin_CurrentPluginType=EPT_None;
  MWin_Max=0;
  if(MWin_pTitleStr!=NULL){
    safefree(pPB->pMM,MWin_pTitleStr); MWin_pTitleStr=NULL;
  }
  
  if(callfree==true){
    if(pPB!=NULL){
      if(pPB->FreeLibrary!=NULL) pPB->FreeLibrary();
    }
  }
  
  if(pPB->pData!=NULL){
    safefree(pPB->pMM,pPB->pData); pPB->pData=NULL;
  }
  if(pPB->pbss!=NULL){
    safefree(pPB->pMM,pPB->pbss); pPB->pbss=NULL;
  }
  
//  memset(pPB,0,sizeof(TPluginBody));
  
#ifdef ShowPluginInfo
  _consolePrint("FreeLibrary:Destroied.\n");
#endif
}

// ------------------------------------------------------------

typedef struct {
  u32 ext;
  EPluginType PluginType;
  char fn[PluginFilenameMax];
  const TPlugin_ImageLib *pIL;
  const TPlugin_SoundLib *pSL;
} TDLLList;

DATA_IN_AfterSystem static int DLLListCount;
DATA_IN_AfterSystem TDLLList *DLLList=NULL;

DATA_IN_IWRAM_MainPass static void DLLList_Regist_Dummy(EPluginType PluginType,const char *pext)
{
  u32 Ext32=0;
  {
    const char *ptmp=pext;
    while((*ptmp!=0)&&(*ptmp!='.')){
      u32 ch=*ptmp++;
      if((0x41<=ch)&&(ch<=0x5a)) ch+=0x20;
      Ext32=(Ext32>>8)|(ch<<24);
    }
    Ext32>>=8;
  }
  if(Ext32==0) return;
  
  DLLList[DLLListCount].ext=Ext32;
  DLLList[DLLListCount].PluginType=PluginType;
  DLLList[DLLListCount].fn[0]=0;
  DLLListCount++;
}

DATA_IN_IWRAM_MainPass void DLLList_Regist(char *fn)
{
  TPluginHeader PH;
  
  Shell_FAT_ReadMSP_ReadHeader(fn,&PH,sizeof(TPluginHeader));
  
  if(PH.ID!=0x0050534d){
    _consolePrintf("%s is illigal format.\n",fn);
    return;
  }
  if(0x05<PH.VersionHigh){
    _consolePrintf("%s check version error ver%d.%d\n",fn,PH.VersionHigh,PH.VersionLow);
    return;
  }
  
  switch((EPluginType)PH.PluginType){
    case EPT_None: {
      _consolePrintf("%s Plugin type is EPT_None(0)\n",fn);
    } break;
    case EPT_Image: case EPT_Sound: {
      for(int idx=0;idx<PluginHeader_ExtCount;idx++){
        if(PH.ext[idx]!=0){
          DLLList[DLLListCount].ext=PH.ext[idx];
          DLLList[DLLListCount].PluginType=(EPluginType)PH.PluginType;
          strncpy(DLLList[DLLListCount].fn,fn,PluginFilenameMax);
#ifdef ShowPluginInfo
          _consolePrintf("regist:%2d .%s %s\n",DLLListCount,(char*)&DLLList[DLLListCount].ext,DLLList[DLLListCount].fn);
#endif
          DLLListCount++;
        }
      }
    } break;
    default: {
      _consolePrintf("unknown plugin type(%d):%2d %s\n",PH.PluginType,DLLListCount,DLLList[DLLListCount].fn);
    } break;
  }
}

DATA_IN_IWRAM_MainPass void DLLList_Init(void)
{
  DLLListCount=256;
  
  DLLList=(TDLLList*)safemalloc_chkmem(&MM_System,sizeof(TDLLList)*DLLListCount);
  
  DLLListCount=0;
  
#ifdef ExceptMP3
  DLLList_Regist_Dummy(EPT_Sound,"mp1");
  DLLList_Regist_Dummy(EPT_Sound,"mp2");
  DLLList_Regist_Dummy(EPT_Sound,"mp3");
#endif

#ifdef ExceptMIDI
  DLLList_Regist_Dummy(EPT_Sound,"mid");
  DLLList_Regist_Dummy(EPT_Sound,"rcp");
  DLLList_Regist_Dummy(EPT_Sound,"r36");
#endif

#ifdef ExceptGME
  DLLList_Regist_Dummy(EPT_Sound,"nsf");
  DLLList_Regist_Dummy(EPT_Sound,"gbs");
  DLLList_Regist_Dummy(EPT_Sound,"hes");
  DLLList_Regist_Dummy(EPT_Sound,"ay");
  DLLList_Regist_Dummy(EPT_Sound,"sap");
  DLLList_Regist_Dummy(EPT_Sound,"kss");
#endif
  
#ifdef ExceptOGG
  DLLList_Regist_Dummy(EPT_Sound,"ogg");
#endif
  
#ifdef ExceptWAVE
  DLLList_Regist_Dummy(EPT_Sound,"wav");
#endif
  
#ifdef ExceptJpeg
  DLLList_Regist_Dummy(EPT_Image,"jpg");
  DLLList_Regist_Dummy(EPT_Image,"jpe");
#endif

#ifdef ExceptBmp
  DLLList_Regist_Dummy(EPT_Image,"bmp");
#endif

#ifdef ExceptGif
  DLLList_Regist_Dummy(EPT_Image,"gif");
#endif

#ifdef ExceptPsd
  DLLList_Regist_Dummy(EPT_Image,"psd");
#endif

#ifdef ExceptPng
  DLLList_Regist_Dummy(EPT_Image,"png");
#endif
  
  _consolePrint("Plugin Read headers.\n");
  
  if(Shell_FAT_fopen_isExists_Internal(MSPPackageFilename)) {
      Shell_FAT_ReadMSP_Open();
  
      //DLLList_Regist("ogg.msp");
      //DLLList_Regist("tta.msp");
  
      //DLLList_Regist("m4a.msp");
  
      //DLLList_Regist("mikmod.msp");
      //DLLList_Regist("spc.msp");
      //DLLList_Regist("wav.msp");
      //DLLList_Regist("wma.msp");
  
      //DLLList_Regist("png.msp");
      //DLLList_Regist("psd.msp");
      //DLLList_Regist("gif.msp");
  
      Shell_FAT_ReadMSP_Close();
  }
  
  {
    u32 size=sizeof(TDLLList)*DLLListCount;
    DLLList=(TDLLList*)saferealloc(&MM_System,DLLList,size);
    _consolePrintf("%d plugin Registed. (%dbytes)\n",DLLListCount,size);
  }
  
  _consolePrint("Support formats. ");
  
  char extlst[128];
  extlst[0]=0;
  for(u32 idx=0;idx<DLLListCount;idx++){
    u32 ext32=DLLList[idx].ext;
    char str[6]={0,0,0,0,0,0};
    u32 pos=0;
    str[pos++]='/';
    str[pos++]='.';
    u32 c;
    c=(ext32>>0)&0xff;
    if(c!=0) str[pos++]=c;
    c=(ext32>>8)&0xff;
    if(c!=0) str[pos++]=c;
    c=(ext32>>16)&0xff;
    if(c!=0) str[pos++]=c;
    c=(ext32>>24)&0xff;
    if(c!=0) str[pos++]=c;
    strcat(extlst,str);
    if(100<=strlen(extlst)){
      _consolePrint(extlst);
      extlst[0]=0;
    }
  }
  _consolePrintf("%s\n",extlst);
}

void DLLList_Free(void)
{
  if(DLLList!=NULL){
    safefree(&MM_System,DLLList); DLLList=NULL;
  }
}

EPluginType DLLList_isSupportFormatFromExt(const char *ExtStr)
{
  if((ExtStr==NULL)||(ExtStr[0]==0)) return(EPT_None);
  
  u32 ext=0;
  
  u32 c;
  
  c=(u32)ExtStr[1];
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext|=c << 0;
    c=(u32)ExtStr[2];
    if(c!=0){
      if((0x41<=c)&&(c<0x5a)) c+=0x20;
      ext|=c << 8;
      c=(u32)ExtStr[3];
      if(c!=0){
        if((0x41<=c)&&(c<0x5a)) c+=0x20;
        ext|=c << 16;
        c=(u32)ExtStr[4];
        if(c!=0){
          if((0x41<=c)&&(c<0x5a)) c+=0x20;
          ext|=c << 24;
        }
      }
    }
  }
  
  for(int idx=0;idx<DLLListCount;idx++){
    if(DLLList[idx].ext==ext) return(DLLList[idx].PluginType);
  }
  
  return(EPT_None);
}

EPluginType DLLList_isSupportFormatFromFilenameUnicode(const UnicodeChar *pFilenameUnicode)
{
  if((pFilenameUnicode==NULL)||(pFilenameUnicode[0]==0)) return(EPT_None);
  
  u32 Ext32=0;
  {
    const UnicodeChar *ptmp=pFilenameUnicode;
    while(*ptmp!=0){
      u32 ch=*ptmp++;
      if(ch==(u32)'.'){
        Ext32=0;
        }else{
        if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
        Ext32=(Ext32<<8)|ch;
      }
    }
  }
  
  return(DLLList_isSupportFormatExt32(Ext32));
}

EPluginType DLLList_isSupportFormatFromFilenameAlias(const char *pFilenameAlias)
{
  if((pFilenameAlias==NULL)||(pFilenameAlias[0]==0)) return(EPT_None);
  
  u32 Ext32=0;
  {
    const char *ptmp=pFilenameAlias;
    while(*ptmp!=0){
      u32 ch=*ptmp++;
      if(ch==(u32)'.'){
        Ext32=0;
        }else{
        if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
        Ext32=(Ext32<<8)|ch;
      }
    }
  }
  
  return(DLLList_isSupportFormatExt32(Ext32));
}

EPluginType DLLList_isSupportFormatExt32(u32 Ext32)
{
  u32 ext=0;
  
  u32 c;
  
  c=(Ext32>>0)&0xff;
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext=(ext<<8)|c;
  }
  c=(Ext32>>8)&0xff;
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext=(ext<<8)|c;
  }
  c=(Ext32>>16)&0xff;
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext=(ext<<8)|c;
  }
  c=(Ext32>>24)&0xff;
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext=(ext<<8)|c;
  }
  
  for(int idx=0;idx<DLLListCount;idx++){
    if(DLLList[idx].ext==ext) return(DLLList[idx].PluginType);
  }
  
  return(EPT_None);
}

EPluginType DLLList_GetPluginFilename(const char *extstr,char *resfn)
{
  if(resfn==NULL) StopFatalError(10302,"DLLList_GetPluginFilename: resfn is NULL.\n");
  
  resfn[0]=0;
  
  if((extstr==NULL)||(extstr[0]==0)) return(EPT_None);
  
  u32 ext=0;
  
  u32 c;
  
  c=(u32)extstr[1];
  if(c!=0){
    if((0x41<=c)&&(c<0x5a)) c+=0x20;
    ext|=c << 0;
    c=(u32)extstr[2];
    if(c!=0){
      if((0x41<=c)&&(c<0x5a)) c+=0x20;
      ext|=c << 8;
      c=(u32)extstr[3];
      if(c!=0){
        if((0x41<=c)&&(c<0x5a)) c+=0x20;
        ext|=c << 16;
        c=(u32)extstr[4];
        if(c!=0){
          if((0x41<=c)&&(c<0x5a)) c+=0x20;
          ext|=c << 24;
        }
      }
    }
  }
  
  for(int idx=0;idx<DLLListCount;idx++){
    if(DLLList[idx].ext==ext){
      strncpy(resfn,DLLList[idx].fn,PluginFilenameMax);
      return(DLLList[idx].PluginType);
    }
  }
  
  StopFatalError(0,"Not found dll in list. [0x%08x]",ext);
  return(EPT_None);
}

// ----------------------------------

TPluginBody* DLLList_LoadPlugin(EPluginType PluginType,const char *fn)
{
  TMM *pMM=NULL;
  switch(PluginType){
    case EPT_Image: pMM=&MM_DLLImage; break;
    case EPT_Sound: pMM=&MM_DLLSound; break;
    case EPT_None: default: StopFatalError(0,"Unknown or none plugin type. [%s]",fn); break;
  }
  
  TPluginBody *pPB=(TPluginBody*)safemalloc_chkmem(pMM,sizeof(TPluginBody));
  
  void *buf;
  s32 size;
  
  Shell_FAT_ReadMSP_Open();
  Shell_FAT_ReadMSP_ReadBody(pMM,fn,&buf,&size);
  Shell_FAT_ReadMSP_Close();
  
  if((buf==NULL)||(size==0)){
    _consolePrintf("%s file read error.\n",fn);
    return(NULL);
  }
  
  if(DLL_LoadLibrary(pMM,pPB,Plugin_GetStdLib(),buf,size)==false){
    safefree(pMM,pPB); pPB=NULL;
    return(NULL);
  }
  
  return(pPB);
}

void DLLList_FreePlugin(TPluginBody *pPB)
{
  if(pPB==NULL) return;
  
  if(pPB->pIL!=NULL) pPB->pIL->Free();
  if(pPB->pSL!=NULL) pPB->pSL->Free();
  
  DLL_FreeLibrary(pPB,true);
  
  safefree(pPB->pMM,pPB); pPB=NULL;
}

// ----------------------------

// ----------------------------

