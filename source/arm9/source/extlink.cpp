
#pragma Ospace

#include <nds.h>

#include "_console.h"
#include "_const.h"
#include "maindef.h"
#include "unicode.h"
#include "strtool.h"
#include "shell.h"
#include "memtool.h"
#include "lang.h"
#include "dll.h"

#include "arm9tcm.h"

#include "splash.h"

#include "extlink.h"

typedef struct {
  u32 Ext32;
  UnicodeChar *pNDSFullPathFilenameUnicode;
} TExtLink;

DATA_IN_AfterSystem static u32 ExtLinkCount;
DATA_IN_AfterSystem static TExtLink *pExtLink;

// ------------------------------------

DATA_IN_IWRAM_MainPass static void DataFileClear(void)
{
  FAT_FILE *pf=Shell_FAT_fopenwrite_Root(ExtLinkDATFilename);
  if(pf==NULL) StopFatalError(10401,"ExtLink: Create error. [%s]\n",ExtLinkDATFilename);
  
  u32 dummy=0;
  FAT2_fwrite(&dummy,1,4,pf);
  FAT2_fclose(pf);
}

DATA_IN_IWRAM_MainPass static u32 GetExt32(const UnicodeChar *pufn)
{
  UnicodeChar ufn[256];
  Unicode_Copy(ufn,pufn);
  
  {
    UnicodeChar *ptmp=ufn;
    while(*ptmp!=0){
      u32 ch=*ptmp;
      if((0x61<=ch)&&(ch<=0x7a)) *ptmp=ch-0x20;
      ptmp++;
    }
  }
  
  u32 ufnlen=Unicode_GetLength(ufn);
  if(ufnlen<4) return(0);
  if((ufn[ufnlen-4]!='.')||(ufn[ufnlen-3]!='N')||(ufn[ufnlen-2]!='D')||(ufn[ufnlen-1]!='S')) return(0);
  
  u32 Ext32=0;
  {
    const UnicodeChar *ptmp=ufn;
    while((*ptmp!=0)&&(*ptmp!='.')){
      u32 ch=*ptmp++;
      if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
      Ext32=(Ext32<<8)|ch;
    }
  }
  
  return(Ext32);
}

DATA_IN_IWRAM_MainPass void ExtLink_Init(void)
{
  DataFileClear();
  
  isExistsTextEditor=false;
  
  UnicodeChar BasePathUnicode[256];
  StrConvert_Ank2Unicode(ExtLinkPath,BasePathUnicode);
  const char *pBasePathAlias=ConvertFull_Unicode2Alias(BasePathUnicode,NULL);
  
  if(FAT2_chdir_Alias(pBasePathAlias)==false) StopFatalError(10402,"ExtLink_Init: Not found folder. [%s]\n",pBasePathAlias);
  
  ExtLinkCount=0;
  pExtLink=NULL;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
        Splash_Update();
      if(FAT_FileType==FT_FILE){
        const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
        if(pufn==NULL){
          _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
          }else{
          u32 Ext32=GetExt32(pufn);
          if(Ext32!=0){
            ExtLinkCount++;
          }
        }
      }
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  pExtLink=(TExtLink*)safemalloc_chkmem(&MM_System,sizeof(TExtLink)*ExtLinkCount);
  ExtLinkCount=0;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
        Splash_Update();
      if(FAT_FileType==FT_FILE){
        const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
        if(pufn==NULL){
          _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
          }else{
          u32 Ext32=GetExt32(pufn);
          if(Ext32!=0){
            TExtLink *pel=&pExtLink[ExtLinkCount];
            pel->Ext32=Ext32;
            UnicodeChar ufn[256]={0,};
            Unicode_Copy(ufn,BasePathUnicode);
            static const UnicodeChar su[2]={(UnicodeChar)'/',0};
            Unicode_Add(ufn,su);
            Unicode_Add(ufn,pufn);
            pel->pNDSFullPathFilenameUnicode=Unicode_AllocateCopy(&MM_System,ufn);
            _consolePrintf("Found ExtLink file. (%d) [%s]\n",ExtLinkCount,pafn);
            // _consolePrintf("Debug: %s\n",StrConvert_Unicode2Ank_Test(pel->pNDSFullPathFilenameUnicode));
            ExtLinkCount++;
            if(Ext32==MakeExt32(0,'_','T','E')) isExistsTextEditor=true;
          }
        }
      }
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
}

void ExtLink_Free(void)
{
  for(u32 idx=0;idx<ExtLinkCount;idx++){
    TExtLink *pel=&pExtLink[idx];
    pel->Ext32=0;
    if(pel->pNDSFullPathFilenameUnicode!=NULL){
      safefree(&MM_System,pel->pNDSFullPathFilenameUnicode); pel->pNDSFullPathFilenameUnicode=NULL;
    }
  }
  ExtLinkCount=0;
  
  if(pExtLink!=NULL){
    safefree(&MM_System,pExtLink); pExtLink=NULL;
  }
}

u32 ExtLink_GetTargetIndex(u32 Ext32)
{
  for(u32 idx=0;idx<ExtLinkCount;idx++){
    if(Ext32==pExtLink[idx].Ext32) return(idx);
  }
  
  return((u32)-1);
}

const UnicodeChar* ExtLink_GetNDSFullPathFilenameUnicode(u32 idx)
{
  if(ExtLinkCount<=idx) StopFatalError(10403,"ExtLink_GetNDSFullPathFilenameUnicode index overflow. %d/%d\n",idx,ExtLinkCount);
  
  return(pExtLink[idx].pNDSFullPathFilenameUnicode);  
}

