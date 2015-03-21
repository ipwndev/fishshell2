
#include <nds.h>

#include <stdio.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"

#include "shell.h"
#include "glib.h"
#include "dll.h"
#include "procstate.h"

#include "NDSFiles.h"

#include "ThumbDPG.h"

#define DPG0ID (0x30475044)
#define DPG1ID (0x31475044)
#define DPG2ID (0x32475044)
#define DPG3ID (0x33475044)
#define DPG4ID (0x34475044)

#define DPGThumbnailImageID (0x304d4854)

DATA_IN_IWRAM_FileList static bool DPGThumbLoadFlag;
DATA_IN_IWRAM_FileList static FAT_FILE *pDPGThumbFile;
DATA_IN_IWRAM_FileList static u16 *pDPGThumbnail;

void InitDPGThumb(void)
{
  DPGThumbLoadFlag=false;
  pDPGThumbFile=NULL;
  pDPGThumbnail=NULL;
}

void FreeDPGThumb(void)
{
  if(pDPGThumbFile!=NULL){
    FAT2_fclose(pDPGThumbFile); pDPGThumbFile=NULL;
  }
  
  if(pDPGThumbnail!=NULL){
    safefree(&MM_Process,pDPGThumbnail); pDPGThumbnail=NULL;
  }
  
  DPGThumbLoadFlag=false;
}

static bool FileCheck_isDPGWithThumb(FAT_FILE *pf)
{
  FAT2_fseek(pf,0,SEEK_SET);
  
  int ID;
  FAT2_fread((u8*)&ID,1,4,pf);
  if((ID!=DPG0ID)&&(ID!=DPG1ID)&&(ID!=DPG2ID)&&(ID!=DPG3ID)&&(ID!=DPG4ID)) return(false);
  
  FAT2_fseek(pf,0x30,SEEK_SET);
  FAT2_fread((u8*)&ID,1,4,pf);
  if(ID!=DPGThumbnailImageID) return(false);
    
  return(true);
}

void ReloadDPGThumb(const UnicodeChar *pCurrentPathUnicode,s32 fileidx)
{
  FreeDPGThumb();
  
  TNDSFile *pndsf=NDSFiles_GetFileBody(fileidx);
  
  if(pndsf->FileType!=ENFFT_Video) return;
  
  {
    u32 Ext32=0;
    {
      const char *ptmp=pndsf->pFilenameAlias;
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
    
    if(Ext32!=MakeExt32(0,'D','P','G')) return;
  }
  
  const UnicodeChar *pfnw=pndsf->pFilenameUnicode;
  pDPGThumbFile=FAT2_fopen_AliasForRead(ConvertFull_Unicode2Alias(pCurrentPathUnicode,pfnw));
  
  if(FileCheck_isDPGWithThumb(pDPGThumbFile)==false){
    FreeDPGThumb();
    return;
  }
  
  if(pDPGThumbnail==NULL) pDPGThumbnail=(u16*)safemalloc_chkmem(&MM_Process,ScreenWidth*ScreenHeight*2);
  
  if(pDPGThumbnail!=NULL){
        FAT2_fseek(pDPGThumbFile,0x34,SEEK_SET);
        FAT2_fread(pDPGThumbnail,1,ScreenWidth*ScreenHeight*2,pDPGThumbFile);
  }else{
      FreeDPGThumb();
      return;
  }
  
  DPGThumbLoadFlag=true;
  
  if(VerboseDebugLog==true) _consolePrint("DPGThumb: Loaded.\n");
}

bool DrawDPGThumb(CglCanvas *pcan)
{
  if(DPGThumbLoadFlag==false) return(false);
  
  if(pDPGThumbnail==NULL) return(false);
  
  u32 canw=pcan->GetWidth();
  u32 canh=pcan->GetHeight();
  
  u16 *psrcbm=pDPGThumbnail;
  u16 *pdstbm=pcan->GetVRAMBuf();

  MemCopy32CPU(psrcbm,pdstbm,canw*canh*2);
  return(true);
}

