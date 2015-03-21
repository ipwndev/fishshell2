
#define namelen (128)

DATA_IN_AfterSystem static char section[namelen];
DATA_IN_AfterSystem static u32 readline;

static void readsection(char *str)
{
  str++;
  
  u32 ofs;
  
  ofs=0;
  while(*str!=']'){
    if((namelen<=ofs)||(*str==0)) StopFatalError(14303,"line%d error.\nThe section name doesn't end correctly.\n",readline);
    section[ofs]=*str;
    str++;
    ofs++;
  }
  section[ofs]=0;
}

static u16 GetColorCoord(const char *value)
{
  u32 v=0;
  
  while(1){
    char c=*value;
    if(c==0) break;
    
    bool use=false;
    
    if(('0'<=c)&&(c<='9')){
      use=true;
      v<<=4;
      v|=0x00+(c-'0');
    }
    if(('a'<=c)&&(c<='f')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'a');
    }
    if(('A'<=c)&&(c<='F')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'A');
    }
    
    if(use==false) break;
    
    value++;
  }
  
  u32 r,g,b;
  
  r=(v >> 16) & 0xff;
  g=(v >> 8) & 0xff;
  b=(v >> 0) & 0xff;
  
  return(RGB15(r/8,g/8,b/8) | BIT(15));
}

static void readkey(char *str)
{
  if(section[0]==0) StopFatalError(14304,"line%d error.\nThere is a key ahead of the section name.\n",readline);
  
  char key[namelen],value[namelen];
  
  u32 ofs;
  
  ofs=0;
  while(*str!='='){
    if((namelen<=ofs)||(*str==0)) StopFatalError(14305,"line%d error.\nThe key name doesn't end correctly.\n",readline);
    key[ofs]=*str;
    str++;
    ofs++;
  }
  key[ofs]=0;
  
  str++;
  
  ofs=0;
  while(*str!=0){
    if(namelen<=ofs) StopFatalError(14306,"line%d error.\nThe value doesn't end correctly.\n",readline);
    value[ofs]=*str;
    str++;
    ofs++;
  }
  value[ofs]=0;
  
  s32 ivalue=atoi(value);
  bool bvalue;
  
  if(ivalue==0){
    bvalue=false;
    }else{
    bvalue=true;
  }
  
  u16 colvalue=GetColorCoord(value);
  
  if(strcmp(section,"MetaData")==0){
    return;
  }
  
  if(strcmp(section,"OwnerDrawText")==0){
    TSkin_OwnerDrawText *psetup=&Skin_OwnerDrawText;
    if(strcmp(key,"Setup_Top")==0){
      psetup->Setup_Top=bvalue;
      return;
    }
    if(strcmp(key,"Setup_Bottom")==0){
      psetup->Setup_Bottom=bvalue;
      return;
    }
    if(strcmp(key,"BookmarkMenu")==0){
      psetup->BookmarkMenu=bvalue;
      return;
    }
    if(strcmp(key,"FileList_Top")==0){
      psetup->FileList_Top=bvalue;
      return;
    }
    if(strcmp(key,"SlideTitle")==0){
      psetup->SlideTitle=bvalue;
      return;
    }
    if(strcmp(key,"Custom_Top")==0){
    	psetup->Custom_Top=bvalue;
    	return;
    }
    if(strcmp(key,"DisableParticles")==0){
      psetup->DisableParticles=bvalue;
      return;
    }
  }
  
  if(strcmp(section,"Calender")==0){
    TSkin_Calender *psetup=&Skin_Calender;
    if(strcmp(key,"Enabled")==0){
      psetup->Enabled=bvalue;
      return;
    }
    if(strcmp(key,"PosX")==0){
      psetup->PosX=ivalue;
      return;
    }
    if(strcmp(key,"PosY")==0){
      psetup->PosY=ivalue;
      return;
    }
  }
  
  if(strcmp(section,"Setup")==0){
    TColorTable_Setup *psetup=&ColorTable.Setup;
    if(strcmp(key,"HelpTop_Text")==0){
      psetup->HelpTop_Text=colvalue;
      return;
    }
    if(strcmp(key,"HelpBody_Text")==0){
      psetup->HelpBody_Text=colvalue;
      return;
    }
    if(strcmp(key,"Label_Text")==0){
      psetup->Label_Text=colvalue;
      return;
    }
    if(strcmp(key,"Check_Text")==0){
      psetup->Check_Text=colvalue;
      return;
    }
    if(strcmp(key,"Button_NormalText")==0){
      psetup->Button_NormalText=colvalue;
      return;
    }
    if(strcmp(key,"Button_PressText")==0){
      psetup->Button_PressText=colvalue;
      return;
    }
  }
  
  if(strcmp(section,"Component")==0){
    TColorTable_Component *pcmp=&ColorTable.Component;
    if(strcmp(key,"HelpTop_Text")==0){
      pcmp->HelpTop_Text=colvalue;
      return;
    }
    if(strcmp(key,"HelpBody_Text")==0){
      pcmp->HelpBody_Text=colvalue;
      return;
    }
    if(strcmp(key,"Label_Text")==0){
      pcmp->Label_Text=colvalue;
      return;
    }
    if(strcmp(key,"TitleLabel_Text")==0){
      pcmp->TitleLabel_Text=colvalue;
      return;
    }
    if(strcmp(key,"Check_Text")==0){
      pcmp->Check_Text=colvalue;
      return;
    }
    if(strcmp(key,"Button_NormalHighlight")==0){
      pcmp->Button_NormalHighlight=colvalue;
      return;
    }
    if(strcmp(key,"Button_NormalShadow")==0){
      pcmp->Button_NormalShadow=colvalue;
      return;
    }
    if(strcmp(key,"Button_NormalBG")==0){
      pcmp->Button_NormalBG=colvalue;
      return;
    }
    if(strcmp(key,"Button_NormalText")==0){
      pcmp->Button_NormalText=colvalue;
      return;
    }
    if(strcmp(key,"Button_PressHighlight")==0){
      pcmp->Button_PressHighlight=colvalue;
      return;
    }
    if(strcmp(key,"Button_PressShadow")==0){
      pcmp->Button_PressShadow=colvalue;
      return;
    }
    if(strcmp(key,"Button_PressBG")==0){
      pcmp->Button_PressBG=colvalue;
      return;
    }
    if(strcmp(key,"Button_PressText")==0){
      pcmp->Button_PressText=colvalue;
      return;
    }
    if(strcmp(key,"SoftwareScrollBar_Frame")==0){
      pcmp->SoftwareScrollBar_Frame=colvalue;
      return;
    }
    if(strcmp(key,"SoftwareScrollBar_BG")==0){
      pcmp->SoftwareScrollBar_BG=colvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"ErrorDialog")==0){
    TColorTable_ErrorDialog *pct=&ColorTable.ErrorDialog;
    if(strcmp(key,"MessageText")==0){
      pct->MessageText=colvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"FileList")==0){
    TColorTable_FileList *pct=&ColorTable.FileList;
    if(strcmp(key,"HelpTop_Text")==0){
      pct->HelpTop_Text=colvalue;
      return;
    }
    if(strcmp(key,"HelpBody_Text")==0){
      pct->HelpBody_Text=colvalue;
      return;
    }
    if(strcmp(key,"DeleteFileDialog_Title_Text")==0){
      pct->DeleteFileDialog_Title_Text=colvalue;
      return;
    }
    if(strcmp(key,"DeleteFileDialog_Body_Text")==0){
      pct->DeleteFileDialog_Body_Text=colvalue;
      return;
    }
    if(strcmp(key,"FolderNameText")==0){
      pct->FolderNameText=colvalue;
      return;
    }
    if(strcmp(key,"FileNameText")==0){
      pct->FileNameText=colvalue;
      return;
    }
    if(strcmp(key,"SelectText")==0){
      pct->SelectText=colvalue;
      return;
    }
    if(strcmp(key,"PopupBG")==0){
      pct->PopupBG=colvalue;
      return;
    }
    if(strcmp(key,"PopupFrame")==0){
      pct->PopupFrame=colvalue;
      return;
    }
    if(strcmp(key,"PopupText")==0){
      pct->PopupText=colvalue;
      return;
    }
    if(strcmp(key,"ID3TagText")==0){
      pct->ID3TagText=colvalue;
      return;
    }
    
    if(strcmp(key,"ID3TagProgressBarAdd")==0){
        pct->ID3TagProgressBarAdd=colvalue;
        return;
    }
    
  }
  
  if(strcmp(section,"ScreenSaverCustom")==0){
    TColorTable_ScreenSaverCustom *pct=&ColorTable.ScreenSaverCustom;
    if(strcmp(key,"BG")==0){
      pct->BG=colvalue;
      return;
    }
    if(strcmp(key,"Label_Text")==0){
      pct->Label_Text=colvalue;
      return;
    }
    if(strcmp(key,"TitleLabel_Text")==0){
      pct->TitleLabel_Text=colvalue;
      return;
    }
    if(strcmp(key,"Check_Text")==0){
      pct->Check_Text=colvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"Launch")==0){
    TColorTable_Launch *pct=&ColorTable.Launch;
    if(strcmp(key,"EmptyText")==0){
      pct->EmptyText=colvalue;
      return;
    }
    if(strcmp(key,"FileNameText")==0){
      pct->FileNameText=colvalue;
      return;
    }
    if(strcmp(key,"NDSFileInfoText")==0){
      pct->NDSFileInfoText=colvalue;
      return;
    }
    if(strcmp(key,"SaveFileInfoText")==0){
      pct->SaveFileInfoText=colvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"SystemMenu")==0){
    TColorTable_SystemMenu *pct=&ColorTable.SystemMenu;
    if(strcmp(key,"ItemText")==0){
      pct->ItemText=colvalue;
      return;
    }
    if(strcmp(key,"ItemSelectText")==0){
      pct->ItemSelectText=colvalue;
      return;
    }
    
  }
  
  if(strcmp(section,"Video")==0){
    TColorTable_Video *pct=&ColorTable.Video;
    if(strcmp(key,"InitBG")==0){
      pct->InitBG=colvalue;
      return;
    }
    if(strcmp(key,"FilenameShadow")==0){
      pct->FilenameShadow=colvalue;
      return;
    }
    if(strcmp(key,"FilenameText")==0){
      pct->FilenameText=colvalue;
      return;
    }
    if(strcmp(key,"VolumeShadow")==0){
      pct->VolumeShadow=colvalue;
      return;
    }
    if(strcmp(key,"VolumeText")==0){
      pct->VolumeText=colvalue;
      return;
    }
    if(strcmp(key,"VolumeMaxShadow")==0){
      pct->VolumeMaxShadow=colvalue;
      return;
    }
    if(strcmp(key,"VolumeMaxText")==0){
      pct->VolumeMaxText=colvalue;
      return;
    }
    if(strcmp(key,"InfoBG")==0){
      pct->InfoBG=colvalue;
      return;
    }
    if(strcmp(key,"InfoText")==0){
      pct->InfoText=colvalue;
      return;
    }
    if(strcmp(key,"FrameCacheOff")==0){
      pct->FrameCacheOff=colvalue;
      return;
    }
    if(strcmp(key,"FrameCacheOn")==0){
      pct->FrameCacheOn=colvalue;
      return;
    }
    if(strcmp(key,"FrameCacheWarn")==0){
      pct->FrameCacheWarn=colvalue;
      return;
    }
  }
    
  if(strcmp(section,"TextView")==0){
    TColorTable_TextView *pct=&ColorTable.TextView;
    if(strcmp(key,"MainText")==0){
      pct->MainText=colvalue;
      return;
    }
    if(strcmp(key,"ScrollBar_Frame")==0){
      pct->ScrollBar_Frame=colvalue;
      return;
    }
    if(strcmp(key,"ScrollBar_Inside")==0){
      pct->ScrollBar_Inside=colvalue;
      return;
    }
    if(strcmp(key,"BookmarkMenu_TitleText")==0){
      pct->BookmarkMenu_TitleText=colvalue;
      return;
    }
    if(strcmp(key,"BookmarkMenu_Empty")==0){
      pct->BookmarkMenu_Empty=colvalue;
      return;
    }
    if(strcmp(key,"BookmarkMenu_Exists")==0){
      pct->BookmarkMenu_Exists=colvalue;
      return;
    }
    if(strcmp(key,"PreviewText")==0){
      pct->PreviewText=colvalue;
      return;
    }
    if(strcmp(key,"Line")==0){
      pct->Line=colvalue;
      return;
    }
    
    if(strcmp(key,"Bookmark0_FillBox")==0){
      pct->Bookmark_FillBox[0]=colvalue;
      return;
    }
    if(strcmp(key,"Bookmark1_FillBox")==0){
      pct->Bookmark_FillBox[1]=colvalue;
      return;
    }
    if(strcmp(key,"Bookmark2_FillBox")==0){
      pct->Bookmark_FillBox[2]=colvalue;
      return;
    }
    if(strcmp(key,"Bookmark3_FillBox")==0){
      pct->Bookmark_FillBox[3]=colvalue;
      return;
    }
    
  }
  
  {
    TColorTable_Calender *pct=NULL;
    if(strcmp(section,"Calender_Normal")==0) pct=&ColorTable.Calender_Normal;
    if(strcmp(section,"Calender_Digital")==0) pct=&ColorTable.Calender_Digital;
    if(strcmp(section,"Calender_Extend")==0) pct=&ColorTable.Calender_Extend;
    if(pct!=NULL){
      if(strcmp(key,"BG_Sunday")==0){
        pct->BG_Sunday=colvalue;
        return;
      }
      if(strcmp(key,"BG_Satday")==0){
        pct->BG_Satday=colvalue;
        return;
      }
      if(strcmp(key,"BG_Today")==0){
        pct->BG_Today=colvalue;
        return;
      }
      if(strcmp(key,"BG_Normal")==0){
        pct->BG_Normal=colvalue;
        return;
      }
      if(strcmp(key,"FrameOutColor")==0){
        pct->FrameOutColor=colvalue;
        return;
      }
      if(strcmp(key,"FrameInColor")==0){
        pct->FrameInColor=colvalue;
        return;
      }
    }
  }
  
  if(strcmp(section,"MemoList")==0){
      TColorTable_MemoList *pml=&ColorTable.MemoList;
      if(strcmp(key,"TitleText")==0){
        pml->TitleText=colvalue;
        return;
      }
      if(strcmp(key,"FileNumText")==0){
        pml->FileNumText=colvalue;
        return;
      }
      if(strcmp(key,"HelpText_Normal")==0){
        pml->HelpText_Normal=colvalue;
        return;
      }
      if(strcmp(key,"HelpText_Shadow")==0){
        pml->HelpText_Shadow=colvalue;
        return;
      }
      if(strcmp(key,"FilenameText")==0){
          pml->FilenameText=colvalue;
          return;
      }
      if(strcmp(key,"SelectText")==0){
          pml->SelectText=colvalue;
          return;
      }
    }
  
  if(strcmp(section,"MemoEdit")==0){
    TColorTable_MemoEdit *pme=&ColorTable.MemoEdit;
    if(strcmp(key,"HelpText_Normal")==0){
      pme->HelpText_Normal=colvalue;
      return;
    }
    if(strcmp(key,"HelpText_Shadow")==0){
      pme->HelpText_Shadow=colvalue;
      return;
    }
    if(strcmp(key,"FileInfo_Normal")==0){
      pme->FileInfo_Normal=colvalue;
      return;
    }
    if(strcmp(key,"FileInfo_Shadow")==0){
      pme->FileInfo_Shadow=colvalue;
      return;
    }
    
  }
  
  StopFatalError(14307,"line%d error.\ncurrent section [%s] unknown key=%s value=%s\n",readline,section,key,value);
}

static void internal_LoadGlobalINI(char *pini,u32 inisize)
{
  section[0]=0;
  readline=0;
  
  u32 iniofs=0;
  
  while(iniofs<inisize){
    
    readline++;
    
    u32 linelen=0;
    
    // Calc Line Length
    {
      char *s=&pini[iniofs];
      
      while(0x20<=*s){
        linelen++;
        s++;
        if(inisize<=(iniofs+linelen)) break;
      }
      *s=0;
    }
    
    if(linelen!=0){
      char c=pini[iniofs];
      if((c==';')||(c=='/')||(c=='!')){
        // comment line
        }else{
        if(c=='['){
          readsection(&pini[iniofs]);
          }else{
          readkey(&pini[iniofs]);
        }
      }
    }
    
    iniofs+=linelen;
    
    // skip NULL,CR,LF
    {
      char *s=&pini[iniofs];
      
      while(*s<0x20){
        iniofs++;
        s++;
        if(inisize<=iniofs) break;
      }
    }
    
  }
}

void LoadColorTable_colortbl_ini(void)
{
  MemSet8CPU(0,&Skin_OwnerDrawText,sizeof(TSkin_OwnerDrawText));
  MemSet8CPU(0,&Skin_Calender,sizeof(TSkin_Calender));
  
  u16 *pchkbuf=(u16*)&ColorTable;
  u32 chkbufsize=sizeof(TColorTable);
  
  MemSet16CPU(0,pchkbuf,chkbufsize);
  
  char *pbuf=NULL;
  s32 bufsize=0;
  
  const char fn[]="colortbl.ini";
  
  if(VerboseDebugLog==true) _consolePrintf("Load color table. [%s]\n",fn);
  
  u32 fidx=SkinFile_GetFileIndexFromFilename(fn);
  
  if(fidx==(u32)-1) StopFatalError(14301,"Skin file '%s' not found.\n",fn);
  
  SkinFile_LoadFileAllocate(fidx,(void**)&pbuf,&bufsize);
  
  if((pbuf==NULL)||(bufsize==0)) StopFatalError(14308,"File is empty. [%s]\n",fn);
  
  internal_LoadGlobalINI(pbuf,bufsize);
  
  safefree(&MM_Temp,pbuf); pbuf=NULL;
  
  if(ColorTable.FileList.SelectText==0) ColorTable.FileList.SelectText=ColorTable.FileList.FileNameText;
  
  //for(u32 idx=0;idx<chkbufsize/2;idx++){
  //  if((pchkbuf[idx]&BIT15)==0) StopFatalError(14302,"The item doesn't suffice for this file. '%s'\n",fn);
  //}
  
  ColorTable.Component.SoftwareScrollBar_BG=((ColorTable.Component.SoftwareScrollBar_BG & RGB15(30,30,30))>>1)|BIT15;
  ColorTable.TextView.ScrollBar_Inside=((ColorTable.TextView.ScrollBar_Inside & RGB15(30,30,30))>>1)|BIT15;
}

extern TColorTable_Calender* ColorTable_GetCalenderFromScreenSaver(EProcStateScreenSaver ScreenSaver)
{
  switch(ScreenSaver){
    case EPSSS_Normal: return(&ColorTable.Calender_Normal); 
    case EPSSS_Digital: return(&ColorTable.Calender_Digital); 
    case EPSSS_Extend: return(&ColorTable.Calender_Extend); 
  }
  
  StopFatalError(0,"Illigal ScreenSaver type.");
  return(NULL);
}

