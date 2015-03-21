
DATA_IN_IWRAM_TextView static bool BookmarkUI_isSaveMode;

DATA_IN_IWRAM_TextView static TCallBack BookmarkUI_BackupCallBack;

static void BookmarkUI_End(void)
{
    TCallBack *pCallBack=CallBack_GetPointer();
    *pCallBack=BookmarkUI_BackupCallBack;
    
    ScreenRedrawFlag=true;
        
    if(ScreenRedrawFlag==true){
        ScreenRedrawFlag=false;
        TextView_SubDrawText(&ScrollBar);
        TextView_MainDrawText(&ScrollBar);
    }
    
    if(ProcState.Text.isSwapDisp==true) {
        if(ProcState.Text.BacklightFlag==false){
            IPC6->LCDPowerControl=LCDPC_ON_TOP;
            BrightLevel=0*0x100;
        }else{
            IPC6->LCDPowerControl=LCDPC_ON_BOTH;
            BrightLevel=16*0x100;
        }
        pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
    }
    ClockTimeOut=0;
}

static void BookmarkUI_DrawPreview(void)
{
    u32 itemidx=Bookmark_GetCurrentItemIndex();
    TBookmarkItem *pbmi=Bookmark_Load(itemidx);
  
    CglCanvas *pcan=pScreenSub->pCanvas;
    TextView_GetSkin(ETV_Bookmark_PreviewBG)->pCanvas->BitBltFullBeta(pcan);
  
    if(BookmarkUI_isSaveMode==true) return;
    
    pcan->SetFontTextColor(ColorTable.TextView.PreviewText);
    pExtFont->Color=ColorTable.TextView.PreviewText;
  
    u32 x=LeftMargin;
  
    char str[128];
    if(pbmi->LineNum==0){
        snprintf(str,128,Lang_GetUTF8("TV_BookmarkText_Empty"),1+itemidx);
    }else{
        char datestr[16],timestr[16];
        Date_GetDateStrBuf(datestr,16,pbmi->DateTime.Date);
        Date_GetTimeStrBuf_12h(timestr,16,pbmi->DateTime.Time);
        snprintf(str,128,Lang_GetUTF8("TV_BookmarkText_Info"),1+itemidx,pbmi->LineNum,datestr,timestr);
    }
    pcan->TextOutUTF8(x,10,str);
  
    if(pbmi->LineNum==0) return;
  
    u32 th=ShowLineHeight;
    u32 y=TopMargin+1+(th*2);
  
    for(u32 idx=0;idx<ShowLineCount-2;idx++){
        UnicodeChar strw[128+1];
        if(libconv_GetTextLine(pbmi->LineNum-1+idx,strw)==true){
            ExtFont_TextOutW(pcan,x,y,strw);
        }
        y+=th;
    }
}

static void BookmarkUI_DrawScreen(void)
{
    CglCanvas *pcan=pScreenMain->pBackCanvas;
  
    {
        const char *pmsg=NULL;
        if(BookmarkUI_isSaveMode==false){
            TextView_GetSkin(ETV_Bookmark_LoadBG)->pCanvas->BitBltFullBeta(pcan);
            pmsg=Lang_GetUTF8("TV_BookmarkText_LoadTitle");
        }else{
            TextView_GetSkin(ETV_Bookmark_SaveBG)->pCanvas->BitBltFullBeta(pcan);
            pmsg=Lang_GetUTF8("TV_BookmarkText_SaveTitle");
        }
        if(Skin_OwnerDrawText.BookmarkMenu==true) pmsg=NULL;
        if(str_isEmpty(pmsg)==false){
            u32 x=32,y=6;
            pcan->SetFontTextColor(ColorTable.TextView.BookmarkMenu_TitleText);
            pcan->TextOutUTF8(x,y,pmsg);
        }
    }
  
    u32 th=glCanvasTextHeight+2;
    u32 x=44,y=40;
  
    for(u32 idx=0;idx<BookmarkItemCount;idx++){
        TBookmarkItem *pbmi=Bookmark_Load(idx);
    
        if(idx==Bookmark_GetCurrentItemIndex()){
            TextViewAlpha_GetSkin(ETVA_Bookmark_Cursor)->BitBlt(pcan,0,y-8);
            pcan->SetFontTextColor(ColorTable.TextView.BookmarkMenu_Empty);
        }else{
            TextViewAlpha_GetSkin(ETVA_Bookmark_Clear)->BitBlt(pcan,0,y-8);
            pcan->SetFontTextColor(ColorTable.TextView.BookmarkMenu_Exists);
        }
    
        char str[128];
        if(pbmi->LineNum==0){
            snprintf(str,128,Lang_GetUTF8("TV_BookmarkText_Empty"),1+idx);
        }else{
            CglTGF *pIconBG=NULL;
            switch(idx){
                case 0: pIconBG=TextViewAlpha_GetSkin(ETVA_Bookmark_Slot0Icon); break;
                case 1: pIconBG=TextViewAlpha_GetSkin(ETVA_Bookmark_Slot1Icon); break;
                case 2: pIconBG=TextViewAlpha_GetSkin(ETVA_Bookmark_Slot2Icon); break;
                case 3: pIconBG=TextViewAlpha_GetSkin(ETVA_Bookmark_Slot3Icon); break;
            }
            if(pIconBG!=NULL) pIconBG->BitBlt(pcan,0,y-8);
      
            char datestr[16],timestr[16];
            Date_GetDateStrBuf(datestr,16,pbmi->DateTime.Date);
            Date_GetTimeStrBuf_12h(timestr,16,pbmi->DateTime.Time);
            snprintf(str,128,Lang_GetUTF8("TV_BookmarkText_Info"),1+idx,pbmi->LineNum,datestr,timestr);
        }
        pcan->TextOutUTF8(x,y,str);
        y+=th;
    
        UnicodeChar strw[128+1];
        if(pbmi->LineNum!=0){
            if(libconv_GetTextLine(pbmi->LineNum-1,strw)==true) pcan->TextOutW(x,y,strw);
        }
        y+=th;
    
        y+=10;
    }
  
    BookmarkUI_DrawPreview();
  
    ScreenMain_Flip_ProcFadeEffect();
}

// ------------------------------------------------------------------

static void BookmarkUI_Execute(void)
{
    u32 cidx=Bookmark_GetCurrentItemIndex();
  
    if(BookmarkUI_isSaveMode==false){
        TBookmarkItem *pbmi=Bookmark_Load(cidx);
        if(pbmi->LineNum!=0){
            ScrollBar_SetCurrentLineIndex(&ScrollBar,pbmi->LineNum-1);
            Sound_Start(WAVFN_Open);
            Popup_Show_Boomark_Load();
        }else{
            Sound_Start(WAVFN_Notify);
            Popup_Show_Boomark_Empty();
        }
    }else{
        Sound_Start(WAVFN_Open);
        Popup_Show_Boomark_Save();
        Bookmark_Save(cidx,1+ScrollBar.CurrentLineIndex,libconv_GetTextFileOffset(ScrollBar.CurrentLineIndex));
    }
}

// -----------------------------------------------------------------

static void BKMUI_CB_KeyPress(u32 VsyncCount,u32 Keys)
{
    if((Keys&KEY_B)!=0){
        Sound_Start(WAVFN_Open);
        BookmarkUI_End();
        return;
    }
  
    if((Keys&KEY_A)!=0){
        BookmarkUI_Execute();
        BookmarkUI_End();
        return;
    }
  
    if(WaitKeyRelease) return;
    
    if((Keys&(KEY_X|KEY_Y))!=0){
        if((Keys&KEY_Y)!=0) BookmarkUI_isSaveMode=false;
        if((Keys&KEY_X)!=0) BookmarkUI_isSaveMode=true;
        Sound_Start(WAVFN_Click);
        Bookmark_SetCurrentItemIndex(0);
        BookmarkUI_DrawScreen();
        WaitKeyRelease=true;
        return;
    }
  
    if((Keys&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))!=0){
        u32 cidx=Bookmark_GetCurrentItemIndex();
        if((Keys&KEY_UP)!=0){
            if(0<cidx) cidx--;
        }
        if((Keys&KEY_DOWN)!=0){
            if(cidx<(BookmarkItemCount-1)) cidx++;
        }
        if((Keys&KEY_LEFT)!=0) cidx=0;
        if((Keys&KEY_RIGHT)!=0) cidx=BookmarkItemCount-1;
        if(Bookmark_GetCurrentItemIndex()!=cidx){
            Sound_Start(WAVFN_Click);
            Bookmark_SetCurrentItemIndex(cidx);
            BookmarkUI_DrawScreen();
        }
        WaitKeyRelease=true;
    }
}

// -------------------------------------------------------

DATA_IN_IWRAM_TextView static bool bkmf;
DATA_IN_IWRAM_TextView static u32 bkmlastidx;

static u32 BKMUI_Mouse_GetCursorIndexFromPoint(s32 x,s32 y)
{
    u32 th=glCanvasTextHeight+2;
    u32 ix=44,iy=40,iw=ScreenWidth-ix,ih=th*2;
  
    u32 cidx=(u32)-1;
  
    for(u32 idx=0;idx<BookmarkItemCount;idx++){
        if((ix<=x)&&(x<=(ix+iw))&&(iy<=y)&&(y<=(iy+ih))) cidx=idx;
        iy+=ih+10;
    }
  
    return(cidx);
}

static void BKMUI_CB_MouseDown(s32 x,s32 y)
{
    if(ProcState.Text.isSwapDisp==true) return;
    
    bkmf=false;
    bkmlastidx=Bookmark_GetCurrentItemIndex();
  
    u32 cidx=BKMUI_Mouse_GetCursorIndexFromPoint(x,y);
    if(cidx==(u32)-1) return;
  
    bkmf=true;
    Sound_Start(WAVFN_Click);
    Bookmark_SetCurrentItemIndex(cidx);
    BookmarkUI_DrawScreen();
}

static void BKMUI_CB_MouseMove(s32 x,s32 y)
{
    if(bkmf==false) return;
  
    u32 cidx=BKMUI_Mouse_GetCursorIndexFromPoint(x,y);
    if(cidx==(u32)-1) return;
  
    Bookmark_SetCurrentItemIndex(cidx);
    BookmarkUI_DrawScreen();
}

static void BKMUI_CB_MouseUp(s32 x,s32 y)
{
    if(bkmf==false) return;
    bkmf=false;
  
    u32 cidx=BKMUI_Mouse_GetCursorIndexFromPoint(x,y);
    if(cidx!=bkmlastidx) return;
  
    BookmarkUI_Execute();
    BookmarkUI_End();
}

static void BKMUI_CB_VsyncUpdate(u32 VsyncCount)
{
    Popup_VsyncUpdate(VsyncCount);
}

// -----------------------------------------------------------------

static void BookmarkUI_Start(bool isSaveMode)
{
    bkmf=false;
    bkmlastidx=0;
  
    BookmarkUI_isSaveMode=isSaveMode;
  
    TCallBack *pCallBack=CallBack_GetPointer();
    BookmarkUI_BackupCallBack=*pCallBack;
  
    pCallBack->Start=NULL;
    pCallBack->VsyncUpdate=BKMUI_CB_VsyncUpdate;
    pCallBack->End=NULL;
    pCallBack->KeyPress=BKMUI_CB_KeyPress;
    pCallBack->MouseDown=BKMUI_CB_MouseDown;
    pCallBack->MouseMove=BKMUI_CB_MouseMove;
    pCallBack->MouseUp=BKMUI_CB_MouseUp;
  
    for(u32 idx=0;idx<BookmarkItemCount;idx++){
        TBookmarkItem *pbmi=Bookmark_Load(idx);
        if(pbmi->LineNum==0) continue;
        
        u32 offset=libconv_GetTextFileOffset(pbmi->LineNum-1);
        if(pbmi->FileOffset!=offset){
            u32 linenum=pbmi->LineNum-1;
            if(pbmi->FileOffset>offset){
                while(linenum<TextLinesCount){
                    u32 oft=libconv_GetTextFileOffset(linenum);
                    if(pbmi->FileOffset<=oft){
                        Bookmark_Save(idx,1+linenum,oft);
                        break;
                    }
                    linenum++;
                }
            }else{
                while(linenum>0){
                    u32 oft=libconv_GetTextFileOffset(linenum);
                    if(pbmi->FileOffset>=oft){
                        Bookmark_Save(idx,1+linenum,oft);
                        break;
                    }
                    linenum--;
                }
            }
        }
    }
    
    Sound_Start(WAVFN_Open);
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
  
    if(ProcState.Text.isSwapDisp==true){
        pScreenSub->SetBlackOutLevel16(0);
    }
    
    SetProcFadeEffect(EPFE_FastCrossFade);
    BookmarkUI_DrawScreen();
}

