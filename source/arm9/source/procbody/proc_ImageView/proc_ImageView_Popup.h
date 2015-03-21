
DATA_IN_IWRAM_ImageView static u32 Popup_TimeoutVSync;

DATA_IN_IWRAM_ImageView static TRect PopupRect={16,16,0,16};

static void Popup_Init(void)
{
    Popup_TimeoutVSync=0;
}

static void Popup_Free(void)
{
    Popup_TimeoutVSync=0;
    pScreenMainOverlay->SetVisible_for_LeftTop128x64(false);
}

static void Popup_VsyncUpdate(u32 VsyncCount) // (u32)-1Ç≈èÌÇ…âBÇ∑
{
    if(Popup_TimeoutVSync==0) return;
  
    if(VsyncCount!=(u32)-1){
        Popup_TimeoutVSync+=VsyncCount;
        if(Popup_TimeoutVSync<(60*2)) return;
    }
  
    Popup_TimeoutVSync=0;
    //pScreenMainOverlay->SetVisible_for_LeftTop128x64(false);
    pScreenMainOverlay->pCanvas->SetColor(0);
    pScreenMainOverlay->pCanvas->FillFast(PopupRect.x,PopupRect.y,128,PopupRect.h);
}

static void Popup_Show(const char *pstr)
{
//  _consolePrintf("Popup_Show(%s);\n",pstr);
    
    if(pstr==NULL || strlen(pstr)==0) return;
    
    CglCanvas *pCanvas=pScreenMainOverlay->pCanvas;
  
    s32 tw=pCanvas->GetTextWidthUTF8(pstr);
    s32 th=glCanvasTextHeight;
      
    pCanvas->SetColor(0);
    pCanvas->FillFast(PopupRect.x,PopupRect.y,128,PopupRect.h);
  
    PopupRect.w=tw+3*2;
  
    TRect r=PopupRect;
  
    pCanvas->SetColor(ColorTable.FileList.PopupBG);
    pCanvas->FillFast(r.x,r.y,r.w,r.h);
    pCanvas->SetColor(ColorTable.FileList.PopupFrame);
    pCanvas->DrawBox(r.x,r.y,r.w,r.h);
  
    pCanvas->SetFontTextColor(ColorTable.FileList.PopupText);
  
    s32 tx=r.x+((r.w-tw)/2);
    s32 ty=r.y+((r.h-th)/2);
  
    pCanvas->TextOutUTF8(tx,ty,pstr);
  
    pScreenMainOverlay->SetVisible_for_LeftTop128x64(true);
  
    Popup_TimeoutVSync=1;
}

static void Popup_Show_BacklightLevel(void)
{
    char str[32];
    snprintf(str,32,Lang_GetUTF8("IV_Popup_BacklightLevel"),1+ProcState.System.BacklightLevel);
    Popup_Show(str);
}

static void Popup_Show_Zoom(s32 fix8)
{
    char str[32];
    snprintf(str,32,Lang_GetUTF8("IV_Popup_Zoom"),fix8*100/0x100);
    Popup_Show(str);
}

static void Popup_Show_MP3Cnt_Pause(void)
{
    if(PlayList_GetPause()==false){
        Popup_Show(Lang_GetUTF8("IV_Popup_Pause_Play"));
    }else{
        Popup_Show(Lang_GetUTF8("IV_Popup_Pause_Pause"));
    }
}

static void Popup_Show_MP3Cnt_Prev(void)
{
    Popup_Show(Lang_GetUTF8("IV_Popup_PreviousFile"));
}

static void Popup_Show_MP3Cnt_Next(void)
{
    Popup_Show(Lang_GetUTF8("IV_Popup_NextFile"));
}

static void Popup_Show_MP3Cnt_Seek(s32 val)
{
    char str[32];
    char valstr[32];
          
    if(val<0){
        snprintf(valstr,32,"-%d%%",-val);
    }else{
        snprintf(valstr,32,"+%d%%",val);
    }
        
    snprintf(str,32,Lang_GetUTF8("IV_Popup_Seek"),valstr);

    Popup_Show(str);
}

