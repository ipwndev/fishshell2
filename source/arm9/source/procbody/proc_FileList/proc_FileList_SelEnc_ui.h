
DATA_IN_IWRAM_FileList static bool SelectEncodeUI_BackupPlayPause;

DATA_IN_IWRAM_FileList static CglCanvas *pMasterBG;

DATA_IN_IWRAM_FileList static s32 FSEUI_ItemsCount;
DATA_IN_IWRAM_FileList static s32 FSEUI_ItemIndex;

static void SelectEncodeUI_DrawScreen(void)
{
    CglCanvas *pcan=pScreenMain->pBackCanvas;
    pMasterBG->BitBltFullBeta(pcan);
    
    const char *ptext=NULL;
    
    ptext=Lang_GetUTF8("TV_SelEnc_Title");
        
    s32 x=(ScreenWidth-pcan->GetTextWidthUTF8(ptext))/2;
    s32 y=5;
    
    pcan->SetFontTextColor(ColorTable.FileList.SelectText);
    pcan->TextOutUTF8(x,y,ptext);
    
    y+=glCanvasTextHeight+20;
    
    
    for(u32 idx=0;idx<FSEUI_ItemsCount;idx++){
        if(idx!=FSEUI_ItemIndex){
            pcan->SetFontTextColor(RGB15(24,24,24)|BIT15);
        }else{
            pcan->SetFontTextColor(ColorTable.FileList.SelectText);
        } 
      
        switch(idx){
            case 0: ptext=Lang_GetUTF8("TV_SelEnc_Items_AutoDetect"); break;
            case 1: ptext=Lang_GetUTF8("TV_SelEnc_Items_ANSI"); break;
            case 2: ptext=Lang_GetUTF8("TV_SelEnc_Items_EUC"); break;
            case 3: ptext=Lang_GetUTF8("TV_SelEnc_Items_UTF16BE"); break;
            case 4: ptext=Lang_GetUTF8("TV_SelEnc_Items_UTF16LE");break;
            case 5: ptext=Lang_GetUTF8("TV_SelEnc_Items_UTF8"); break;
            case 6: ptext=Lang_GetUTF8("TV_SelEnc_Items_TextEdit");break;
        }
      
        char tmpstr[255];
        sprintf(tmpstr,"[ %s ]",ptext);
        
        if(idx==FSEUI_ItemIndex) ptext=tmpstr;
        if(ptext!=NULL){
            x=(ScreenWidth-pcan->GetTextWidthUTF8(ptext))/2;
        
            pcan->TextOutUTF8(x,y,ptext);
            
            y+=glCanvasTextHeight+4;
        
        }
    }
    
    y=ScreenHeight-(glCanvasTextHeight+4)*3-5;
    
    pcan->SetFontTextColor(RGB15(24,24,24)|BIT15);
    
    if(FSEUI_ItemsCount==6) {
        ptext=Lang_GetUTF8("TV_SelEnc_Help_NoEdit0");
        if(ptext!=NULL){
            x=(ScreenWidth-pcan->GetTextWidthUTF8(ptext))/2;
                
            pcan->TextOutUTF8(x,y,ptext);
                    
            y+=glCanvasTextHeight+4;
            
        }
        ptext=Lang_GetUTF8("TV_SelEnc_Help_NoEdit1");
        if(ptext!=NULL){
            x=(ScreenWidth-pcan->GetTextWidthUTF8(ptext))/2;
                        
            pcan->TextOutUTF8(x,y,ptext);
                            
            y+=glCanvasTextHeight+4;
                    
        }
    }
    ptext=Lang_GetUTF8("TV_SelEnc_Help_Normal");
    if(ptext!=NULL){
        x=(ScreenWidth-pcan->GetTextWidthUTF8(ptext))/2;
        y=ScreenHeight-(glCanvasTextHeight+4);
        pcan->TextOutUTF8(x,y,ptext);
                            
    }
    
    ScreenMain_Flip_ProcFadeEffect();
}

// ------------------------------------------------------------------

static void SelectEncodeUI_Execute(void)
{
    ManualTextEncode=(ETextEncode)FSEUI_ItemIndex;
    ManualTextEncode_OverrideFlag=true;
}

// -----------------------------------------------------------------

static void SelectEncodeUI_Start(void)
{
    Sound_Start(WAVFN_Opening);
  
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    
    { // Draw sub frame.
        u16 *pb=pScreenSub->GetVRAMBuf();
        u32 size=ScreenHeight*ScreenWidth;
            
        pb=FileList_DrawID3Tag_asm_Fill25per(pb,size);
    }
    { // Draw main frame.
        pMasterBG=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
        CglCanvas *psrccan=pScreenMain->pViewCanvas;
        psrccan->BitBltFullBeta(pMasterBG);
        u16 *pb=pMasterBG->GetVRAMBuf();
        u32 size=ScreenHeight*ScreenWidth;
        
        pb=FileList_DrawID3Tag_asm_Fill25per(pb,size);
    }
    
    FSEUI_ItemsCount=6;
    FSEUI_ItemIndex=0;

    TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
    if(isExistsTextEditor && pndsf->FileSize<(128*1024)) FSEUI_ItemsCount=7;
    
    SetProcFadeEffect(EPFE_FastCrossFade);
    SelectEncodeUI_DrawScreen();
    
    while(WaitKeyRelease) WaitForVBlank();
    
    while(1){
        if(!WaitKeyRelease){
            if(IPC6->RequestUpdateIPC==false){
                u32 Keys=(~REG_KEYINPUT)&0x3ff;
            
                if((Keys&KEY_B)!=0){
                    Sound_Start(WAVFN_Notify);
                    WaitKeyRelease=true;
                    break;
                }
          
                if((Keys&KEY_A)!=0){
                    Sound_Start(WAVFN_Click);
                    SelectEncodeUI_Execute();
                    break;
                }
          
                if((Keys&(KEY_UP|KEY_DOWN))!=0){
                    if((Keys&KEY_DOWN)!=0) FSEUI_ItemIndex++;
                    if((Keys&KEY_UP)!=0) FSEUI_ItemIndex--;
                    if(FSEUI_ItemIndex<0) FSEUI_ItemIndex=(FSEUI_ItemsCount-1);
                    if(FSEUI_ItemIndex>(FSEUI_ItemsCount-1)) FSEUI_ItemIndex=0;
                    SelectEncodeUI_DrawScreen();
                    WaitKeyRelease=true;
                }
                IPC6->RequestUpdateIPC=true;
            }
        }else{
            WaitForVBlank();
        }
    }
    
    if(pMasterBG!=NULL){
        delete pMasterBG; pMasterBG=NULL;
    }
    
    PlayList_SetPause(SelectEncodeUI_BackupPlayPause);
    
    Backlight_ResetTimer();
    ScreenRedrawFlag=true;
    ForceUpdateSubScreenFlag=true;
}

