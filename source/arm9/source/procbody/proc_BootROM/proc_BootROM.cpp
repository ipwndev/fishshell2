
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "lang.h"
#include "skin.h"
#include "../../ipc6.h"

#include "glib.h"
#include "rect.h"

#include "fat2.h"
#include "shell.h"
#include "sndeff.h"
#include "splash.h"
#include "procstate.h"
#include "strpcm.h"
#include "BootROM.h"
// -----------------------------

static void CB_Start(void)
{
  u16 TextColor=RGB15(31,31,31)|BIT15;
  
  {
    CglCanvas *pCanvas=pScreenMain->pViewCanvas;
    pCanvas->SetCglFont(pCglFontDefault);
    pCanvas->SetFontTextColor(TextColor);
  }
  
  {
    CglCanvas *pCanvas=pScreenSub->pCanvas;
    pCanvas->SetCglFont(pCglFontDefault);
    pCanvas->SetFontTextColor(TextColor);
  }
  
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  BootROM_Execute();
  
  SetNextProc(ENP_FileList,EPFE_CrossFade); // for dummy.
}

static void CB_End(void)
{
}

void ProcBootROM_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
}

