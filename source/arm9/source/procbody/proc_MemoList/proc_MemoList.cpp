
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "ErrorDialog.h"
#include "../../ipc6.h"

#include "procstate.h"
#include "datetime.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "strtool.h"

#include "skin.h"
#include "unicode.h"
#include "sndeff.h"
#include "lang.h"
#include "rect.h"
#include "msgwin.h"

#include "OverlayManager.h"

// ------------------------------------------------------

static void CB_MWin_ProgressShow(const char *pTitleStr,s32 Max)
{
  msgwin_Draw(pTitleStr,"",0,0);
}

static void CB_MWin_ProgressSetPos(const char *pTitleStr,s32 pos, s32 max)
{
  char str[64];
  snprintf(str,64,"%d/%d %d%%",pos,max,pos*100/max);
  msgwin_Draw(pTitleStr,str,pos,max);
}

static void CB_MWin_ProgressDraw(const char *pstr0,const char *pstr1,s32 pos,s32 max)
{
  msgwin_Draw(pstr0,pstr1,pos,max);
}

static void CB_MWin_ProgressHide(void)
{
  msgwin_Clear();
}

// ------------------------------------------------------

void ProcMemoList_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=NULL;
  pCallBack->VsyncUpdate=NULL;
  pCallBack->End=NULL;
  pCallBack->KeyPress=NULL;
  pCallBack->MouseDown=NULL;
  pCallBack->MouseMove=NULL;
  pCallBack->MouseUp=NULL;
  
  pCallBack->MWin_ProgressShow=CB_MWin_ProgressShow;
  pCallBack->MWin_ProgressSetPos=CB_MWin_ProgressSetPos;
  pCallBack->MWin_ProgressDraw=CB_MWin_ProgressDraw;
  pCallBack->MWin_ProgressHide=CB_MWin_ProgressHide;
}

