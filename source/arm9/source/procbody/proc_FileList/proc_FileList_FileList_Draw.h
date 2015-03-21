
enum EMP3Cnt_WindowState {EMCWS_Hide,EMCWS_Link,EMCWS_Lock};

DATA_IN_IWRAM_FileList static EMP3Cnt_WindowState MP3Cnt_WindowState=EMCWS_Hide;
DATA_IN_IWRAM_FileList static s32 MP3Cnt_PosX;
DATA_IN_IWRAM_FileList static vu32 MP3Cnt_PosForVSync;

DATA_IN_IWRAM_FileList static bool MP3Cnt_AutoFlag;
DATA_IN_IWRAM_FileList static u32 MP3Cnt_LockTimeOut;
#define MP3Cnt_LockTimeOutSetup (60*4)

#define MP3Cnt_Width (64)
#define MP3Cnt_Height (ScreenHeight)

static void FileList_DrawMP3Cnt(CglCanvas *pDstBM,s32 SelectXOfs)
{
  if(MP3Cnt_PosX<=0) return;
	
  CglCanvas *pTmpBM=new CglCanvas(&MM_Temp,NULL,MP3Cnt_Width,MP3Cnt_Height,pf15bit);
  
  pDstBM->BitBlt(pTmpBM,0,0,MP3Cnt_Width,MP3Cnt_Height,ScreenWidth-MP3Cnt_Width,0,false);
  
  CglTGF *pbm;
  
  u32 y=0;
  
  if(MP3Cnt_AutoFlag==true){
	  pbm=MP3CntAlpha_GetSkin(EMP3SA_p0_Auto);
  }else{
	  pbm=MP3CntAlpha_GetSkin(EMP3SA_p0_Lock);
  }
  pbm->BitBlt(pTmpBM,0,y);
  y+=pbm->GetHeight();
  
  pbm=MP3CntAlpha_GetSkin(EMP3SA_p1_prev);
  pbm->BitBlt(pTmpBM,0,y);
  y+=pbm->GetHeight();
  
  if(PlayList_isOpened()==false){
    pbm=MP3CntAlpha_GetSkin(EMP3SA_p2_stop);
    }else{
    if(PlayList_GetPause()==false){
      pbm=MP3CntAlpha_GetSkin(EMP3SA_p2_pause);
      }else{
      pbm=MP3CntAlpha_GetSkin(EMP3SA_p2_play);
    }
  }
  pbm->BitBlt(pTmpBM,0,y);
  y+=pbm->GetHeight();
  
  pbm=MP3CntAlpha_GetSkin(EMP3SA_p3_next);
  pbm->BitBlt(pTmpBM,0,y);
  y+=pbm->GetHeight();
  
  {
    TProcState_FileList *pfl=&ProcState.FileList;
    switch(pfl->PlayMode){
      case EPSFLPM_Repeat: pbm=MP3CntAlpha_GetSkin(EMP3SA_p4_repeat); break;
      case EPSFLPM_AllRep: pbm=MP3CntAlpha_GetSkin(EMP3SA_p4_allrep); break;
      case EPSFLPM_Shuffle: pbm=MP3CntAlpha_GetSkin(EMP3SA_p4_shuffle); break;
      default: pbm=MP3CntAlpha_GetSkin(EMP3SA_p4_repeat); break;
    }
  }
  pbm->BitBlt(pTmpBM,0,y);
  y+=pbm->GetHeight();
  
  pTmpBM->BitBlt(pScreenMainOverlay->pCanvas,ScreenWidth-MP3Cnt_Width,0,MP3Cnt_Width,MP3Cnt_Height,0,0,false);
  
  if(pTmpBM!=NULL){
    delete pTmpBM; pTmpBM=NULL;
  }
}

static void FileList_DrawBG_DrawNDSFile(CglCanvas *pItemBM,s32 xofs,TNDSFile *pNDSFile,bool Selected,s32 scrxofs,bool isPlay)
{
  CglTGF *psrcbm=NULL;
  
  if(Selected==false){
    psrcbm=FileListAlpha_GetSkin(EFLSA_ItemBG_Clear);
    }else{
    psrcbm=FileListAlpha_GetSkin(EFLSA_ItemBG_Select);
  }
  
  DrawSkinAlpha(psrcbm,pItemBM,xofs,0);
  
  if(isPlay==true) DrawSkinAlpha(FileListAlpha_GetSkin(EFLSA_ItemBG_PlayIcon),pItemBM,xofs,0);
  
  s32 tx=0;
  switch(FileList_Mode){
    case EPSFLM_Single: tx=NDSROMIconXMargin+NDSROMIcon16Width+NDSROMIconXMargin; break;
    case EPSFLM_Double: tx=NDSROMIconXMargin+NDSROMIcon32Width+NDSROMIconXMargin; break;
  }
  
  NDSFiles_LoadNDSIcon(pNDSFile);
  NDSFiles_LoadFileInfo(pNDSFile);
  
  s32 ty;
  s32 ItemHeight=pItemBM->GetHeight();
  
  if(pNDSFile->pNDSROMIcon!=NULL){
    switch(FileList_Mode){
      case EPSFLM_Single: NDSROMIcon_DrawIcon16(pNDSFile->pNDSROMIcon,pItemBM,xofs+NDSROMIconXMargin,1); break;
      case EPSFLM_Double: NDSROMIcon_DrawIcon32(pNDSFile->pNDSROMIcon,pItemBM,xofs+NDSROMIconXMargin,1); break;
    }
    }else{
    psrcbm=pNDSFile->pIcon;
    DrawSkinAlpha(psrcbm,pItemBM,xofs+NDSROMIconXMargin,1);
  }
  
  s32 TextHeight=glCanvasTextHeight+2;
  
  if(Selected==false){
    switch(pNDSFile->FileType){
      case ENFFT_UpFolder: case ENFFT_Folder: {
        pItemBM->SetFontTextColor(ColorTable.FileList.FolderNameText);
      } break;
      default: {
        pItemBM->SetFontTextColor(ColorTable.FileList.FileNameText);
      } break;
    }
    }else{
    pItemBM->SetFontTextColor(ColorTable.FileList.SelectText);
  }
  
  switch(FileList_Mode){
    case EPSFLM_Single: {
      ty=(ItemHeight-TextHeight)/2;
      pItemBM->TextOutW(xofs+tx,ty+1,pNDSFile->pFilenameUnicode);
    } break;
    case EPSFLM_Double: {
      const UnicodeChar *plt0=pNDSFile->pFilenameUnicode_DoubleLine0;
      const UnicodeChar *plt1=pNDSFile->pFilenameUnicode_DoubleLine1;
      u32 fiposx=ScreenWidth-pNDSFile->FileInfoWidth;
      const char *pfi=pNDSFile->pFileInfo;
      if(pNDSFile->UseDoubleLine==false){
        ty=((ItemHeight-TextHeight)/2)+1;
        pItemBM->TextOutW(xofs+tx,ty,plt0);
        pItemBM->TextOutA(xofs+fiposx,ty,pfi);
        }else{
        ty=((ItemHeight-(TextHeight*2))/2)+1;
        pItemBM->TextOutW(xofs+tx,ty,plt0);
        ty+=TextHeight;
        pItemBM->TextOutW(xofs+tx,ty,plt1);
        pItemBM->TextOutA(xofs+fiposx,ty,pfi);
      }
    } break;
  }
  
  const char *pTitle_Left=NULL,*pTitle_Right=NULL;
  EFileListSkinAlpha FLSA_Left=EFLSACount,FLSA_Right=EFLSACount;
  
    switch(pNDSFile->FileType){
      case ENFFT_UnknownFile: break;
      case ENFFT_UpFolder: {
    	  pTitle_Left=pSlide_UpFolder_Title; pTitle_Right=pTitle_Left;
    	  FLSA_Left=EFLSA_Slide_UpFolder; FLSA_Right=FLSA_Left;
      } break;
      case ENFFT_Folder: {
    	  pTitle_Left=pSlide_Folder_Title; pTitle_Right=pTitle_Left;
    	  FLSA_Left=EFLSA_Slide_Folder; FLSA_Right=FLSA_Left;
      } break;
      case ENFFT_Sound: {
    	  pTitle_Left=pSlide_Sound_Left_Title;
    	  FLSA_Left=EFLSA_Slide_Sound_Left;
      } break;
      case ENFFT_Image: {
    	  pTitle_Left=pSlide_Image_Left_Title; 
    	  pTitle_Right=pSlide_Image_Right_Title;
    	  FLSA_Left=EFLSA_Slide_Image_Left; FLSA_Right=EFLSA_Slide_Image_Right;
      } break;
      case ENFFT_Text: {
    	  pTitle_Left=pSlide_Text_Left_Title; 
    	  pTitle_Right=pSlide_Text_Right_Title;
    	  FLSA_Left=EFLSA_Slide_Text_Left; FLSA_Right=EFLSA_Slide_Text_Right;
      } break;
      case ENFFT_Video: {
    	  pTitle_Left=pSlide_Video_Left_Title; 
    	  pTitle_Right=pSlide_Video_Right_Title;
    	  FLSA_Left=EFLSA_Slide_Video_Left; FLSA_Right=EFLSA_Slide_Video_Right;
      } break;
      case ENFFT_NDSROM: {
    	  pTitle_Left=pSlide_NDS_Left_Title; 
    	  pTitle_Right=pTitle_Left;
    	  FLSA_Left=EFLSA_Slide_NDS_Left; FLSA_Right=FLSA_Left;
    	  if(pNDSFile->isNDSCommercialROM) {
    		  pTitle_Right=pSlide_NDS_Right_Title;
    		  FLSA_Right=EFLSA_Slide_NDS_Right; 
    	  }
      } break;
      case ENFFT_GBAROM: {
    	  pTitle_Left=pSlide_NDS_Left_Title; 
		  pTitle_Right=pSlide_NDS_Right_Title;
    	  FLSA_Left=EFLSA_Slide_NDS_Left; FLSA_Right=EFLSA_Slide_NDS_Right;
      } break;
      case ENFFT_Skin: {
    	  pTitle_Left=pSlide_Skin_Title; 
    	  pTitle_Right=pTitle_Left;
    	  FLSA_Left=EFLSA_Slide_Skin; FLSA_Right=FLSA_Left;
      } break;
    }
    
    tx=0;
    s32 SlideWidth=32;
    ty=((ItemHeight-TextHeight)/2)+1;
    
    if(FileList_Mode==EPSFLM_Double) {
    	SlideWidth=64;
    	ty=ItemHeight-TextHeight;
    }
    	
    if(0<scrxofs){
      if(FLSA_Left!=EFLSACount){
        CglTGF *pbm=FileListAlpha_GetSkin(FLSA_Left);
        if(pbm!=NULL) DrawSkinAlpha(pbm,pItemBM,0,0);
        if(Skin_OwnerDrawText.SlideTitle==false) {
        	tx+=(SlideWidth-pItemBM->GetTextWidthUTF8(pTitle_Left))/2;
        	if(tx<0) tx=0;
        	pItemBM->TextOutUTF8(tx,ty,pTitle_Left);
        }
      }
    }
    if(scrxofs<0){
      if(FLSA_Right!=EFLSACount){
        CglTGF *pbm=FileListAlpha_GetSkin(FLSA_Right);
        if(pbm!=NULL) DrawSkinAlpha(pbm,pItemBM,xofs+ScreenWidth,0);
        if(Skin_OwnerDrawText.SlideTitle==false) {
        	tx+=(SlideWidth-pItemBM->GetTextWidthUTF8(pTitle_Right))/2;
        	if(tx<0) tx=0;
        	pItemBM->TextOutUTF8(tx+xofs+ScreenWidth,ty,pTitle_Right);
        }
      }
    }
}

