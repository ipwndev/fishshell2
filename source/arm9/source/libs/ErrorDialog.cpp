
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "shell.h"
#include "strtool.h"

#include "lang.h"
#include "ErrorDialog.h"
#include "glib.h"
#include "skin.h"

DATA_IN_AfterSystem static bool isExists;
DATA_IN_AfterSystem static EErrorCode ErrorCode;

DATA_IN_IWRAM_MainPass void ErrorDialog_Init(void)
{
  isExists=false;
  return;
  
  // for test
  //isExists=true;
  //ErrorCode=EEC_UnknownError;
}

void ErrorDialog_Clear(void)
{
  isExists=false;
}

bool ErrorDialog_isExists(void)
{
  return(isExists);
}

void ErrorDialog_Set(EErrorCode _ErrorCode)
{
  if(isExists==true) return;
  isExists=true;
  ErrorCode=_ErrorCode;
}

void ErrorDialog_Draw(CglCanvas *pcan)
{
  CglTGF* perrdlgbg=Skin_GetErrorDialogBG();
  if(perrdlgbg==NULL) StopFatalError(18801,"ErrorDialog_Draw: Can not load ErrorDialogBG.\n");
  
  u32 w=perrdlgbg->GetWidth(),h=perrdlgbg->GetHeight();
  u32 x=2,y=ScreenHeight-h-4;
  
  perrdlgbg->BitBlt(pcan,x,y);
  
  if(perrdlgbg!=NULL){
    delete perrdlgbg; perrdlgbg=NULL;
  }
  
  const char *perrmsg0=NULL,*perrmsg1=NULL,*perrmsg2=NULL;
  
  switch(ErrorCode){
/*
    case EEC_: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_");
      perrmsg1=Lang_GetUTF8("ERRMSG1_");
      perrmsg2=Lang_GetUTF8("ERRMSG2_");
    } break;
*/
    case EEC_MemoryOverflow_CanRecovery: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_MemoryOverflow_CanRecovery");
      perrmsg1=Lang_GetUTF8("ERRMSG1_MemoryOverflow_CanRecovery");
      perrmsg2=Lang_GetUTF8("ERRMSG2_MemoryOverflow_CanRecovery");
    } break;
    case EEC_NotSupportFileFormat: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_NotSupportFileFormat");
      perrmsg1=Lang_GetUTF8("ERRMSG1_NotSupportFileFormat");
      perrmsg2=Lang_GetUTF8("ERRMSG2_NotSupportFileFormat");
    } break;
    case EEC_ProgressiveJpeg: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_ProgressiveJpeg");
      perrmsg1=Lang_GetUTF8("ERRMSG1_ProgressiveJpeg");
      perrmsg2=Lang_GetUTF8("ERRMSG2_ProgressiveJpeg");
    } break;
    case EEC_Text0byte: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_Text0byte");
      perrmsg1=Lang_GetUTF8("ERRMSG1_Text0byte");
      perrmsg2=Lang_GetUTF8("ERRMSG2_Text0byte");
    } break;
    case EEC_OverflowLargeImage: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_OverflowLargeImage");
      perrmsg1=Lang_GetUTF8("ERRMSG1_OverflowLargeImage");
      perrmsg2=Lang_GetUTF8("ERRMSG2_OverflowLargeImage");
    } break;
    case EEC_NotFoundMusicFile: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_NotFoundMusicFile");
      perrmsg1=Lang_GetUTF8("ERRMSG1_NotFoundMusicFile");
      perrmsg2=Lang_GetUTF8("ERRMSG2_NotFoundMusicFile");
    } break;
    case EEC_UnknownError: default: {
      perrmsg0=Lang_GetUTF8("ERRMSG0_UnknownError");
      perrmsg1=Lang_GetUTF8("ERRMSG1_UnknownError");
      perrmsg2=Lang_GetUTF8("ERRMSG2_UnknownError");
    }
  }
  
  x+=8;
  y+=24+3;
  h=14;
  
  pcan->SetFontTextColor(ColorTable.ErrorDialog.MessageText);
  if(perrmsg0!=NULL) pcan->TextOutUTF8(x,y,perrmsg0);
  y+=h;
  if(perrmsg1!=NULL) pcan->TextOutUTF8(x,y,perrmsg1);
  y+=h;
  y+=8;
  if(perrmsg2!=NULL) pcan->TextOutUTF8(x,y,perrmsg2);
}

