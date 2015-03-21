
#include <nds.h>

#include "_console.h"

#include "resume.h"
#include "memtool.h"
#include "arm9tcm.h"
#include "splash.h"

#include "shell.h"
#include "disc_io.h"

extern DISC_INTERFACE* active_interface;

DATA_IN_AfterSystem static u32 ResumeDataSectorIndex=0;

#define FilenameLength (192)

typedef struct {
  u32 ID1;
  EResumeMode ResumeMode;
  UnicodeChar FullFilenameUnicode[FilenameLength];
  u32 pos;
  u32 ID2;
  u8 dummy[128];
} TResumeData;

DATA_IN_AfterSystem static TResumeData ResumeData;

static bool CheckResumeData(void)
{
  if(ResumeData.ID1==0) return(false);
  if(ResumeData.ID1!=ResumeData.ID2) return(false);
  return(true);
}

DATA_IN_IWRAM_MainPass void Resume_Load(void)
{
  ResumeDataSectorIndex=0;
  
  Splash_Update();
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Internal(ResumeFilename);
    if(pf!=NULL){
      if(FAT2_GetFileSize(pf)==512){
        if(pf->firstCluster!=0){
          ResumeDataSectorIndex=FAT2_ClustToSect(pf->firstCluster);
        }
      }
      FAT2_fclose(pf);
    }
  }
  
  Splash_Update();
  
  _consolePrintf("ResumeDataSectorIndex=%d.\n",ResumeDataSectorIndex);
  
  MemSet32CPU(0,&ResumeData,sizeof(TResumeData));
  
  if(ResumeDataSectorIndex!=0) active_interface->readSectors(ResumeDataSectorIndex,1,&ResumeData);
  
  Splash_Update();
  
  if(CheckResumeData()==false) Resume_Clear();
}

void Resume_Clear(void)
{
  MemSet8CPU(0,&ResumeData,sizeof(TResumeData));
  Resume_Save();
}

void Resume_Save(void)
{
//  _consolePrint("Resume_Save();\n");
  
  REG_IME=0;
  if(ResumeDataSectorIndex!=0) active_interface->writeSectors(ResumeDataSectorIndex,1,&ResumeData);
  REG_IME=1;
}

void Resume_SetResumeMode(EResumeMode rm)
{
//  _consolePrintf("Resume_SetResumeMode(%d);\n",rm);
  
  TResumeData *prd=&ResumeData;
  prd->ResumeMode=rm;
  prd->ID1++;
  prd->ID2=prd->ID1;
}

void Resume_SetFilename(const UnicodeChar *pFullFilenameUnicode)
{
//  _consolePrintf("Resume_SetFilename(...);\n");
  
  TResumeData *prd=&ResumeData;
  MemCopy16CPU(pFullFilenameUnicode,prd->FullFilenameUnicode,(FilenameLength-1)*2);
  prd->FullFilenameUnicode[FilenameLength-1]=0;
  prd->ID1++;
  prd->ID2=prd->ID1;
}

void Resume_SetPos(u32 pos)
{
//  _consolePrintf("Resume_SetPos(%d);\n",pos);
  
  TResumeData *prd=&ResumeData;
  prd->pos=pos;
  prd->ID1++;
  prd->ID2=prd->ID1;
}

EResumeMode Resume_GetResumeMode(void)
{
  if(CheckResumeData()==false) return(ERM_None);
  return(ResumeData.ResumeMode);
}

const UnicodeChar* Resume_GetFilename(void)
{
  if(CheckResumeData()==false) return(NULL);
  return(ResumeData.FullFilenameUnicode);
}

u32 Resume_GetPos(void)
{
  if(CheckResumeData()==false) return(0);
  return(ResumeData.pos);
}
