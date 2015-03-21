
#pragma Ospace

#include <nds.h>

#include "_console.h"
#include "_const.h"
#include "maindef.h"
#include "dll.h"
#include "unicode.h"
#include "strtool.h"
#include "procstate.h"
#include "launchstate.h"
#include "shell.h"
#include "lang.h"
#include "extlink.h"
#include "sndeff.h"
#include "ErrorDialog.h"
#include "strpcm.h"

#include "arm9tcm.h"

#include "md5.h"

#include "../../ipc6.h"

#include "OverlayManager.h"

#include "BootROM.h"

typedef struct {
	UnicodeChar PathUnicode[MaxFilenameLength];
	UnicodeChar FilenameUnicode[MaxFilenameLength];
	char FullPathAlias[MaxFilenameLength];
	bool RequestBackupSave;
	bool Execute;
} TBootROMInfo;

DATA_IN_AfterSystem static TBootROMInfo BootROMInfo;
// ------------------------------------

DATA_IN_IWRAM_MainPass void BootROM_Init(void)
{
	BootROMInfo.PathUnicode[0]=0;
	BootROMInfo.FilenameUnicode[0]=0;
	BootROMInfo.FullPathAlias[0]=0;
	BootROMInfo.RequestBackupSave=false;
	BootROMInfo.Execute=false;
}

bool BootROM_GetExecuteFlag(void)
{
  return(BootROMInfo.Execute);
}

const UnicodeChar* BootROM_GetPathUnicode(void)
{
  return(BootROMInfo.PathUnicode);
}

const UnicodeChar* BootROM_GetFilenameUnicode(void)
{
  return(BootROMInfo.FilenameUnicode);
}

const char* BootROM_GetFullPathAlias(void)
{
  return(BootROMInfo.FullPathAlias);
}

// ------------------------------------

#include "extlink_filestruct.h"

void BootROM_SetInfo_TextEdit(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave)
{
  if(!isExistsTextEditor) return;
  
  u32 Ext32=MakeExt32(0,'_','T','E');
      
  u32 bodysize=sizeof(TExtLinkBody);
  TExtLinkBody *pbody=(TExtLinkBody*)safemalloc_chkmem(&MM_Temp,bodysize);
  
  MemSet8CPU(0,pbody,bodysize);
  pbody->ID=ExtLinkBody_ID;
  
  u32 extlinkidx=ExtLink_GetTargetIndex(Ext32);
  if(extlinkidx==(u32)-1){
    pbody->DataFullPathFilenameUnicode[0]=0;
    Unicode_Copy(pbody->NDSFullPathFilenameUnicode,FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode));
    }else{
    Unicode_Copy(pbody->DataFullPathFilenameUnicode,FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode));
    Unicode_Copy(pbody->NDSFullPathFilenameUnicode,ExtLink_GetNDSFullPathFilenameUnicode(extlinkidx));
  }
  
  if(pbody->DataFullPathFilenameUnicode[0]!=0){
    strcpy(pbody->DataFullPathFilenameAlias,ConvertFullPath_Unicode2Alias(pbody->DataFullPathFilenameUnicode));
    SplitItemFromFullPathAlias(pbody->DataFullPathFilenameAlias,pbody->DataPathAlias,pbody->DataFilenameAlias);
    SplitItemFromFullPathUnicode(pbody->DataFullPathFilenameUnicode,pbody->DataPathUnicode,pbody->DataFilenameUnicode);
  }
  
  if(pbody->NDSFullPathFilenameUnicode[0]!=0){
    strcpy(pbody->NDSFullPathFilenameAlias,ConvertFullPath_Unicode2Alias(pbody->NDSFullPathFilenameUnicode));
    SplitItemFromFullPathAlias(pbody->NDSFullPathFilenameAlias,pbody->NDSPathAlias,pbody->NDSFilenameAlias);
    SplitItemFromFullPathUnicode(pbody->NDSFullPathFilenameUnicode,pbody->NDSPathUnicode,pbody->NDSFilenameUnicode);
  }
  
  if(str_isEmpty(pbody->NDSFullPathFilenameAlias)==true) StopFatalError(10002,"Can not found NDS file.\n");
  
  BootROMInfo.RequestBackupSave=RequestBackupSave;
    
  Unicode_Copy(BootROMInfo.PathUnicode,pPathUnicode);
  Unicode_Copy(BootROMInfo.FilenameUnicode,pFilenameUnicode);
  strcpy(BootROMInfo.FullPathAlias,pbody->NDSFullPathFilenameAlias);
  BootROMInfo.Execute=false;
  
  _consolePrintf("DataFullPathFilenameAlias=%s\n",pbody->DataFullPathFilenameAlias);
  _consolePrintf("NDSFullPathFilenameAlias=%s\n",pbody->NDSFullPathFilenameAlias);
  
  FAT_FILE *pf=Shell_FAT_fopenwrite_Root(ExtLinkDATFilename);
  if(pf==NULL) StopFatalError(10003,"ExtLink: Create error. [%s]\n",ExtLinkDATFilename);
  
  FAT2_fwrite(pbody,1,bodysize,pf);
  FAT2_fclose(pf);

  if(pbody!=NULL){
    safefree(&MM_Temp,pbody); pbody=NULL;
  }
  
  SetNextProc(ENP_BootROM,EPFE_None);
}

void BootROM_SetInfo_NoLaunch(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave)
{
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
      
  u32 bodysize=sizeof(TExtLinkBody);
  TExtLinkBody *pbody=(TExtLinkBody*)safemalloc_chkmem(&MM_Temp,bodysize);
  
  MemSet8CPU(0,pbody,bodysize);
  pbody->ID=ExtLinkBody_ID;
  
  u32 extlinkidx=ExtLink_GetTargetIndex(Ext32);
  if(extlinkidx==(u32)-1){
    pbody->DataFullPathFilenameUnicode[0]=0;
    Unicode_Copy(pbody->NDSFullPathFilenameUnicode,FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode));
    }else{
    Unicode_Copy(pbody->DataFullPathFilenameUnicode,FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode));
    Unicode_Copy(pbody->NDSFullPathFilenameUnicode,ExtLink_GetNDSFullPathFilenameUnicode(extlinkidx));
  }
  
  if(pbody->DataFullPathFilenameUnicode[0]!=0){
    strcpy(pbody->DataFullPathFilenameAlias,ConvertFullPath_Unicode2Alias(pbody->DataFullPathFilenameUnicode));
    SplitItemFromFullPathAlias(pbody->DataFullPathFilenameAlias,pbody->DataPathAlias,pbody->DataFilenameAlias);
    SplitItemFromFullPathUnicode(pbody->DataFullPathFilenameUnicode,pbody->DataPathUnicode,pbody->DataFilenameUnicode);
  }
  
  if(pbody->NDSFullPathFilenameUnicode[0]!=0){
    strcpy(pbody->NDSFullPathFilenameAlias,ConvertFullPath_Unicode2Alias(pbody->NDSFullPathFilenameUnicode));
    SplitItemFromFullPathAlias(pbody->NDSFullPathFilenameAlias,pbody->NDSPathAlias,pbody->NDSFilenameAlias);
    SplitItemFromFullPathUnicode(pbody->NDSFullPathFilenameUnicode,pbody->NDSPathUnicode,pbody->NDSFilenameUnicode);
  }
  
  if(str_isEmpty(pbody->NDSFullPathFilenameAlias)==true) StopFatalError(10002,"Can not found NDS file.\n");
  
  BootROMInfo.RequestBackupSave=RequestBackupSave;
    
  Unicode_Copy(BootROMInfo.PathUnicode,pPathUnicode);
  Unicode_Copy(BootROMInfo.FilenameUnicode,pFilenameUnicode);
  strcpy(BootROMInfo.FullPathAlias,pbody->NDSFullPathFilenameAlias);
  BootROMInfo.Execute=false;
  
  _consolePrintf("DataFullPathFilenameAlias=%s\n",pbody->DataFullPathFilenameAlias);
  _consolePrintf("NDSFullPathFilenameAlias=%s\n",pbody->NDSFullPathFilenameAlias);
  
  FAT_FILE *pf=Shell_FAT_fopenwrite_Root(ExtLinkDATFilename);
  if(pf==NULL) StopFatalError(10003,"ExtLink: Create error. [%s]\n",ExtLinkDATFilename);
  
  FAT2_fwrite(pbody,1,bodysize,pf);
  FAT2_fclose(pf);

  if(pbody!=NULL){
    safefree(&MM_Temp,pbody); pbody=NULL;
  }
  
  SetNextProc(ENP_BootROM,EPFE_None);
}

void BootROM_SetInfo(const UnicodeChar *pPathUnicode,const UnicodeChar *pFilenameUnicode,bool RequestBackupSave)
{
  bool opened=LaunchState_isOpened();
  
  if(opened==false) LaunchState_Open();
  const UnicodeChar *pFullPathUnicode=FullPathUnicodeFromSplitItem(pPathUnicode,pFilenameUnicode);
  LaunchState_Add(ELST_NDS,pFullPathUnicode);
  pLaunchState->LastTab=ELST_NDS;
  LaunchState_Save();
  if(opened==false) LaunchState_Close();
  
  if(ProcState.System.AutoLastState==true){
    ProcState.System.LastState=ELS_Launch;
    ProcState_RequestSave=true;
  }
  
  BootROM_SetInfo_NoLaunch(pPathUnicode,pFilenameUnicode,RequestBackupSave);
}

// ------------------------------------

#include "mediatype.h"

bool BootROM_isExistsSoftResetToFirmware(void)
{
  TBootROMInfo *pbri=&BootROMInfo;
  
  snprintf(pbri->FullPathAlias,256,ResetMSEPath "/%s.nds",DIMediaID);
  UnicodeChar ufn[256];
  StrConvert_Ank2Unicode(pbri->FullPathAlias,ufn);
  
  pbri->FullPathAlias[0]=0;
  
  return(FullPath_FileExistsUnicode(ufn));
}

void BootROM_SoftResetToFirmware(void)
{
  TBootROMInfo *pbri=&BootROMInfo;
  
  _consolePrintf("Request soft reset. [%s] %s\n",DIMediaID,DIMediaName);
  
  snprintf(pbri->FullPathAlias,256,ResetMSEPath "/%s.nds",DIMediaID);
  UnicodeChar ufn[256];
  StrConvert_Ank2Unicode(pbri->FullPathAlias,ufn);
  
  if(FullPath_FileExistsUnicode(ufn)==false){
    _consolePrintf("Not found soft reset. [%s]\n",pbri->FullPathAlias);
    pbri->FullPathAlias[0]=0;
    pbri->Execute=false;
    return;
  }
  
  StrCopy(ConvertFullPath_Unicode2Alias(ufn),pbri->FullPathAlias);
  pbri->Execute=true;
}

// ------------------------------------
bool BootROM_CheckNDSHomeBrew(const char *pFilename)
{
    u8 header[192];
    {
        FAT_FILE *pf=FAT2_fopen_AliasForRead(pFilename);
        if(pf==NULL) StopFatalError(10004,"Can not open NDS file. [%s]\n",pFilename);
        FAT2_fread(header,1,192,pf);
        FAT2_fclose(pf);
    }
      
    char ID[5];
    ID[0]=header[12+0];
    ID[1]=header[12+1];
    ID[2]=header[12+2];
    ID[3]=header[12+3];
    ID[4]=0;
      
    //_consolePrintf("Detected ROMID: %s\n",ID);
      
    char ID2[5];
    ID2[0]=header[172+0];
    ID2[1]=header[172+1];
    ID2[2]=header[172+2];
    ID2[3]=header[172+3];
    ID2[4]=0;
        
    bool homebrew=false;
      
    if(strcmp("####",ID)==0) homebrew=true;
    if(strcmp("PASS",ID)==0) homebrew=true;
    if(strcmp("PASS",ID2)==0) homebrew=true;
    if((ID[0]==0x3d)&&(ID[1]==0x84)&&(ID[2]==0x82)&&(ID[3]==0x0a)) homebrew=true;
      
    return(homebrew);
}

void BootROM_Execute(void)
{
    BootROMInfo.Execute=true;
}

