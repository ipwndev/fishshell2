
static void CB_VsyncUpdate(u32 VsyncCount)
{
  if(VsyncCount==1) ProcState_Save();
  
  if(isPressMouseButton==true){
    Backlight_ResetTimer();
    }else{
    Backlight_VsyncUpdate(VsyncCount);
  }
  
  Popup_VsyncUpdate(VsyncCount);
  
  if(PanelClosePowerOffTimeOut!=0){
    if(PanelClosePowerOffTimeOut<VsyncCount){
      PanelClosePowerOffTimeOut=0;
      }else{
      PanelClosePowerOffTimeOut-=VsyncCount;
    }
    if(PanelClosePowerOffTimeOut==0){
      _consolePrint("Panel closed timeout. Auto power off.\n");
      Sound_Start(WAVFN_PowerOff);
      u32 vsync=Sound_GetCurrentPlayTimePerVsync();
      _consolePrintf("FileList: PowerOffTimeOut: Wait for terminate. (%d)\n",vsync);
      for(u32 w=0;w<vsync;w++){
        swiWaitForVBlank();
      }
      IPC6->LCDPowerControl=LCDPC_SOFT_POWEROFF;
      while(1);
    }
  }
  
  if(PowerOffTimerWhileNoInput!=0){
	  //_consolePrintf("PowerOffTimerWhileNoInput=%d.\n",PowerOffTimerWhileNoInput);
      if(PowerOffTimerWhileNoInput<VsyncCount){
    	  PowerOffTimerWhileNoInput=0;
        }else{
        	PowerOffTimerWhileNoInput-=VsyncCount;
      }
      if(PowerOffTimerWhileNoInput==0){
        _consolePrint("No input timeout. Auto power off.\n");
        Sound_Start(WAVFN_PowerOff);
        u32 vsync=Sound_GetCurrentPlayTimePerVsync();
        _consolePrintf("FileList: PowerOffTimeOut: Wait for terminate. (%d)\n",vsync);
        for(u32 w=0;w<vsync;w++){
          swiWaitForVBlank();
        }
        IPC6->LCDPowerControl=LCDPC_SOFT_POWEROFF;
        while(1);
      }
  }
  
  PlayList_UpdateResume(VsyncCount);
  
  if(isPressMouseButtonTime!=0){
	  isPressMouseButtonTime+=VsyncCount;
	  if(isPressMouseButtonTime<1*30){
		  MP3Cnt_SeekNext=false;
		  MP3Cnt_SeekPrev=false;
	  }
  }
  
  if((MP3Cnt_SeekNext==true)||(MP3Cnt_SeekPrev==true)||(Process_SeekNext==true)||(Process_SeekPrev==true)||(HPSwitch_ProcessLong==true)||(HPSwitch_ProcessSingleLong==true)){
    if(Process_WaitCount<VsyncCount){
      Process_WaitCount=0;
      }else{
      Process_WaitCount-=VsyncCount;
    }
    if(Process_WaitCount==0){
      Process_WaitCount=8;
      s32 val=0;
      if(MP3Cnt_SeekPrev==true) val=-1;
      if(MP3Cnt_SeekNext==true) val=+1;
      if(Process_SeekPrev==true) val=-1;
      if(Process_SeekNext==true) val=+1;
      if(HPSwitch_ProcessLong==true) val=-1;
      if(HPSwitch_ProcessSingleLong==true) val=+1;
      if(DLLSound_SeekPer(val)==true){
        Popup_Show_Seek(val);
        DLLSound_SeekPer(val);
        ScreenRedrawFlag=true;
        ForceUpdateSubScreenFlag=true;
      }
    }
  }
  
  if(PanelClosePowerOffTimeOut==0){
    for(u32 idx=0;idx<VsyncCount;idx++){
      ScrollBar_MouseIdle(&ScrollBar,VsyncCount);
      FileList_MouseIdle(&ScrollBar,VsyncCount);
      
      if(MP3Cnt_WindowState==EMCWS_Lock){
    	  if(MP3Cnt_AutoFlag==true){
    		  MP3Cnt_LockTimeOut--;
    		  if(MP3Cnt_LockTimeOut==0) MP3Cnt_WindowState=EMCWS_Hide;
    	  }
      }
    }
    
    LongTapState_AddVsync(VsyncCount);
    
    if((PlayList_isOpened()==true)&&(PlayList_GetPause()==false)){
      ScrollBar.ShowPos=ScrollBar.TopPos;
    }
    
    if(RequestRefreshPlayCursorIndex==true){
      RequestRefreshPlayCursorIndex=false;
      ScreenRedrawFlag=true;
      ForceUpdateSubScreenFlag=true;
      PlayCursorIndex=-1;
      if(PlayList_isOpened()==true){
        if(Unicode_isEqual(ProcState.FileList.CurrentPathUnicode,PlayList_GetCurrentPath())==true){
          const u32 cnt=NDSFiles_GetFilesCount();
          for(s32 idx=0;idx<cnt;idx++){
            TNDSFile *pndsf=NDSFiles_GetFileBody(idx);
            if(Unicode_isEqual(pndsf->pFilenameUnicode,PlayList_GetCurrentFilename())==true){
              PlayCursorIndex=idx;
              break;
            }
          }
        }
      }
    }
    
    u32 subscrlimit;
    
    if(ScreenRedrawFlag==true){
      ScreenRedrawFlag=false;
      if(ForceUpdateSubScreenFlag==true){
        ForceUpdateSubScreenFlag=false;
        subscrlimit=0;
        }else{
        subscrlimit=1;
      }
      if(Backlight_isStandby()==false){
        if((PlayList_isOpened()==false)||(PlayList_GetPause()==true)){
          }else{
          if(DLLSound_isComplexDecoder()==false){
            }else{
            SetProcFadeEffect(EPFE_None);
          }
        }
        FileList_MainDrawBG(&ScrollBar);
      }
      }else{
      ProcState_RefreshSave();
      if((PlayList_isOpened()==false)||(PlayList_GetPause()==true)){
        subscrlimit=2;
        }else{
        if(DLLSound_isComplexDecoder()==false){
          subscrlimit=10;
          }else{
          subscrlimit=60;
        }
      }
    }
    
    {
    	DATA_IN_IWRAM_FileList static u32 redrawsubscrvsync=0;
      redrawsubscrvsync+=VsyncCount;
      if(subscrlimit<redrawsubscrvsync){
        redrawsubscrvsync=0;
        FileList_SubDrawBG(&ScrollBar);
      }
    }
    
  }
  
  if(NDSIconInfoLoaded==false){
    TScrollBar *psb=&ScrollBar;
    u32 idx=psb->ShowPos/psb->ItemHeight;
    u32 tag=idx;
    idx++;
    if(idx==NDSFiles_GetFilesCount()) idx=0;
    while(VBlankPassedFlag==false){
      NDSFiles_LoadNDSIcon(NDSFiles_GetFileBody(idx));
      NDSFiles_LoadFileInfo(NDSFiles_GetFileBody(idx));
      idx++;
      if(idx==NDSFiles_GetFilesCount()) idx=0;
      if(idx==tag){
    	NDSIconInfoLoaded=true;
        _consolePrint("All NDS icon loaded.\n");
        break;
      }
    }
  }
  
  if(strpcmRequestStop==true){
    strpcm_UseLoopOnOverflow=false;
    PlayList_Stop(true);
    
    if(PlayList_GetListEndFlag()){
    	switch(ProcState.Music.PlayListEnd){
    		case EPLE_Loop:{
    		} break;
    		case EPLE_Stop:{
    			Sound_Start(WAVFN_Notify);
    			PlayList_Stop(false);
    			PlayList_Free();
    			PlayList_MakeBlank();
    			
    			ScreenRedrawFlag=true;
    			ForceUpdateSubScreenFlag=true;
    			return;
    		} 
    		case EPLE_Off:{
			PlayList_Stop(false);
    			PlayList_Free();
    			PlayList_MakeBlank();
    			_consolePrint("Play List End. Auto power off.\n");
    			Sound_Start(WAVFN_PowerOff);
    			u32 vsync=Sound_GetCurrentPlayTimePerVsync();
    			_consolePrintf("Wait for terminate. (%d)\n",vsync);
    			for(u32 w=0;w<vsync;w++){
    				swiWaitForVBlank();
    			}
    			IPC6->LCDPowerControl=LCDPC_SOFT_POWEROFF;
    			while(1);
    		}
    	}
    }
    
    MP3Cnt_Exec_Next();
    DLLSound_UpdateLoop(false);
    strpcm_UseLoopOnOverflow=true;
    
    RequestRefreshPlayCursorIndex=true;
    
    ScreenRedrawFlag=true;
    ForceUpdateSubScreenFlag=true;
  }
}

