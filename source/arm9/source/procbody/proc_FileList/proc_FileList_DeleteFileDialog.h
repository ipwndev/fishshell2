
static void CB_KeyLongPress_ins_WaitForKeyReleases(void)
{
  _consolePrint("Wait for key releases.\n");
  while(1){
    if(IPC6->RequestUpdateIPC==false){
      u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
      if(keys==0) break;
      IPC6->RequestUpdateIPC=true;
    }
  }
}

static void CB_KeyLongPress_ins_WaitForKeyPress(void)
{
  _consolePrint("Wait for key press.\n");
  while(1){
    if(IPC6->RequestUpdateIPC==false){
      u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
      if(keys!=0) break;
      IPC6->RequestUpdateIPC=true;
    }
  }
}

static void CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(const char *pTitle,const char *pLine0,const char *pLine1,const char *pLine2)
{
  CglCanvas *pDstBM=pScreenMain->pBackCanvas;
  
  CglB15 *pb15=FileList_GetSkin(EFLS_DeleteFileDialog);
  u32 WinPosX=(ScreenWidth-pb15->GetWidth())/2;
  u32 WinPosY=(ScreenHeight-pb15->GetHeight())/2;
  
  pScreenMain->pViewCanvas->BitBltFullBeta(pDstBM);
  pb15->BitBlt(pDstBM,WinPosX,WinPosY,pb15->GetWidth(),pb15->GetHeight(),0,0);
  
  u32 tx=WinPosX,ty=WinPosY,th=glCanvasTextHeight+1;
  
  tx+=22; ty+=5;
  if(pTitle!=NULL){
    pDstBM->SetFontTextColor(ColorTable.FileList.DeleteFileDialog_Title_Text);
    pDstBM->TextOutUTF8(tx,ty,pTitle);
  }
  
  tx-=22;
  tx+=5; ty+=17;
  
//  if(str_isEmpty(pLine0)==true) ty+=th/2;
  if(str_isEmpty(pLine1)==true) ty+=th/2;
  if(str_isEmpty(pLine2)==true) ty+=th/2;
  
  for(u32 idx=0;idx<3;idx++){
    const char *pmsg=NULL;
    switch(idx){
      case 0: pmsg=pLine0; break;
      case 1: pmsg=pLine1; break;
      case 2: pmsg=pLine2; break;
    }
    if(str_isEmpty(pmsg)==false){
      pDstBM->SetFontTextColor(ColorTable.FileList.DeleteFileDialog_Body_Text);
      pDstBM->TextOutUTF8(tx,ty,pmsg);
      ty+=th;
    }
  }
  
  ScreenMain_Flip_ProcFadeEffect();
}

static void CB_KeyLongPress_ins_ExecuteDelete(void)
{
  TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
  
  char SrcFullAlias[256];
  
  {
      if(VerboseDebugLog==true) _consolePrint("Move: Get source alias.\n");
    const char *pfn=ConvertFull_Unicode2Alias(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode);
    if(pfn==NULL) StopFatalError(16803,"CB_KeyLongPress_ins_ExecuteDelete: Not found source alias.\n");
    StrCopy(pfn,SrcFullAlias);
    if(VerboseDebugLog==true) _consolePrintf("Move: Source='%s'.\n",SrcFullAlias);
  }
  
  if(VerboseDebugLog==true) _consolePrint("Move: Check exists dist file.\n");
  if(FileExistsUnicode(Shell_GetDustBoxPathUnicode(),pndsf->pFilenameUnicode)==false){
      if(VerboseDebugLog==true) _consolePrint("Move: Create dist file.\n");
    if(Shell_CreateNewFileUnicode(Shell_GetDustBoxPathUnicode(),pndsf->pFilenameUnicode)==NULL) StopFatalError(16805,"CB_KeyLongPress_ins_ExecuteDelete: Can not create move-to file.\n");
  }
  
  char DstFullAlias[256];
  
  {
      if(VerboseDebugLog==true) _consolePrint("Move: Get dist alias.\n");
    const char *pfn=ConvertFull_Unicode2Alias(Shell_GetDustBoxPathUnicode(),pndsf->pFilenameUnicode);
    if(pfn==NULL) StopFatalError(16806,"CB_KeyLongPress_ins_ExecuteDelete: Not found dist alias.\n");
    StrCopy(pfn,DstFullAlias);
    if(VerboseDebugLog==true) _consolePrintf("Move: Dist='%s'.\n",DstFullAlias);
  }
  
  if(FAT2_Move(SrcFullAlias,DstFullAlias)==false) StopFatalError(16807,"CB_KeyLongPress_ins_ExecuteDelete: FAT2_Move() failed.\n");
}

static bool CB_KeyLongPress_ins_isInsidePath(const UnicodeChar *pBasePath,const UnicodeChar *pCheckPath)
{
  while(*pBasePath!=0){
    if(*pBasePath++!=*pCheckPath++) return(false);
  }
  
  return(true);
}

static void CB_KeyLongPress_ins_DeleteFile(void)
{
  Backlight_ResetTimer();
  
  strpcm_ExclusivePause=true;
  
  Sound_Start(WAVFN_Notify);
  
  RequestRefreshPlayCursorIndex=false;
  PlayCursorIndex=-1;
  
  Popup_VsyncUpdate((u32)-1);
  
  FileList_MainDrawBG(&ScrollBar);
  FileList_SubDrawBG(&ScrollBar);
  
/*
  FAT2_ShowDirectoryEntryList("/");
  FAT2_ShowDirectoryEntryList("/DUSTBOX");
  while(1);
*/
  
  {
    const char *ppath=ConvertFull_Unicode2Alias(Shell_GetDustBoxPathUnicode(),NULL);
/*
    // 新規フォルダ作成はchkdskがエラー報告をするので実装しない。
    if(ppath==NULL){
      if(FAT2_mkdir("/DUSTBOX")==false) StopFatalError(16808,"CB_KeyLongPress: Can not create dustbox folder.\n");
    }
    ppath=ConvertFull_Unicode2Alias(Shell_GetDustBoxPathUnicode(),NULL);
*/
    if(ppath==NULL){
      // ゴミ箱フォルダが見つからないエラーダイアログを出す
        if(VerboseDebugLog==true) _consolePrint("CB_KeyLongPress: Can not found dustbox.\n");
      CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_NotFoundDustBox_Title"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_NotFoundDustBox_Line0"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_NotFoundDustBox_Line1"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_NotFoundDustBox_Line2"));
      CB_KeyLongPress_ins_WaitForKeyReleases();
      CB_KeyLongPress_ins_WaitForKeyPress();
      CB_KeyLongPress_ins_WaitForKeyReleases();
      FileList_MainDrawBG(&ScrollBar);
      FileList_SubDrawBG(&ScrollBar);
      REG_IME=0;
      VBlankPassedCount=0;
      REG_IME=1;
      strpcm_ExclusivePause=false;
      return;
    }
  }
  
  {
    if(CB_KeyLongPress_ins_isInsidePath(Shell_GetDustBoxPathUnicode(),ProcState.FileList.CurrentPathUnicode)==true){
      // ゴミ箱フォルダ内エラーダイアログを出す
        if(VerboseDebugLog==true) _consolePrint("CB_KeyLongPress: Can not move in dustbox folder.\n");
      CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_DisabledDustBox_Title"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledDustBox_Line0"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledDustBox_Line1"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledDustBox_Line2"));
      CB_KeyLongPress_ins_WaitForKeyReleases();
      CB_KeyLongPress_ins_WaitForKeyPress();
      CB_KeyLongPress_ins_WaitForKeyReleases();
      FileList_MainDrawBG(&ScrollBar);
      FileList_SubDrawBG(&ScrollBar);
      REG_IME=0;
      VBlankPassedCount=0;
      REG_IME=1;
      strpcm_ExclusivePause=false;
      return;
    }
  }
  
  {
    if(CB_KeyLongPress_ins_isInsidePath(Shell_GetSystemPathUnicode(),ProcState.FileList.CurrentPathUnicode)==true){
      // システムフォルダ内エラーダイアログを出す
        if(VerboseDebugLog==true) _consolePrint("CB_KeyLongPress: Can not move in dustbox folder.\n");
      CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_DisabledSystemFolder_Title"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledSystemFolder_Line0"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledSystemFolder_Line1"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledSystemFolder_Line2"));
      CB_KeyLongPress_ins_WaitForKeyReleases();
      CB_KeyLongPress_ins_WaitForKeyPress();
      CB_KeyLongPress_ins_WaitForKeyReleases();
      FileList_MainDrawBG(&ScrollBar);
      FileList_SubDrawBG(&ScrollBar);
      REG_IME=0;
      VBlankPassedCount=0;
      REG_IME=1;
      strpcm_ExclusivePause=false;
      return;
    }
  }
  
  {
    TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
    if((pndsf->FileType==ENFFT_UpFolder)||(pndsf->FileType==ENFFT_Folder)){
      // フォルダ移動エラーダイアログを出す
        if(VerboseDebugLog==true) _consolePrint("CB_KeyLongPress: Can not move folder.\n");
      CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_DisabledMoveFolder_Title"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledMoveFolder_Line0"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledMoveFolder_Line1"),
                                                       Lang_GetUTF8("FL_DeleteFileDialog_DisabledMoveFolder_Line2"));
      CB_KeyLongPress_ins_WaitForKeyReleases();
      CB_KeyLongPress_ins_WaitForKeyPress();
      CB_KeyLongPress_ins_WaitForKeyReleases();
      FileList_MainDrawBG(&ScrollBar);
      FileList_SubDrawBG(&ScrollBar);
      REG_IME=0;
      VBlankPassedCount=0;
      REG_IME=1;
      strpcm_ExclusivePause=false;
      return;
    }
  }
  
  {
    ScrollBar_SetSelectedIndex(&ScrollBar,ScrollBar.SelectedIndex);
    ScrollBar.ShowPos=ScrollBar.TopPos;
    FileList_MainDrawBG(&ScrollBar);
    FileList_SubDrawBG(&ScrollBar);
  }
  
  if(ProcState.FileList.EasyDeleteKey==false){
    CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_Question_Title"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_Line0"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_Line1"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_Line2"));
    }else{
    CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_Question_EasyDelete_Title"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_EasyDelete_Line0"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_EasyDelete_Line1"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Question_EasyDelete_Line2"));
  }
  
  CB_KeyLongPress_ins_WaitForKeyReleases();
  
  bool execute=false;
  
  while(1){
      if(IPC6->RequestUpdateIPC==false){
        u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
        if(keys==KEY_B){
          execute=false;
          break;
        }
        if(ProcState.FileList.EasyDeleteKey==false){
          if(keys==(KEY_A|KEY_L|KEY_R)){
            execute=true;
            break;
          }
          }else{
          if(keys==(KEY_X|KEY_A)){
            execute=true;
            break;
          }
        }
        IPC6->RequestUpdateIPC=true;
      }
    }
  
  if(execute==false){
    FileList_MainDrawBG(&ScrollBar);
    FileList_SubDrawBG(&ScrollBar);
    CB_KeyLongPress_ins_WaitForKeyReleases();
    strpcm_ExclusivePause=false;
    
    }else{
    Sound_Start(WAVFN_Notify);
    TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
    if(Unicode_isEqual(PlayList_GetCurrentPath(),ProcState.FileList.CurrentPathUnicode) || PlayList_DeleteListItem(pndsf->pFilenameUnicode)) {
    	//if(Unicode_isEqual(PlayList_GetCurrentFilename(),pndsf->pFilenameUnicode)) {
    		
    	//}
    	
    	PlayList_Stop(false);
    	PlayList_Free();
    	PlayList_MakeBlank();
    }
    
    CB_KeyLongPress_ins_ExecuteDelete();
    
    u32 backtoppos=ScrollBar.TopPos;
    u32 backidx=ScrollBar.SelectedIndex;
    
    ChangedCurrentPath=true;
    FileListInit();
    
    ScrollBar_SetDirectTopPos(&ScrollBar,backtoppos);
    ScrollBar_SetSelectedIndex(&ScrollBar,backidx);
    ScrollBar.ShowPos=ScrollBar.TopPos;
    
    FileList_MainDrawBG(&ScrollBar);
    FileList_SubDrawBG(&ScrollBar);
    
    CB_KeyLongPress_ins_ExecuteDelete_ins_DrawWindow(Lang_GetUTF8("FL_DeleteFileDialog_Success_Title"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Success_Line0"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Success_Line1"),
                                                     Lang_GetUTF8("FL_DeleteFileDialog_Success_Line2"));
    
    CB_KeyLongPress_ins_WaitForKeyReleases();
    CB_KeyLongPress_ins_WaitForKeyPress();
    CB_KeyLongPress_ins_WaitForKeyReleases();
    
    FileList_MainDrawBG(&ScrollBar);
    FileList_SubDrawBG(&ScrollBar);
  }
  
  REG_IME=0;
  VBlankPassedCount=0;
  REG_IME=1;
  
  ChangedCurrentPath=true;
  
  PlayList_Start(true,NULL,NULL);

  strpcm_ExclusivePause=false;
}

