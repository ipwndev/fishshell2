
DATA_IN_IWRAM_DPGPlay static u32 Popup_TimeoutVSync;
DATA_IN_IWRAM_DPGPlay static bool Popup_Visible;
DATA_IN_IWRAM_DPGPlay static char Popup_Msg[64];

DATA_IN_IWRAM_DPGPlay static TRect PopupRect={12,4,0,16};

static void Popup_Init(void)
{
    Popup_TimeoutVSync=0;
    Popup_Visible=false;
}

static void Popup_Free(void)
{
    Popup_TimeoutVSync=0;
    Popup_Visible=false;
}

static void Popup_Draw(CglCanvas *pCanvas)
{
    if(Popup_Visible==false || strlen(Popup_Msg)==0) return;
  
    s32 tw=pCanvas->GetTextWidthUTF8(Popup_Msg);
    s32 th=glCanvasTextHeight;
        
    //pCanvas->SetColor(0);
    //pCanvas->FillFast(PopupRect.x,PopupRect.y,PopupRect.w,PopupRect.h);
          
    PopupRect.w=tw+3*2;
    
    TRect r=PopupRect;
    
    pCanvas->SetColor(ColorTable.FileList.PopupBG);
    pCanvas->FillFast(r.x,r.y,r.w,r.h);
    pCanvas->SetColor(ColorTable.FileList.PopupFrame);
    pCanvas->DrawBox(r.x,r.y,r.w,r.h);
    
    pCanvas->SetFontTextColor(ColorTable.FileList.PopupText);
    
    s32 tx=r.x+((r.w-tw)/2);
    s32 ty=r.y+((r.h-th)/2);
    
    pCanvas->TextOutUTF8(tx,ty,Popup_Msg);
}

static void Popup_VsyncUpdate(u32 VsyncCount) // (u32)-1Ç≈èÌÇ…âBÇ∑
{
    if(Popup_TimeoutVSync==0) return;
      
    if(VsyncCount!=(u32)-1){
        Popup_TimeoutVSync+=VsyncCount;
        if(Popup_TimeoutVSync<(60*2)) return;
    }
      
    Popup_TimeoutVSync=0;
    
    Popup_Visible=false;
    ScreenRedrawFlag=true;
}   

static void Popup_Show(const char *pstr)
{
//  _consolePrintf("Popup_Show(%s);\n",pstr);
  
    if(pstr==NULL || strlen(pstr)==0) return;
    
    strcpy(Popup_Msg,pstr);
    
    Popup_Visible=true;
    
    Popup_TimeoutVSync=1;
    
    ScreenRedrawFlag=true;
}

static void Popup_Show_Pause(void)
{
    if(IPC6->MP2PauseFlag==false){
        Popup_Show(Lang_GetUTF8("DV_Popup_Pause_Play"));
    }else{
        Popup_Show(Lang_GetUTF8("DV_Popup_Pause_Pause"));
    }
}

static void Popup_Show_Prev(void)
{
    Popup_Show(Lang_GetUTF8("DV_Popup_PreviousFile"));
}

static void Popup_Show_Next(void)
{
    Popup_Show(Lang_GetUTF8("DV_Popup_NextFile"));
}

static void Popup_Show_Volume(void)
{
    char str[32];
    snprintf(str,32,Lang_GetUTF8("DV_Popup_Volume"),(strpcmGetVideoVolume64()*100)/64);
    Popup_Show(str);
}

static void Popup_Show_BacklightLevel(void)
{
    char str[32];
    snprintf(str,32,Lang_GetUTF8("DV_Popup_BacklightLevel"),1+ProcState.System.BacklightLevel);
    Popup_Show(str);
}

static void Popup_Show_Seek(s32 val)
{
    char str[32];
    char valstr[32];
      
    if(val<0){
        snprintf(valstr,32,"-%d%%",-val);
    }else{
        snprintf(valstr,32,"+%d%%",val);
    }
    
    snprintf(str,32,Lang_GetUTF8("DV_Popup_Seek"),valstr);

    Popup_Show(str);
}

