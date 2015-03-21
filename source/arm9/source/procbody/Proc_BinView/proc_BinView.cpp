
#pragma Ospace

#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "../../ipc6.h"

#include "procstate.h"
#include "datetime.h"
#include "ErrorDialog.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "skin.h"

#include "inifile.h"

#include "msgwin.h"
#include "sndeff.h"
#include "lang.h"
#include "strpcm.h"
#include "dllsound.h"
#include "rect.h"
#include "resume.h"
#include "playlist.h"
#include "strtool.h"
#include "cfont.h"
#include "cctf.h"
#include "euc2unicode.h"

// -----------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
    
}

DATA_IN_IWRAM_BinView static bool isPressMouseButton;

static void CB_MouseDown(s32 x,s32 y)
{
    isPressMouseButton=true;
}

static void CB_MouseMove(s32 x,s32 y)
{
    if(isPressMouseButton==false) return;
}

static void CB_MouseUp(s32 x,s32 y)
{
    if(isPressMouseButton==false) return;
    isPressMouseButton=false;
  
}

// ------------------------------------------------------------

static void CB_Start(void)
{
    
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
    
}

static void CB_End(void)
{
	
}

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

void ProcBinView_SetCallBack(TCallBack *pCallBack)
{
    pCallBack->Start=CB_Start;
    pCallBack->VsyncUpdate=CB_VsyncUpdate;
    pCallBack->End=CB_End;
    pCallBack->KeyPress=CB_KeyPress;
    pCallBack->MouseDown=CB_MouseDown;
    pCallBack->MouseMove=CB_MouseMove;
    pCallBack->MouseUp=CB_MouseUp;
  
    pCallBack->MWin_ProgressShow=CB_MWin_ProgressShow;
    pCallBack->MWin_ProgressSetPos=CB_MWin_ProgressSetPos;
    pCallBack->MWin_ProgressDraw=CB_MWin_ProgressDraw;
    pCallBack->MWin_ProgressHide=CB_MWin_ProgressHide;
}

