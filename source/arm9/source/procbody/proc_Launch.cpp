
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"

#include "procstate.h"
#include "launchstate.h"
#include "datetime.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"

#include "skin.h"
#include "sndeff.h"
#include "lang.h"
#include "rect.h"
#include "dll.h"
#include "strtool.h"
#include "BootROM.h"
#include "extlink.h"

#include "NDSROMIcon.h"

DATA_IN_IWRAM_Launch static s32 SelectFileIndex;

DATA_IN_IWRAM_Launch static CglCanvas *pPageCanvas,*pBackCanvas;

DATA_IN_IWRAM_Launch static TLaunchState_Tab *pCurrentTab;

// -----------------------------

typedef struct {
  UnicodeChar *pFilenameUnicode;
  u32 FileSize;
  TFAT2_TIME CreationTime,LastWriteTime;
} TCurrentFile_Body;

static void CurrentFile_Body_Init(TCurrentFile_Body *pbody)
{
  pbody->pFilenameUnicode=NULL;
  pbody->FileSize=0;
  
  pbody->CreationTime.Year=0;
  pbody->CreationTime.Month=0;
  pbody->CreationTime.Day=0;
  pbody->CreationTime.Hour=0;
  pbody->CreationTime.Minuts=0;
  pbody->CreationTime.Second=0;
  
  pbody->LastWriteTime.Year=0;
  pbody->LastWriteTime.Month=0;
  pbody->LastWriteTime.Day=0;
  pbody->LastWriteTime.Hour=0;
  pbody->LastWriteTime.Minuts=0;
  pbody->LastWriteTime.Second=0;
}

typedef struct {
  UnicodeChar *pPathUnicode;
  u32 Ext32;
  TCurrentFile_Body BaseFileBody;
  bool SaveFileExists;
  TCurrentFile_Body SaveFileBody;
  TNDSROMIcon *pNDSROMIcon;
  CglTGF *pIcon;
  u32 FilenameWidth;
} TCurrentFile;

DATA_IN_IWRAM_Launch static s32 CurrentFilesCount;
DATA_IN_IWRAM_Launch static TCurrentFile *pCurrentFiles;

static void CurrentFiles_Free(void)
{
  if(pCurrentFiles!=NULL){
    for(s32 idx=0;idx<CurrentFilesCount;idx++){
      TCurrentFile *pfile=&pCurrentFiles[idx];
      if(pfile->pPathUnicode!=NULL){
        safefree(&MM_Process,pfile->pPathUnicode); pfile->pPathUnicode=NULL;
      }
      if(pfile->BaseFileBody.pFilenameUnicode!=NULL){
        safefree(&MM_Process,pfile->BaseFileBody.pFilenameUnicode); pfile->BaseFileBody.pFilenameUnicode=NULL;
      }
      if(pfile->SaveFileBody.pFilenameUnicode!=NULL){
        safefree(&MM_Process,pfile->SaveFileBody.pFilenameUnicode); pfile->SaveFileBody.pFilenameUnicode=NULL;
      }
      if(pfile->pNDSROMIcon!=NULL){
        safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
      }
      pfile->pIcon=NULL;
    }
    safefree(&MM_Process,pCurrentFiles); pCurrentFiles=NULL;
  }
  
  CurrentFilesCount=0;
}

static void GetNDSIcon(TCurrentFile *pfile)
{
  pfile->pNDSROMIcon=(TNDSROMIcon*)safemalloc_chkmem(&MM_Process,sizeof(TNDSROMIcon));
  
  if(pfile->Ext32==MakeExt32(0,'N','D','S')){
    const char *pfullalias=ConvertFull_Unicode2Alias(pfile->pPathUnicode,pfile->BaseFileBody.pFilenameUnicode);
    if(NDSROMIcon_Get(pfullalias,pfile->pNDSROMIcon,NULL,NULL,NULL)==false){
      safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
      pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    }
    return;
  }
  
  u32 extlinkidx=ExtLink_GetTargetIndex(pfile->Ext32);
  
  if(extlinkidx==(u32)-1){
    safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
    pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    return;
  }
  
  const char *pAlias=ConvertFullPath_Unicode2Alias(ExtLink_GetNDSFullPathFilenameUnicode(extlinkidx));
  
  if(pAlias==NULL){
    _consolePrint("ExtLink: Not found extlink nds file.\n");
    safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
    pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    return;
  }
  
  if(NDSROMIcon_Get(pAlias,pfile->pNDSROMIcon,NULL,NULL,NULL)==false){
    safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
    pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
  }
}

static void CurrentFiles_AddFilesFromTab(void)
{
  CurrentFiles_Free();
  
  const char *pBasePathAlias=ConvertFull_Unicode2Alias(ProcState.FileList.CurrentPathUnicode,NULL);
  
  CurrentFilesCount=pCurrentTab->FilesCount;
  
  if(CurrentFilesCount==0) return;
  
  pCurrentFiles=(TCurrentFile*)safemalloc_chkmem(&MM_Process,sizeof(TCurrentFile)*CurrentFilesCount);
  
  for(s32 idx=0;idx<CurrentFilesCount;idx++){
    TCurrentFile *pfile=&pCurrentFiles[idx];
    pfile->pPathUnicode=NULL;
    CurrentFile_Body_Init(&pfile->BaseFileBody);
    pfile->SaveFileExists=false;
    CurrentFile_Body_Init(&pfile->SaveFileBody);
    pfile->pNDSROMIcon=NULL;
    pfile->pIcon=NULL;
    pfile->FilenameWidth=0;
  }
  
  CurrentFilesCount=0;
  
  for(s32 idx=0;idx<pCurrentTab->FilesCount;idx++){
    UnicodeChar PathUnicode[MaxFilenameLength];
    UnicodeChar FilenameUnicode[MaxFilenameLength];
    SplitItemFromFullPathUnicode(pCurrentTab->FullPathUnicode[idx],PathUnicode,FilenameUnicode);
    
    if(FileExistsUnicode(PathUnicode,FilenameUnicode)==true){
      TCurrentFile *pfile=&pCurrentFiles[CurrentFilesCount];
      
      {
        const UnicodeChar *ptmp=FilenameUnicode;
        u32 Ext32=0;
        while(*ptmp!=0){
          u32 ch=*ptmp++;
          if(ch==(u32)'.'){
            Ext32=0;
            }else{
            if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
            Ext32=(Ext32<<8)|ch;
          }
        }
        pfile->Ext32=Ext32;
      }
      
      pfile->pPathUnicode=Unicode_AllocateCopy(&MM_Process,PathUnicode);
      pfile->BaseFileBody.pFilenameUnicode=Unicode_AllocateCopy(&MM_Process,FilenameUnicode);
      pfile->BaseFileBody.FileSize=FAT2_CurEntry_GetFileSize();
      pfile->BaseFileBody.CreationTime=FAT2_GetFileCreationTime();
      pfile->BaseFileBody.LastWriteTime=FAT2_GetFileLastWriteTime();
      
      pfile->SaveFileBody.pFilenameUnicode=Unicode_AllocateCopy(&MM_Process,pfile->BaseFileBody.pFilenameUnicode);
      u32 fnlen=Unicode_GetLength(pfile->SaveFileBody.pFilenameUnicode);
      if(4<fnlen){
        pfile->SaveFileBody.pFilenameUnicode[fnlen-3]='s';
        pfile->SaveFileBody.pFilenameUnicode[fnlen-2]='a';
        pfile->SaveFileBody.pFilenameUnicode[fnlen-1]='v';
        if(FileExistsUnicode(pfile->pPathUnicode,pfile->SaveFileBody.pFilenameUnicode)==true){
          pfile->SaveFileExists=true;
          pfile->SaveFileBody.FileSize=FAT2_CurEntry_GetFileSize();
          pfile->SaveFileBody.CreationTime=FAT2_GetFileCreationTime();
          pfile->SaveFileBody.LastWriteTime=FAT2_GetFileLastWriteTime();
        }
      }
    
      if(pfile->SaveFileExists==false){
        if(pfile->SaveFileBody.pFilenameUnicode!=NULL){
          safefree(&MM_Process,pfile->SaveFileBody.pFilenameUnicode); pfile->SaveFileBody.pFilenameUnicode=NULL;
        }
      }
      
      CurrentFilesCount++;
    }
  }
  
  _consolePrint("Load icons.\n");
  for(s32 idx=0;idx<CurrentFilesCount;idx++){
    TCurrentFile *pfile=&pCurrentFiles[idx];
    GetNDSIcon(pfile);
  }
  
  CglCanvas *pbm=new CglCanvas(&MM_Temp,NULL,1,1,pf15bit);
  pbm->SetCglFont(pCglFontDefault);
  
  for(s32 idx=0;idx<CurrentFilesCount;idx++){
    TCurrentFile *pfile=&pCurrentFiles[idx];
    pfile->FilenameWidth=pbm->GetTextWidthW(pfile->BaseFileBody.pFilenameUnicode);
  }
  
  if(pbm!=NULL){
    delete pbm; pbm=NULL;
  }
  
  _consolePrint("Current folder refreshed.\n");
}

// -----------------------------

static void Setting_Redraw(void);

#include "proc_Launch_LongTapState.h"

// -----------------------------

#define ItemWidth (232)
#define ItemHeight (33)

#define TabTitleHeight (24)
DATA_IN_IWRAM_Launch const TRect ListRect={(ScreenWidth-232)/2,1+TabTitleHeight,232,(ItemHeight*LaunchState_Tab_FilesCountMax)};

static void DrawPageCanvas(void)
{
  SelectFileIndex=-1;
  
  pCurrentTab=&pLaunchState->Tabs[pLaunchState->LastTab];
  CurrentFiles_AddFilesFromTab();
  
  CglCanvas *pCanvas=pPageCanvas;
  
  {
    CglB15 *pbg=Launch_GetSkin(ELS_BGBottom);
    pbg->pCanvas->BitBltFullBeta(pCanvas);
  }
  {
    CglTGF *ptgf=NULL;
    switch(pLaunchState->LastTab){
      case ELST_Launch: ptgf=LaunchAlpha_GetSkin(ELSA_Tab0_Launch); break;
      case ELST_NDS: ptgf=LaunchAlpha_GetSkin(ELSA_Tab1_NDS); break;
    }
    if(ptgf!=NULL){
      ptgf->BitBlt(pCanvas,0,0);
    }
  }
  
  pCanvas->SetCglFont(pCglFontDefault);
  pCanvas->SetFontTextColor(ColorTable.Launch.EmptyText);
  
  if(CurrentFilesCount==0){
    pCanvas->TextOutA(ListRect.x+16,ListRect.y+16,"file not registed.");
    return;
  }
  
  SelectFileIndex=0; 
}

static void DrawTabFileList(CglCanvas *pCanvas)
{
  pCanvas->SetCglFont(pCglFontDefault);
  pCanvas->SetFontTextColor(ColorTable.Launch.FileNameText);
  
  pPageCanvas->BitBltFullBeta(pCanvas);
  
  u32 TextHeight=glCanvasTextHeight;
  
  for(s32 idx=0;idx<CurrentFilesCount;idx++){
    TCurrentFile *pfile=&pCurrentFiles[idx];
  
    u32 x=ListRect.x;
    u32 y=ListRect.y+(ItemHeight*idx);
    u32 w=ListRect.w;
    
    CglTGF *ptgf;
    if(SelectFileIndex!=idx){
      ptgf=LaunchAlpha_GetSkin(ELSA_Item_ClearBG);
      }else{
      ptgf=LaunchAlpha_GetSkin(ELSA_Item_SelectBG);
    }
    ptgf->BitBlt(pCanvas,x,y-1);
    
    UnicodeChar *pPathUnicode=pfile->pPathUnicode;
    UnicodeChar *pFilenameUnicode=pfile->BaseFileBody.pFilenameUnicode;
    
    x+=NDSROMIconXMargin;
    w-=NDSROMIconXMargin;
    
    if(pfile->pNDSROMIcon!=NULL){
      NDSROMIcon_DrawIcon32(pfile->pNDSROMIcon,pCanvas,x,y);
      }else{
      DrawSkinAlpha(pfile->pIcon,pCanvas,x,y);
    }
    
    x+=NDSROMIcon32Width+NDSROMIconXMargin;
    w-=(NDSROMIcon32Width+NDSROMIconXMargin);
    
    
    if(pLaunchState->LastTab==ELST_Launch){
      UnicodeChar NoExtUnicode[MaxFilenameLength];
      Unicode_Copy(NoExtUnicode,pFilenameUnicode);
      u32 DotPos=0;
      u32 idx=0;
      while(1){
        UnicodeChar uc=pFilenameUnicode[idx];
        if(uc==0) break;
        if(uc==(UnicodeChar)'.') DotPos=idx;
        NoExtUnicode[idx]=pFilenameUnicode[idx];
        idx++;
      }
      NoExtUnicode[DotPos]=0;
	  while(1){
		  if(pCanvas->GetTextWidthW(NoExtUnicode)<=w) break;
		  u32 len=Unicode_GetLength(NoExtUnicode);
		  NoExtUnicode[len-1]=0;
	  }
      pCanvas->TextOutW(x,y+((ItemHeight-TextHeight)/2),NoExtUnicode);
      }else{
    	  UnicodeChar TextOutUnicode[MaxFilenameLength];
    	  Unicode_Copy(TextOutUnicode,pPathUnicode);
    	  while(1){
    		  if(pCanvas->GetTextWidthW(TextOutUnicode)<=w) break;
    		  u32 len=Unicode_GetLength(TextOutUnicode);
    		  TextOutUnicode[len-1]=0;
    	  }
      pCanvas->TextOutW(x,y+(TextHeight*0)+1,TextOutUnicode);
      		Unicode_Copy(TextOutUnicode,pFilenameUnicode);
          	  while(1){
          		  if(pCanvas->GetTextWidthW(TextOutUnicode)<=w) break;
          		  u32 len=Unicode_GetLength(TextOutUnicode);
          		  TextOutUnicode[len-1]=0;
          	  }
      pCanvas->TextOutW(x,y+(TextHeight*1)+1,TextOutUnicode);
    }
  }
}

// -----------------------------

static void DrawSubScreen(void)
{
  CglCanvas *pCanvas=new CglCanvas(&MM_Temp,NULL,ScreenWidth,ScreenHeight,pf15bit);
  
  if(pCanvas==NULL) StopFatalError(16303,"DrawSubScreen create canvas failed.\n");
  
  pCanvas->SetCglFont(pCglFontDefault);
  
  {
    CglB15 *pbg=Launch_GetSkin(ELS_BGTop);
    pbg->pCanvas->BitBltFullBeta(pCanvas);
    CglTGF *ptgf=LaunchAlpha_GetSkin(ELSA_FileInfoFrame);
    ptgf->BitBlt(pCanvas,0,0);
  }
  
  if((SelectFileIndex!=-1)&&(SelectFileIndex<CurrentFilesCount)){
    TCurrentFile *pfile=&pCurrentFiles[SelectFileIndex];
    
    char str[256];
    TFAT2_TIME t;
    
    u32 x=8;
    u32 ih=13;

    pCanvas->SetFontTextColor(ColorTable.Launch.NDSFileInfoText);

    UnicodeChar TextOutUnicode[MaxFilenameLength];
    u32 w=ScreenWidth-8*2;
    {
      u32 y=16;
      snprintf(str,256,Lang_GetUTF8("Launch_FileInfo"));
      pCanvas->TextOutUTF8(x,y,str);
      y+=ih+3;
      Unicode_Copy(TextOutUnicode,FullPathUnicodeFromSplitItem(pfile->pPathUnicode,pfile->BaseFileBody.pFilenameUnicode));
      while(1){
    	  if(pCanvas->GetTextWidthW(TextOutUnicode)<=w) break;
    	  u32 len=Unicode_GetLength(TextOutUnicode);
    	  TextOutUnicode[len-1]=0;
      }
      pCanvas->TextOutW(x,y,TextOutUnicode);
      y+=ih+1;
      snprintf(str,256,Lang_GetUTF8("Launch_Size"),pfile->BaseFileBody.FileSize,((pfile->BaseFileBody.FileSize*8)+(1024*1024)-1)/(1024*1024));
      pCanvas->TextOutUTF8(x,y,str);
      y+=ih;
      t=pfile->BaseFileBody.CreationTime;
      snprintf(str,256,Lang_GetUTF8("Launch_Create"),Date_GetDateStr_FAT2_TIME(&t),t.Hour,t.Minuts,t.Second);
      pCanvas->TextOutUTF8(x,y,str);
      y+=ih;
      t=pfile->BaseFileBody.LastWriteTime;
      snprintf(str,256,Lang_GetUTF8("Launch_Update"),Date_GetDateStr_FAT2_TIME(&t),t.Hour,t.Minuts,t.Second);
      pCanvas->TextOutUTF8(x,y,str);
      y+=ih;
    }
    
    pCanvas->SetFontTextColor(ColorTable.Launch.SaveFileInfoText);
    
    {
      u32 y=112;
      if(pfile->SaveFileExists==false){
        snprintf(str,256,Lang_GetUTF8("Launch_NotFoundRelationalFile"));
        pCanvas->TextOutUTF8(x,y,str);
        }else{
        snprintf(str,256,Lang_GetUTF8("Launch_FileInfo_Relational"));
        pCanvas->TextOutUTF8(x,y,str);
        y+=ih+3;
        Unicode_Copy(TextOutUnicode,FullPathUnicodeFromSplitItem(pfile->pPathUnicode,pfile->SaveFileBody.pFilenameUnicode));
        while(1){
        	if(pCanvas->GetTextWidthW(TextOutUnicode)<=w) break;
        	u32 len=Unicode_GetLength(TextOutUnicode);
        	TextOutUnicode[len-1]=0;
        }
        pCanvas->TextOutW(x,y,TextOutUnicode);
        y+=ih+1;
        snprintf(str,256,Lang_GetUTF8("Launch_Size"),pfile->SaveFileBody.FileSize,((pfile->SaveFileBody.FileSize*8)+(1024)-1)/(1024));
        pCanvas->TextOutUTF8(x,y,str);
        y+=ih;
        t=pfile->SaveFileBody.CreationTime;
        snprintf(str,256,Lang_GetUTF8("Launch_Create"),Date_GetDateStr_FAT2_TIME(&t),t.Hour,t.Minuts,t.Second);
        pCanvas->TextOutUTF8(x,y,str);
        y+=ih;
        t=pfile->SaveFileBody.LastWriteTime;
        snprintf(str,256,Lang_GetUTF8("Launch_Update"),Date_GetDateStr_FAT2_TIME(&t),t.Hour,t.Minuts,t.Second);
        pCanvas->TextOutUTF8(x,y,str);
        y+=ih;
      }
    }
  }
  
  pCanvas->BitBltFullBeta(pScreenSub->pCanvas);
  
  if(pCanvas!=NULL){
    delete pCanvas; pCanvas=NULL;
  }
  
}

// -----------------------------

static void Setting_Redraw(void)
{
  DrawTabFileList(pBackCanvas);
  
  LongTapState_DrawIcon(pBackCanvas);
  
  pBackCanvas->BitBltFullBeta(pScreenMain->pBackCanvas);
  DrawSubScreen();
  ScreenMain_Flip_ProcFadeEffect();
}

// -----------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
  if((Keys&(KEY_B|KEY_SELECT))!=0){
    ProcState.System.LastState=ELS_FileList;
    ProcState_RequestSave=true;
    SetNextProc(ENP_FileList,EPFE_CrossFade);
    Sound_Start(WAVFN_Click);
  }
  
  if((Keys&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))!=0){
    if(SelectFileIndex!=-1){
      s32 v=0;
      if((Keys&KEY_UP)!=0) v=-1;
      if((Keys&KEY_DOWN)!=0) v=1;
      if((Keys&KEY_LEFT)!=0) v=-LaunchState_Tab_FilesCountMax;
      if((Keys&KEY_RIGHT)!=0) v=LaunchState_Tab_FilesCountMax;
      SelectFileIndex+=v;
      if(SelectFileIndex<0) SelectFileIndex=0;
      if(CurrentFilesCount<=SelectFileIndex) SelectFileIndex=CurrentFilesCount-1;
      Setting_Redraw();
    }
  }

  if((Keys&KEY_A)!=0){
    if(SelectFileIndex!=-1){
      Sound_Start(WAVFN_Click);
      TCurrentFile *pfile=&pCurrentFiles[SelectFileIndex];
      BootROM_SetInfo(pfile->pPathUnicode,pfile->BaseFileBody.pFilenameUnicode,true);
    }
  }
  
  if((Keys&(KEY_L|KEY_R))!=0){
    bool refresh=false;
    if((Keys&KEY_L)!=0){
      if(pLaunchState->LastTab!=ELST_Launch){
        pLaunchState->LastTab=ELST_Launch;
        refresh=true;
      }
    }
    if((Keys&KEY_R)!=0){
      if(pLaunchState->LastTab!=ELST_NDS){
        pLaunchState->LastTab=ELST_NDS;
        refresh=true;
      }
    }
    if(refresh==true){
      Sound_Start(WAVFN_Click);
      DrawPageCanvas();
      SetProcFadeEffect(EPFE_CrossFade);
      Setting_Redraw();
      return;
    }
  }
}

enum EMouseMode {EMM_Idle,EMM_Tab,EMM_File};
DATA_IN_IWRAM_Launch static EMouseMode MouseMode;
DATA_IN_IWRAM_Launch static s32 File_lx,File_ly;

static void CB_MouseDown(s32 x,s32 y)
{
  MouseMode=EMM_Idle;
  
  if(y<ListRect.y){
    Sound_Start(WAVFN_Click);
    u32 tx=ListRect.x+2+48;
    if(x<tx){
      ProcState.System.LastState=ELS_FileList;
      ProcState_RequestSave=true;
      SetNextProc(ENP_FileList,EPFE_CrossFade);
      return;
    }
    tx+=56;
    if(x<tx){
      pLaunchState->LastTab=ELST_Launch;
      DrawPageCanvas();
      MouseMode=EMM_Tab;
      SetProcFadeEffect(EPFE_CrossFade);
      Setting_Redraw();
      return;
    }
    tx+=134;
    if(x<tx){
      pLaunchState->LastTab=ELST_NDS;
      DrawPageCanvas();
      MouseMode=EMM_Tab;
      SetProcFadeEffect(EPFE_CrossFade);
      Setting_Redraw();
      return;
    }
    ProcState.System.LastState=ELS_FileList;
    ProcState_RequestSave=true;
    SetNextProc(ENP_FileList,EPFE_CrossFade);
    return;
  }
  
  if((ListRect.y<y)&&(y<=(ListRect.y+ListRect.h))){
    File_lx=x;
    File_ly=y;
    y-=ListRect.y;
    SelectFileIndex=y/ItemHeight;
    if(CurrentFilesCount<=SelectFileIndex) SelectFileIndex=-1;
    if(SelectFileIndex!=-1){
      LongTapState_Start(File_lx,File_ly);
      MouseMode=EMM_File;
    }
    Setting_Redraw();
    return;
  }
}

static void CB_MouseMove(s32 x,s32 y)
{
  switch(MouseMode){
    case EMM_Idle: {
    } break;
    case EMM_Tab: {
    } break;
    case EMM_File: {
      if( (8<=abs(File_lx-x)) || (8<=abs(File_ly-y)) ) LongTapState_ExecStop();
      u32 LastSelectFileIndex=SelectFileIndex;
      SelectFileIndex=-1;
      if(ListRect.y<y){
        y-=ListRect.y;
        SelectFileIndex=y/ItemHeight;
        if(CurrentFilesCount<=SelectFileIndex) SelectFileIndex=-1;
        if(LastSelectFileIndex!=SelectFileIndex) LongTapState_ExecStop();
      }
      Setting_Redraw();
    } break;
  }
}

static void CB_MouseUp(s32 x,s32 y)
{
  EMouseMode _MouseMode=MouseMode;
  MouseMode=EMM_Idle;
  
  switch(_MouseMode){
    case EMM_Idle: {
    } break;
    case EMM_Tab: {
    } break;
    case EMM_File: {
      TCurrentFile *pfile=&pCurrentFiles[SelectFileIndex];
      if(SelectFileIndex!=-1){
        if(LongTapState_GetProceed()==true){
          Sound_Start(WAVFN_Click);
          BootROM_SetInfo(pfile->pPathUnicode,pfile->BaseFileBody.pFilenameUnicode,true);
          }else{
          if((abs(File_lx-x)<4)&&(abs(File_ly-y)<4)){
            Sound_Start(WAVFN_Click);
            BootROM_SetInfo(pfile->pPathUnicode,pfile->BaseFileBody.pFilenameUnicode,true);
          }
        }
      }
      LongTapState_ExecStop();
    } break;
  }
}

static void MouseIdle(u32 VsyncCount)
{
  switch(MouseMode){
    case EMM_Idle: {
    } break;
    case EMM_Tab: {
    } break;
    case EMM_File: {
      LongTapState_AddVsync(VsyncCount);
    } break;
  }
}

static void CB_Start(void)
{
  Sound_Start(WAVFN_Open);

  LaunchState_Open();
  
  SelectFileIndex=0;
  
  pCurrentTab=NULL;
  
  CurrentFilesCount=0;
  pCurrentFiles=NULL;
  
  LongTapState_Init();
  
  pPageCanvas=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
  DrawPageCanvas();
  
  pBackCanvas=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
  
  MouseMode=EMM_Idle;
  
  Setting_Redraw();
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  MouseIdle(VsyncCount);
}

static void CB_End(void)
{
  LaunchState_Save();
  LaunchState_Close();
  
  CurrentFiles_Free();
  LongTapState_Free();
  
  if(pPageCanvas!=NULL){
    delete pPageCanvas; pPageCanvas=NULL;
  }
  
  if(pBackCanvas!=NULL){
    delete pBackCanvas; pBackCanvas=NULL;
  }
}

void ProcLaunch_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
}

