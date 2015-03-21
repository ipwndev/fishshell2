
// -----------------------------

static __attribute__ ((noinline)) void CB_KeyPress_ins_MakePlayList(u32 Keys)
{
  PlayList_Free();
  
  ChangedCurrentPath=true;
  
  if((Keys&KEY_L)==0){
    _consolePrint("Create play list on folder.\n");
    TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
    if(pndsf->FileType!=ENFFT_Folder){
      PlayList_MakeFolder(true,ProcState.FileList.CurrentPathUnicode);
      }else{
      UnicodeChar *pBasePathW=Unicode_AllocateCopy(&MM_Temp,FullPathUnicodeFromSplitItem(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode));
      PlayList_MakeFolder(true,pBasePathW);
      if(pBasePathW!=NULL){
        safefree(&MM_Temp,pBasePathW); pBasePathW=NULL;
      }
    }
    }else{
    _consolePrint("Create play list on disk.\n");
    UnicodeChar BasePathW[256]={(UnicodeChar)'/',0};
    FAT_FILE *pf=Shell_FAT_fopen_Internal(PlayListFolderFilename);
    if(pf!=NULL) {
        char path[256];
        if(FAT2_fgets(path, sizeof(path), pf)!=0) {
            StrConvert_UTF82Unicode(path,BasePathW);
            if(BasePathW[0]!=(UnicodeChar)'/') StopFatalError(0,PlayListFolderFilename " error. '/' is necessary for the head.\n");
            {
                const char *pBasePathAlias=ConvertFull_Unicode2Alias(BasePathW,NULL);
                if(FAT2_chdir_Alias(pBasePathAlias)==false) {
                    BasePathW[0]=(UnicodeChar)'/';
                    BasePathW[1]=(UnicodeChar)0;
                }
            }
        }
        FAT2_fclose(pf);
    }
    PlayList_MakeFolder(true,BasePathW);
  }
  
  if(PlayList_Start(true,NULL,NULL)==false){
    Sound_Start(WAVFN_Notify);
    ErrorDialog_Set(EEC_NotFoundMusicFile);
    ErrorDialog_Draw(pScreenMainOverlay->pCanvas);
    return;
  }
  
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
}

static __attribute__ ((noinline)) void CB_KeyPress_ins_RightButton_Seek(u32 Keys)
{
  s32 val=0;
  
  if(PlugLRC_isOpened()) {
      if((Keys&(KEY_X|KEY_Y))!=0){
          if((Keys&KEY_X)!=0) val=100;
          if((Keys&KEY_Y)!=0) val=-100;
          Popup_Show_LyricOffset(val);
          PlugLRC_SetOffset(PlugLRC_GetOffset()+val);
          return;
      }
  }
  
  if((Keys&KEY_LEFT)!=0) val=-1;
  if((Keys&KEY_RIGHT)!=0) val=1;
  if((Keys&KEY_UP)!=0) val=-5;
  if((Keys&KEY_DOWN)!=0) val=5;
  
  if(DLLSound_SeekPer(val)==true){
    Popup_Show_Seek(val);
    ScreenRedrawFlag=true;
    ForceUpdateSubScreenFlag=true;
  }
}

static __attribute__ ((noinline)) void CB_KeyPress_ins_LeftButton(u32 Keys)
{
  if((IPC6->PanelOpened==false)&&(ProcState.System.LRKeyLockType==ELRLT_RelationalPanel)||(ProcState.System.LRKeyLockType==ELRLT_AlwayOff)) return;
  
  if((Keys&KEY_LEFT)!=0){
    Sound_Start(WAVFN_Click);
    switch(MP3Cnt_WindowState){
    	case EMCWS_Hide: {
    		Sound_Start(WAVFN_Click);
    		MP3Cnt_WindowState=EMCWS_Lock;
    		MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
    	} break;
    	case EMCWS_Link: break;
    	case EMCWS_Lock: {
    		MP3Cnt_WindowState=EMCWS_Hide;
    		MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
    	} break;
    }
    ScreenRedrawFlag=true;
  }
  
  if((Keys&(KEY_RIGHT|KEY_A))!=0){
    MP3Cnt_Exec_Next();
    ScreenRedrawFlag=true;
  }
  
  if((Keys&KEY_B)!=0){
    MP3Cnt_Exec_Prev();
    ScreenRedrawFlag=true;
  }
  
  if((Keys&(KEY_X|KEY_Y))!=0){
    if((Keys&KEY_X)!=0) ChangeNextBacklightLevel();
    if((Keys&KEY_Y)!=0) ChangePrevBacklightLevel();
    Popup_Show_BacklightLevel();
  }
  
  if((Keys&KEY_UP)!=0){
    MP3Cnt_Exec_ChangePause();
    ScreenRedrawFlag=true;
  }
  
  if((Keys&KEY_DOWN)!=0){
    MP3Cnt_Exec_ChangePlayMode();
    ScreenRedrawFlag=true;
  }
}

static __attribute__ ((noinline)) void CB_KeyPress_ins_StartButton(u32 Keys)
{
  ProcState_RequestSave=true;
  
  SetNextProc(ENP_SysMenu,EPFE_CrossFade);
}

DATA_IN_IWRAM_FileList static u32 KEYS_PressBCount=0;

static __attribute__ ((noinline)) void CB_KeyPress_ins_BButton(u32 VsyncCount,u32 Keys)
{
	if(WaitKeyRelease) return;
	
  if(ProcState.FileList.BButtonToFolderUp==false){
    if(PlayList_isOpened()==false){
      MoveUpFolder();
      WaitKeyRelease=true;
      }else{
      PlayList_Stop(false);
      PlayList_Free();
      PlayList_MakeBlank();
      Sound_Start(WAVFN_Click);
      ScreenRedrawFlag=true;
      ForceUpdateSubScreenFlag=true;
      WaitKeyRelease=true;
    }
    }else{
    if(ProcState.FileList.MoveFolderLocked==false){
    	if((ProcState.FileList.CurrentPathUnicode[0]==0)||(ProcState.FileList.CurrentPathUnicode[1]==0)) {
    		const u32 Timeout=8*5;
    		if(PlayList_isOpened()==true){
    			if(KEYS_PressBCount<Timeout){
    				KEYS_PressBCount+=VsyncCount;
    				if(Timeout<=KEYS_PressBCount) {
    					PlayList_Stop(false);
    					PlayList_Free();
    					PlayList_MakeBlank();
    					Sound_Start(WAVFN_Click);
    					ScreenRedrawFlag=true;
    					ForceUpdateSubScreenFlag=true;
    					WaitKeyRelease=true;
    				}
    			}
    		}
    	}else{
    		MoveUpFolder();
    		if((ProcState.FileList.CurrentPathUnicode[0]==0)||(ProcState.FileList.CurrentPathUnicode[1]==0)) WaitKeyRelease=true;
    	}
      }else{
      PlayList_Stop(false);
      PlayList_Free();
      PlayList_MakeBlank();
      Sound_Start(WAVFN_Click);
      ScreenRedrawFlag=true;
      ForceUpdateSubScreenFlag=true;
      WaitKeyRelease=true;
    }
  }
}

static __attribute__ ((noinline)) void CB_KeyPress_ins_XYButton(u32 Keys)
{
  s32 Volume=ProcState.System.AudioVolume64;
  
  if(Keys==(KEY_Y|KEY_X)){
    Volume=64;
    }else{
    if((Keys&KEY_Y)!=0) Volume-=2;
    if((Keys&KEY_X)!=0) Volume+=2;
    
    if(Volume<0) Volume=0;
    if(Volume>strpcmVolumeMax) Volume=strpcmVolumeMax;
  }
  
  Volume&=~1;
  strpcmSetAudioVolume64(Volume);
  ProcState.System.AudioVolume64=Volume;
  ProcState_RequestSave=true;
  
  Popup_Show_Volume();
  
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
}

DATA_IN_IWRAM_FileList static u32 KEYS_PressCount=0;

static __attribute__ ((noinline)) void CB_KeyPress_ins_CursorButton(u32 VsyncCount,u32 Keys)
{
    if(WaitKeyRelease) return;
    TScrollBar *psb=&ScrollBar;
    const u32 Timeout=8*1;
  
    s32 pagesize=((psb->ClientSize+(psb->ItemHeight-1))/psb->ItemHeight)/2;
    s32 filecnt=psb->MaxPos/psb->ItemHeight-1;
    
    s32 v=0;
    if((Keys&KEY_UP)!=0) v=-1;
    if((Keys&KEY_DOWN)!=0) v=1;
    if((Keys&KEY_LEFT)!=0) v=-(pagesize);
    if((Keys&KEY_RIGHT)!=0) v=pagesize;
    
    if((psb->SelectedIndex==0 && v<0) || (psb->SelectedIndex==filecnt && v>0)) {
        if(KEYS_PressCount<Timeout){
            KEYS_PressCount+=VsyncCount;
            if(Timeout<=KEYS_PressCount) {
                if(v>0){
                    ScrollBar_SetSelectedIndex(psb,0);
                }else{
                    ScrollBar_SetSelectedIndex(psb,filecnt);
                }
                Sound_Start(WAVFN_Open);
                ScreenRedrawFlag=true;
                KEYS_PressCount=0;
                WaitKeyRelease=true;
                v=0;
            }
        }
        return;
    }
            
    if(v!=0){
        if(((psb->SelectedIndex+v)<=0) || ((psb->SelectedIndex+v)>=filecnt)) WaitKeyRelease=true;
        ScrollBar_SetSelectedIndex(psb,psb->SelectedIndex+v);
        ScreenRedrawFlag=true;
    }
}

static void CB_KeyLongPress_ins_DeleteFile(void);

static void CB_KeyLongPress(u32 Keys)
{
    if(Keys!=KEY_START) return;
      
    if(GlobalINI.System.ChildrenMode)  {
    	Sound_Start(WAVFN_Notify);
    	return;
    }
    CB_KeyLongPress_ins_DeleteFile();
}

DATA_IN_IWRAM_FileList static bool KEYS_KeyAPressed;

static void CB_KeyReleases(u32 VsyncCount){
	if(KEYS_KeyAPressed) {
		KEYS_KeyAPressed=false;
		if(LongTapState_GetProceed()==true){
			//LongPressRequest=true;
			//LongTapApplication();
			ScreenRedrawFlag=true;
		}else{
			StartApplication();
			ScreenRedrawFlag=true;
		}
	}
	LongTapState_ExecStop();
	KEYS_PressCount=0;
	KEYS_PressBCount=0;
}

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
    if(Keys!=0){
        if((Keys&KEY_TOUCH)!=0) return;
        
        if(PanelClosePowerOffTimeOut!=0){
            CB_ExternalPowerAttach();
            return;
        }
  
        {
            if(Backlight_isStandby()==true){
                Backlight_ResetTimer();
                return;
            }
            Backlight_ResetTimer();
        }
    }
    
    if(WaitKeyRelease) return;
    
  if((Keys&(KEY_R|KEY_A))==(KEY_R|KEY_A)){
    // KEY_Lも押していたらディスク内全て検索
    CB_KeyPress_ins_MakePlayList(Keys);
    return;
  }
  
  if((Keys&KEY_R)!=0){
    CB_KeyPress_ins_RightButton_Seek(Keys);
    return;
  }
  
  if((Keys&KEY_L)!=0){
    CB_KeyPress_ins_LeftButton(Keys);
    return;
  }
  
  if((Keys&KEY_SELECT)!=0){
    if(GlobalINI.FileList.CarSupplyMode==true){
      IPC6->SoundChannels++;
      if(IPC6->SoundChannels==3) IPC6->SoundChannels=0;
      }else{
      ProcState.System.LastState=ELS_Launch;
      ProcState_RequestSave=true;
      SetNextProc(ENP_Launch,EPFE_CrossFade);
      Sound_Start(WAVFN_Click);
    }
    return;
  }
  
  if((Keys&(KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))!=0){
    CB_KeyPress_ins_CursorButton(VsyncCount,Keys);
    return;
  }
  
  {
	  if((Keys & KEY_A)!=0){
		  TScrollBar *psb=&ScrollBar;
		  s32 lx=64;
		  s32 ly=psb->SelectedIndex*psb->ItemHeight-psb->TopPos+psb->ItemHeight/2;
		  
		  if(!KEYS_KeyAPressed) {
			  LongTapState_Start(lx,ly);
			  KEYS_KeyAPressed=true;
		  }
	  }
  }
  
  if((Keys&KEY_B)!=0){
    CB_KeyPress_ins_BButton(VsyncCount,Keys);
    return;
  }
  
  if((Keys&KEY_START)!=0){
	CB_KeyPress_ins_StartButton(Keys);
    return;
  }
  
  if((Keys&(KEY_Y|KEY_X))!=0){
    CB_KeyPress_ins_XYButton(Keys);
    return;
  }
}

// -----------------------------

