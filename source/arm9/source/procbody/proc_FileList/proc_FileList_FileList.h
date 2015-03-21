
#include "proc_FileList_FileList_Draw.h"
#include "proc_FileList_FileList_DrawID3Tag.h"
#include "proc_FileList_FileList_DrawLyric.h"
#include "proc_FileList_FileList_DrawHelp.h"

extern "C" {
void VRAMWriteCache_Enable(void);
void VRAMWriteCache_Disable(void);
}

static void FileList_MainDrawBG(TScrollBar *psb)
{
  if(Backlight_isStandby()==true) return;
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  CglCanvas *pTmpBM=pScreenMain->pBackCanvas;
  
  pDrawItemBM->SetCglFont(pCglFontDefault);
  
// ------------------------------
  
  if( (psb->ShowPos<0) || ((psb->MaxPos-psb->ShowPos)<psb->ClientSize) ){
    CglB15 *pb15=FileList_GetSkin(EFLS_BG_Bottom);
    pb15->pCanvas->BitBltFullBeta(pTmpBM);
  }
  
  {
    CglCanvas *pBGBM=FileList_GetSkin(EFLS_BG_Bottom)->pCanvas;
    
    const u32 cnt=NDSFiles_GetFilesCount();
    for(s32 idx=0;idx<cnt;idx++){
      s32 DrawHeight=psb->ItemHeight;
      s32 DrawTop=(idx*DrawHeight)-psb->ShowPos;
      if((-psb->ItemHeight<DrawTop)&&(DrawTop<psb->ClientSize)){
        if(psb->ClientSize<(DrawTop+DrawHeight)) DrawHeight=psb->ClientSize-DrawTop;
        
        bool Selected=false;
        if(idx==psb->SelectedIndex) Selected=true;
        
        bool isPlay=false;
        if(PlayCursorIndex==idx) isPlay=true;
        
        TNDSFile *pndsf=NDSFiles_GetFileBody(idx);
        if(pndsf->FileType==ENFFT_Folder){    	
        	if(Unicode_isEqual(PlayList_GetCurrentPath(),FullPathUnicodeFromSplitItem(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode))) isPlay=true;
        }
        
        s32 xofs;
        if(Selected==false){
        	xofs=0;
        }else{
        	xofs=psb->SelectXOfs;
        	if(xofs<-psb->SlideWidth) xofs=-psb->SlideWidth;
        	if(psb->SlideWidth<xofs) xofs=psb->SlideWidth;
        }
                
        pBGBM->BitBlt(pDrawItemBM,psb->SlideWidth-xofs,0,ScreenWidth,DrawHeight,0,DrawTop,false);
               
        FileList_DrawBG_DrawNDSFile(pDrawItemBM,psb->SlideWidth,NDSFiles_GetFileBody(idx),Selected,xofs,isPlay);
               
        pDrawItemBM->BitBlt(pTmpBM,0,DrawTop,ScreenWidth,DrawHeight,psb->SlideWidth-xofs,0,false);}
    }
    
    ScrollBar_Draw(psb,pTmpBM);
    
  }
  
// ------------------------------
  
  FileList_DrawBG_DrawHelp(pTmpBM);
  
  FileList_DrawMP3Cnt(pTmpBM,psb->SelectXOfs);
  
  LongTapState_DrawIcon(pTmpBM);

  ScreenMain_Flip_ProcFadeEffect();
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

static void FileList_SubDrawBG_Standby(TScrollBar *psb)
{
  pSubTempBM->SetCglFont(pCglFontDefault);
//  pSubTempBM->FillFull(RGB15(31,31,31)|BIT15);
  
  Clock_Standby_Draw(pSubTempBM);
  
  if(ProcState.ScreenSaver.ShowID3Tag==true) FileList_DrawID3Tag(pSubTempBM,true);
  if(ProcState.ScreenSaver.ShowLyric==true) FileList_DrawLyric(pSubTempBM,true);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  pSubTempBM->BitBltFullBeta(pScreenSub->pCanvas);
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

static void FileList_SubDrawBG(TScrollBar *psb)
{
  if((PlayList_isOpened()==false)||(PlayList_GetPause()==true)){
    }else{
    if(DLLSound_isComplexDecoder()==false){
      }else{
      pScreenSub->pCanvas->SetCglFont(pCglFontDefault);
      pScreenSub->pCanvas->FillFull(RGB15(0,0,0)|BIT15);
      FileList_DrawID3Tag(pScreenSub->pCanvas,false);
      FileList_DrawLyric(pScreenSub->pCanvas,false);
      return;
    }
  }
  
  if(Backlight_isStandby()==true){
    FileList_SubDrawBG_Standby(psb);
    return;
  }
  
  if(DrawIPK(pScreenSub->pCanvas)==true) return;
  
  if(DrawDPGThumb(pScreenSub->pCanvas)==true) return;
  
  pSubTempBM->SetCglFont(pCglFontDefault);
  
  pDrawItemBM->SetCglFont(pCglFontDefault);
// ------------------------------

  if(GlobalINI.FileList.SwapTopBottomDisplay==true){
    CglB15 *pb15=FileList_GetSkin(EFLS_BG_TopMsg);
    pb15->pCanvas->BitBltFullBeta(pSubTempBM);
    }else{
    if(psb->ShowPos<psb->ClientSize){
      CglB15 *pb15=FileList_GetSkin(EFLS_BG_TopMsg);
      pb15->pCanvas->BitBlt(pSubTempBM,0,0,ScreenWidth,psb->ClientSize-psb->ShowPos,0,0,false);
    }
    
    {
      CglCanvas *pBGBM=FileList_GetSkin(EFLS_BG_Top)->pCanvas;
      
      const u32 cnt=NDSFiles_GetFilesCount();
      for(s32 idx=0;idx<cnt;idx++){
        s32 DrawHeight=psb->ItemHeight;
        s32 DrawTop=(idx*DrawHeight)+psb->ClientSize-psb->ShowPos;
        if((-psb->ItemHeight<DrawTop)&&(DrawTop<psb->ClientSize)){
          if(psb->ClientSize<(DrawTop+DrawHeight)) DrawHeight=psb->ClientSize-DrawTop;
          
          bool Selected=false;
          if(idx==psb->SelectedIndex) Selected=true;
          
          bool isPlay=false;
          if(PlayCursorIndex==idx) isPlay=true;
          
          TNDSFile *pndsf=NDSFiles_GetFileBody(idx);
          if(pndsf->FileType==ENFFT_Folder){    	
        	  if(Unicode_isEqual(PlayList_GetCurrentPath(),FullPathUnicodeFromSplitItem(ProcState.FileList.CurrentPathUnicode,pndsf->pFilenameUnicode))) isPlay=true;
          }
          
          s32 xofs;
          if(Selected==false){
        	  xofs=0;
          }else{
        	  xofs=psb->SelectXOfs;
        	  if(xofs<-psb->SlideWidth) xofs=-psb->SlideWidth;
        	  if(psb->SlideWidth<xofs) xofs=psb->SlideWidth;
          }
                  
          pBGBM->BitBlt(pDrawItemBM,psb->SlideWidth-xofs,0,ScreenWidth,DrawHeight,0,DrawTop,false);
                  
          FileList_DrawBG_DrawNDSFile(pDrawItemBM,psb->SlideWidth,NDSFiles_GetFileBody(idx),Selected,xofs,isPlay);
                  
          pDrawItemBM->BitBlt(pSubTempBM,0,DrawTop,ScreenWidth,DrawHeight,psb->SlideWidth-xofs,0,false);
        }
      }
    }
  }
  
  // ------------------------------
  
  Clock_Draw(pSubTempBM);
  
  FileList_DrawID3Tag(pSubTempBM,true);
  FileList_DrawLyric(pSubTempBM,true);
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  pSubTempBM->BitBltFullBeta(pScreenSub->pCanvas);
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

DATA_IN_IWRAM_FileList static bool mb;
DATA_IN_IWRAM_FileList static s32 msx,msy;
DATA_IN_IWRAM_FileList static s32 mx,my;
DATA_IN_IWRAM_FileList static s32 movespeed;
DATA_IN_IWRAM_FileList static s32 mvsynccnt;
DATA_IN_IWRAM_FileList static s32 File_lx,File_ly;

static bool FileList_MouseDown(TScrollBar *psb,s32 x,s32 y)
{
  mb=true;
  msx=x;
  msy=y;
  mx=x;
  my=y;
  movespeed=0;
  mvsynccnt=0;
  
  psb->UsePressImage=true;
  
  s32 filecnt=psb->MaxPos/psb->ItemHeight;
  s32 fileidx=(psb->TopPos+my)/psb->ItemHeight;
  if(fileidx<filecnt) {
	File_lx=x;
	File_ly=y;
	ScrollBar_SetSelectedIndex(psb,fileidx);
	LongTapState_Start(File_lx,File_ly);
  }
  ScreenRedrawFlag=true;
  
  return(true);
}

static bool FileList_MouseMove(TScrollBar *psb,s32 x,s32 y)
{
  if(mb==false) return(false);
  
  if( (8<=abs(File_lx-x)) || (8<=abs(File_ly-y)) ) LongTapState_ExecStop();

  if(y!=my){
    s32 vec=my-y;
    ScrollBar_SetDirectTopPos(psb,psb->TopPos+vec);
    
    if(movespeed<vec){
      movespeed=vec;
      }else{
      movespeed=(movespeed+(vec*15))/16;
    }
  }
  
  if((x-mx)!=0){
      psb->SelectXOfs+=x-mx;
      
      TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
      
      if(pndsf->FileType==ENFFT_UnknownFile) psb->SelectXOfs=0;
      
      switch(pndsf->FileType){
        case ENFFT_UnknownFile: break;
        case ENFFT_UpFolder: break;
        case ENFFT_Folder: break;
        case ENFFT_Sound: {
          if(MP3Cnt_WindowState==EMCWS_Hide) MP3Cnt_WindowState=EMCWS_Link;
  /*
          if(MP3Cnt_WindowState==EMCWS_Link){
            MP3Cnt_PosX=0-psb->SelectXOfs;
            if(MP3Cnt_PosX<0) MP3Cnt_PosX=0;
            if(MP3Cnt_Width<MP3Cnt_PosX) MP3Cnt_PosX=MP3Cnt_Width;
          }
  */
        } break;
        case ENFFT_Image: break;
        case ENFFT_Text: break;
        case ENFFT_Video: break;
        case ENFFT_NDSROM: break;
        case ENFFT_GBAROM: break;
        case ENFFT_Skin: break;
      }
      
      ScreenRedrawFlag=true;
    }
  
  mx=x;
  my=y;
  
  return(true);
}

static bool FileList_MouseUp(TScrollBar *psb,s32 x,s32 y)
{
  if(mb==false) return(false);
  
  mb=false;
  
  LongTapState_ExecStop();

  psb->UsePressImage=false;
  
  if(16<movespeed) movespeed=16;
  
  if((-32<(y-msy))&&((y-msy)<32)){
      if(psb->SelectXOfs<-psb->SlideWidth){
        psb->SelectXOfs=-psb->SlideWidth;
        CustomApplication();
        if(MP3Cnt_WindowState==EMCWS_Link) MP3Cnt_WindowState=EMCWS_Hide;
        return(true);
      }
    
      if(psb->SlideWidth<psb->SelectXOfs){
        psb->SelectXOfs=psb->SlideWidth;
        StartApplication();
        if(MP3Cnt_WindowState==EMCWS_Link) MP3Cnt_WindowState=EMCWS_Hide;
        return(true);
      }
      }else{
  //    _consolePrintf("スクロールと判断したのでStart/Custom処理をしませんでした。(%dpixels)\n",y-msy);
  }
  
  s32 filecnt=psb->MaxPos/psb->ItemHeight;
  s32 fileidx=(psb->TopPos+y)/psb->ItemHeight;
    
  if(fileidx>=filecnt) return(true);
  
  if(MP3Cnt_WindowState==EMCWS_Link) MP3Cnt_WindowState=EMCWS_Hide;
  
  u32 marginlow=0,marginhigh=0;
  
  TNDSFile *pndsf=NDSFiles_GetFileBody(psb->SelectedIndex);
  ENDSFile_FileType FileType=pndsf->FileType;
  
  switch(FileType){
    case ENFFT_UnknownFile: break;
    case ENFFT_UpFolder: case ENFFT_Folder: marginlow=1; marginhigh=30; break;
    case ENFFT_Sound: case ENFFT_PlayList: case ENFFT_Image: case ENFFT_Text: case ENFFT_Video: case ENFFT_Skin: marginlow=1; marginhigh=30; break;
    case ENFFT_NDSROM: case ENFFT_GBAROM: marginlow=3; marginhigh=30; break;
  }
  
  if((-16<psb->SelectXOfs)&&(psb->SelectXOfs<16)){
      if((-16<(y-msy))&&((y-msy)<16)){
    	  if(marginlow<mvsynccnt){
    		  if(mvsynccnt<marginhigh){
    			  if(FileType==ENFFT_NDSROM){
    				  if(psb->LastSelectedIndex==psb->SelectedIndex){
    					  StartApplication();
    					  return(true);
    				  }
    				  if(VerboseDebugLog==true) _consolePrint("Ignored NDS file is selected. The place touched is different from the last cursor position.\n");
    			  }else{
    				  StartApplication();
    				  return(true);
    			  }
    		  }else{
//      			_consolePrintf("クリック時間が%dframes(%dms)以上だったので無視しました。(%dframes)\n",marginhigh,marginhigh*16,mvsynccnt);
    		  }
    	  }else{
//    _consolePrintf("クリック時間が%dframes(%dms)以下だったので無視しました。(%dframes)\n",marginlow,marginlow*16,mvsynccnt);
    	  }
      }else{
//      	_consolePrintf("スクロールと判断したのでクリック処理をしませんでした。(%dpixels)\n",y-msy);
      }
  }
  
  return(true);
}

static void FileList_MouseIdle(TScrollBar *psb,u32 VsyncCount)
{
  if(mb==true){
    mvsynccnt++;
    movespeed=(movespeed*7)/8;
    return;
  }
  
  if(psb->SelectXOfs!=0){
      psb->SelectXOfs=(psb->SelectXOfs*7)/8;
      
  /*
      if(MP3Cnt_WindowState==EMCWS_Link){
        TNDSFile *pndsf=&pNDSFiles[ScrollBar.SelectedIndex];
        if(pndsf->FileType!=ENFFT_Sound){
          MP3Cnt_WindowState=EMCWS_Hide;
          }else{
          if(psb->SelectXOfs!=0){
  //          MP3Cnt_PosX=0-psb->SelectXOfs;
            }else{
            MP3Cnt_WindowState=EMCWS_Hide;
          }
        }
      }
  */
      
      ScreenRedrawFlag=true;
    }
  
  if(movespeed!=0){
    ScrollBar_SetDirectTopPos(psb,psb->TopPos+movespeed);
    movespeed=(movespeed*31)/32;
    ScreenRedrawFlag=true;
  }
}

