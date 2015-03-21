
DATA_IN_IWRAM_FileList static bool isPressMouseButton;
DATA_IN_IWRAM_FileList static u32 isPressMouseButtonTime;
DATA_IN_IWRAM_FileList static bool isPressMouseButtonAndLRKey;

static void CB_MouseDown(s32 x,s32 y)
{
	if(Backlight_isStandby()==true){
		SSC_CB_MouseDown(x,y);
		return;
	}
  
	if(WaitKeyRelease) return;
	
  u32 KEYS_Cur=(~REG_KEYINPUT)&0x3ff;
  if((KEYS_Cur&(KEY_L|KEY_R))!=0){
    isPressMouseButtonAndLRKey=true;
    Sound_Start(WAVFN_Click);
    return;
  }
  
  isPressMouseButton=true;
  isPressMouseButtonTime=1;
  
  if(ErrorDialog_isExists()==true){
	ErrorDialog_Clear();
    pScreenMainOverlay->pCanvas->FillFull(0);
    return;
  }
  
  if(MP3Cnt_WindowState==EMCWS_Lock){
    if((ScreenWidth-MP3Cnt_Width)<=x){
      u32 hlst[5];
      hlst[0]=MP3CntAlpha_GetSkin(EMP3SA_p0_Auto)->GetHeight();
      hlst[1]=MP3CntAlpha_GetSkin(EMP3SA_p1_prev)->GetHeight();
      hlst[2]=MP3CntAlpha_GetSkin(EMP3SA_p2_stop)->GetHeight();
      hlst[3]=MP3CntAlpha_GetSkin(EMP3SA_p3_next)->GetHeight();
      hlst[4]=MP3CntAlpha_GetSkin(EMP3SA_p4_repeat)->GetHeight();
      s32 exec=-1;
      u32 h=0;
      for(s32 idx=0;idx<5;idx++){
        h+=hlst[idx];
        if(y<h){
          exec=idx;
          break;
        }
      }
      if(exec!=-1){
        switch(exec){
          case 0: MP3Cnt_Exec_ChangeAuto(); break;
          case 1: MP3Cnt_SeekPrev=true; break;
          case 2: MP3Cnt_Exec_ChangePause(); break;
          case 3: MP3Cnt_SeekNext=true; break;
          case 4: MP3Cnt_Exec_ChangePlayMode(); break;
        }
        MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
        ScreenRedrawFlag=true;
        return;
      }
    }
  }
  
  if(ScrollBar_MouseDown(&ScrollBar,x,y)==true) return;
  if(FileList_MouseDown(&ScrollBar,x,y)==true) return;
}

static void CB_MouseMove(s32 x,s32 y)
{
    if(Backlight_isStandby()==true){
        SSC_CB_MouseMove(x,y);
        return;
      }
  
  if(isPressMouseButton==false) return;
  
  if(MP3Cnt_WindowState==EMCWS_Lock && ScrollBar.SelectXOfs==0){
        if((ScreenWidth-MP3Cnt_Width)<=x){
          u32 hlst[5];
          hlst[0]=MP3CntAlpha_GetSkin(EMP3SA_p0_Auto)->GetHeight();
          hlst[1]=MP3CntAlpha_GetSkin(EMP3SA_p1_prev)->GetHeight();
          hlst[2]=MP3CntAlpha_GetSkin(EMP3SA_p2_stop)->GetHeight();
          hlst[3]=MP3CntAlpha_GetSkin(EMP3SA_p3_next)->GetHeight();
          hlst[4]=MP3CntAlpha_GetSkin(EMP3SA_p4_repeat)->GetHeight();
          s32 exec=-1;
          u32 h=0;
          for(s32 idx=0;idx<5;idx++){
            h+=hlst[idx];
            if(y<h){
              exec=idx;
              break;
            }
          }
          if(exec!=-1){
            switch(exec){
              case 0: case 2: case 4: {
            	  MP3Cnt_SeekNext=false;
            	  MP3Cnt_SeekPrev=false;
              } break;
              case 1: {
            	  MP3Cnt_SeekPrev=true;
              } break;
              case 3: {
            	  MP3Cnt_SeekNext=true;
              } break;
            }
            MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
            ScreenRedrawFlag=true;
            return;
          }
        }
  }
  
  MP3Cnt_SeekNext=false;
  MP3Cnt_SeekPrev=false;
  
  if(ScrollBar_MouseMove(&ScrollBar,x,y)==true) return;
  if(FileList_MouseMove(&ScrollBar,x,y)==true) return;
}

static void CB_MouseUp(s32 x,s32 y)
{
    if(Backlight_isStandby()==true){
        SSC_CB_MouseUp(x,y);
        return;
    }
  
  if(isPressMouseButtonAndLRKey==true){
    isPressMouseButtonAndLRKey=false;
    RelationalFile_Clear();
    SetNextProc(ENP_MemoEdit,EPFE_CrossFade);
    return;
  }
  
  if(isPressMouseButton==false) return;
  isPressMouseButton=false;
  
  if(MP3Cnt_WindowState==EMCWS_Lock){
      if((ScreenWidth-MP3Cnt_Width)<=x){
        u32 hlst[5];
        hlst[0]=MP3CntAlpha_GetSkin(EMP3SA_p0_Auto)->GetHeight();
        hlst[1]=MP3CntAlpha_GetSkin(EMP3SA_p1_prev)->GetHeight();
        hlst[2]=MP3CntAlpha_GetSkin(EMP3SA_p2_stop)->GetHeight();
        hlst[3]=MP3CntAlpha_GetSkin(EMP3SA_p3_next)->GetHeight();
        hlst[4]=MP3CntAlpha_GetSkin(EMP3SA_p4_repeat)->GetHeight();
        s32 exec=-1;
        u32 h=0;
        for(s32 idx=0;idx<5;idx++){
          h+=hlst[idx];
          if(y<h){
            exec=idx;
            break;
          }
        }
        if(exec!=-1){
          switch(exec){
            case 0: case 2: case 4: break;
            case 1: {
            	if(isPressMouseButtonTime<1*30) MP3Cnt_Exec_Prev();
            } break;
            case 3: {
            	if(isPressMouseButtonTime<1*30) MP3Cnt_Exec_Next();
            } break;
          }
          
          MP3Cnt_SeekNext=false;
          MP3Cnt_SeekPrev=false;
          MP3Cnt_LockTimeOut=MP3Cnt_LockTimeOutSetup;
          ScreenRedrawFlag=true;
          return;
        }
      }
  }

  MP3Cnt_SeekNext=false;
  MP3Cnt_SeekPrev=false;
  isPressMouseButtonTime=0;
  
  if(ScrollBar_MouseUp(&ScrollBar,x,y)==true) return;
  if(FileList_MouseUp(&ScrollBar,x,y)==true) return;
}

