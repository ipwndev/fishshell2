
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"

#include "datetime.h"
#include "strpcm.h"
#include "lang.h"
#include "extlink.h"
#include "skin.h"

#include "procstate.h"
#include "strtool.h"
#include "unicode.h"
#include "inifile.h"

#include "dll.h"
#include "dllsound.h"
#include "internaldrivers.h"
#include "BootROM.h"

#include "glib.h"

#include "hiddenpaths.h"

#include "NDSFiles.h"

DATA_IN_IWRAM_FileList bool ChangedCurrentPath;

DATA_IN_IWRAM_FileList bool NDSIconInfoLoaded;

DATA_IN_IWRAM_FileList static s32 NDSFilesCount;
DATA_IN_IWRAM_FileList static TNDSFile *pNDSFiles;

#include "NDSFiles_TextPool.h"

void NDSFiles_Free(void)
{
  if(pNDSFiles!=NULL){
    for(s32 idx=0;idx<NDSFilesCount;idx++){
        DLLSound_UpdateLoop(true);
      TNDSFile *pfile=&pNDSFiles[idx];
      pfile->pFilenameAlias=NULL;
      pfile->pFilenameUnicode=NULL;
      pfile->pFilenameUnicode_DoubleLine0=NULL;
      pfile->pFilenameUnicode_DoubleLine1=NULL;
      pfile->pJpnTitle = NULL;
      pfile->pEngTitle = NULL;
      pfile->pFileInfo=NULL;
      if(pfile->pNDSROMIcon!=NULL){
        safefree(&MM_Process,pfile->pNDSROMIcon); pfile->pNDSROMIcon=NULL;
      }
      pfile->pIcon=NULL;
    }
    pNDSFiles=NULL;
  }
  
  NDSFilesCount=0;
  
  FreeTextPool();
}

static bool isHiddenExt32(u32 Ext32)
{
  if(ProcState.FileList.HiddenNotSupportFileType==false) return(false);
  
#ifdef ExceptMP3
  if(Ext32==MakeExt32(0,'M','P','3')) return(false);
#endif

#ifdef ExceptMIDI
#endif
  
#ifdef ExceptGME
#endif
  
#ifdef ExceptOGG
#endif
  
#ifdef ExceptWAVE
#endif
  
#ifdef ExceptJpeg
  if(Ext32==MakeExt32(0,'J','P','G')) return(false);
#endif

#ifdef ExceptBmp
  if(Ext32==MakeExt32(0,'B','M','P')) return(false);
#endif

#ifdef ExceptGif
  if(Ext32==MakeExt32(0,'G','I','F')) return(false);
#endif

#ifdef ExceptPsd
  if(Ext32==MakeExt32(0,'P','S','D')) return(false);
#endif
  
#ifdef ExceptPng
  if(Ext32==MakeExt32(0,'P','N','G')) return(false);
#endif
    
  if(Ext32==MakeExt32(0,'N','D','S')) return(false);
  if(Ext32==MakeExt32(0,'I','D','S')) return(false);
  
  if(Ext32==MakeExt32(0,'G','B','A')) return(false);

  if(ExtLink_GetTargetIndex(Ext32)!=(u32)-1) return(false);
  
  if(Ext32==MakeExt32(0,'D','P','G')) return(false);
  
  if(Ext32==MakeExt32(0,'S','K','N')) return(false);
  
  if(Ext32==MakeExt32(0,'M','3','U')) return(false);
  if(Ext32==MakeExt32(0,'W','P','L')) return(false);
  
  if(Ext32==MakeExt32(0,'T','X','T')) return(false);
  if(Ext32==MakeExt32(0,'D','O','C')) return(false);
  if(Ext32==MakeExt32(0,'I','N','I')) return(false);
  if(Ext32==MakeExt32(0,0,0,'C')) return(false);
  if(Ext32==MakeExt32(0,0,0,'S')) return(false);
  if(Ext32==MakeExt32(0,'C','P','P')) return(false);
  if(Ext32==MakeExt32(0,0,0,'H')) return(false);
  if(Ext32==MakeExt32(0,'M','M','L')) return(false);
  if(Ext32==MakeExt32(0,'F','R','M')) return(false);
  if(Ext32==MakeExt32(0,'H','T','M')) return(false);
  if(Ext32==MakeExt32(0,'N','F','O')) return(false);
  if(Ext32==MakeExt32(0,'D','I','Z')) return(false);
  //if(Ext32==MakeExt32(0,'L','R','C')) return(false);
  
  switch(DLLList_isSupportFormatExt32(Ext32)){
    case EPT_None: break;
    case EPT_Image: return(false); 
    case EPT_Sound: return(false); 
  }
  
  return(true);
}

#include "NDSFiles_GetFileInfo.h"

#define ATTRIB_ARCH  0x20
#define ATTRIB_HID  0x02
#define ATTRIB_SYS  0x04
#define ATTRIB_RO  0x01

void NDSFiles_RefreshCurrentFolder(void)
{
  NDSFiles_Free();
  
  NDSIconInfoLoaded=false;
  
  InitTextPool();
  
  const char *pBasePathAlias=ConvertFull_Unicode2Alias(ProcState.FileList.CurrentPathUnicode,NULL);
  
  if((pBasePathAlias==NULL)||(FAT2_chdir_Alias(pBasePathAlias)==false)) StopFatalError(17004,"Can not change path. [%s]\n",pBasePathAlias);
  
  NDSFilesCount=0;
  
  u8 AttribChk=0;
  
  if(ProcState.FileList.HideAttribute_Archive) AttribChk|=ATTRIB_ARCH;
  if(ProcState.FileList.HideAttribute_Hidden) AttribChk|=ATTRIB_HID;
  if(ProcState.FileList.HideAttribute_System) AttribChk|=ATTRIB_SYS;
  if(ProcState.FileList.HideAttribute_ReadOnly) AttribChk|=ATTRIB_RO;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      DLLSound_UpdateLoop(true);
      if((FAT2_GetAttrib()&AttribChk)==0){
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            if(strcmp(pafn,".")==0){
              }else{
              NDSFilesCount++;
            }
          } break;
          case FT_FILE: {
            u32 Ext32=0;
            {
              const char *ptmp=pafn;
              while(*ptmp!=0){
                u32 ch=*ptmp++;
                if(ch==(u32)'.'){
                  Ext32=0;
                  }else{
                  if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
                  Ext32=(Ext32<<8)|ch;
                }
              }
            }
            
            if(isHiddenExt32(Ext32)==false){
              NDSFilesCount++;
            }
          } break;
        }
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  pNDSFiles=(TNDSFile*)GetTextPoolVoid(sizeof(TNDSFile)*NDSFilesCount);
  
  for(s32 idx=0;idx<NDSFilesCount;idx++){
      DLLSound_UpdateLoop(true);
    TNDSFile *pfile=&pNDSFiles[idx];
    pfile->FileType=ENFFT_UnknownFile;
    pfile->Ext32=0;
    pfile->FileSize=0;
    pfile->pFilenameAlias=NULL;
    pfile->pFilenameUnicode=NULL;
    pfile->pFilenameUnicode_DoubleLine0=NULL;
    pfile->pFilenameUnicode_DoubleLine1=NULL;
    pfile->pJpnTitle = NULL;
    pfile->pEngTitle = NULL;
    pfile->pFileInfo=NULL;
    pfile->UseDoubleLine=false;
    pfile->isNDSCommercialROM=false;
    pfile->pNDSROMIcon=NULL;
    pfile->pIcon=NULL;
  }
  
  NDSFilesCount=0;
  
  {
    const char *pafn;
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      DLLSound_UpdateLoop(true);
      if((FAT2_GetAttrib()&AttribChk)==0){
        const UnicodeChar *pufn=FAT2_GetLongFilenameUnicode();
        if(pufn==NULL) StopFatalError(17002,"Can not read unicode filename.\n");
        switch(FAT_FileType){
          case FT_NONE: break;
          case FT_DIR: {
            if(strcmp(pafn,".")==0){
              }else{
              if((strcmp(pafn,"..")==0)){
                TNDSFile *pfile=&pNDSFiles[NDSFilesCount];
                pfile->FileType=ENFFT_UpFolder;
                pfile->pFilenameAlias=TextPoolChar_AllocateCopy("..");
                pfile->pFilenameUnicode=TextPoolUnicode_AllocateCopy(ProcState.FileList.CurrentPathUnicode);
                NDSFilesCount++;
                }else{
                const UnicodeChar *pfullpath=FullPathUnicodeFromSplitItem(ProcState.FileList.CurrentPathUnicode,pufn);
                if(HiddenPaths_isHidden(pfullpath)==true){
                  _consolePrintf("Hide path. [%s]\n",StrConvert_Unicode2Ank_Test(pfullpath));
                  }else{
                  TNDSFile *pfile=&pNDSFiles[NDSFilesCount];
                  pfile->FileType=ENFFT_Folder;
                  pfile->pFilenameAlias=TextPoolChar_AllocateCopy(pafn);
                  pfile->pFilenameUnicode=TextPoolUnicode_AllocateCopy(pufn);
                  NDSFilesCount++;
                }
              }
            }
          } break;
          case FT_FILE: {
            u32 Ext32=0;
            {
              const char *ptmp=pafn;
              while(*ptmp!=0){
                u32 ch=*ptmp++;
                if(ch==(u32)'.'){
                  Ext32=0;
                  }else{
                  if((0x61<=ch)&&(ch<=0x7a)) ch-=0x20;
                  Ext32=(Ext32<<8)|ch;
                }
              }
            }
            
            if(isHiddenExt32(Ext32)==false){
              TNDSFile *pfile=&pNDSFiles[NDSFilesCount];
              pfile->FileType=ENFFT_UnknownFile;
              pfile->Ext32=Ext32;
              pfile->FileSize=FAT2_CurEntry_GetFileSize();
              pfile->pFilenameAlias=TextPoolChar_AllocateCopy(pafn);
              pfile->pFilenameUnicode=TextPoolUnicode_AllocateCopy(pufn);
              pfile->pFileInfo=GetTextPoolChar(0x10) ;
              pfile->pFileInfo[0]=0;
              pfile->FileInfoWidth=(u32)-1;
              NDSFilesCount++;
            }
          } break;
        }
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  
  if(2<=NDSFilesCount){
      if(VerboseDebugLog==true) _consolePrint("Sort for filenames.\n");
    for(s32 idx0=0;idx0<NDSFilesCount-1;idx0++){
      DLLSound_UpdateLoop(true);
      for(s32 idx1=idx0+1;idx1<NDSFilesCount;idx1++){
          DLLSound_UpdateLoop(true);
        TNDSFile *pf0=&pNDSFiles[idx0];
        TNDSFile *pf1=&pNDSFiles[idx1];
        ENDSFile_FileType ft0=pf0->FileType;
        ENDSFile_FileType ft1=pf1->FileType;
        
        if(ft0==ft1){
          if(isSwapFilenameUnicode(&pf0->pFilenameUnicode[0],&pf1->pFilenameUnicode[0])==true){
            TNDSFile ftemp=*pf0;
            *pf0=*pf1;
            *pf1=ftemp;
          }
          }else{
          if(ft0==ENFFT_UpFolder){
            }else{
            if(ft1==ENFFT_UpFolder){
              TNDSFile ftemp=*pf0;
              *pf0=*pf1;
              *pf1=ftemp;
              }else{
              if((ft0==ENFFT_UnknownFile)&&(ft1==ENFFT_Folder)){
                TNDSFile ftemp=*pf0;
                *pf0=*pf1;
                *pf1=ftemp;
              }
            }
          }
        }
      }
    }
    
    if(VerboseDebugLog==true) _consolePrint("End of sort.\n");
  }
  
  if(VerboseDebugLog==true) _consolePrint("Load icons.\n");
  for(s32 idx=0;idx<NDSFilesCount;idx++){
    DLLSound_UpdateLoop(true);
    TNDSFile *pfile=&pNDSFiles[idx];
//    _consolePrintf("%d/%d %s",idx,NDSFilesCount,pfile->pFilenameAlias);
    
    if(pfile->FileType==ENFFT_UnknownFile){
      if(pfile->Ext32==MakeExt32(0,'N','D','S')) pfile->FileType=ENFFT_NDSROM;
      if(pfile->Ext32==MakeExt32(0,'I','D','S')) pfile->FileType=ENFFT_NDSROM;
    }
    
    if(pfile->FileType==ENFFT_UnknownFile){
    	if(pfile->Ext32==MakeExt32(0,'G','B','A')) pfile->FileType=ENFFT_GBAROM;
    }
    
    if(ExtLink_GetTargetIndex(pfile->Ext32)!=(u32)-1) pfile->FileType=ENFFT_NDSROM;
    
    if(pfile->FileType==ENFFT_UnknownFile){
      if(pfile->Ext32==MakeExt32(0,'D','P','G')) pfile->FileType=ENFFT_Video;
    }
    
    if(pfile->FileType==ENFFT_UnknownFile){
      if(pfile->Ext32==MakeExt32(0,'S','K','N')) pfile->FileType=ENFFT_Skin;
    }
    
    if(pfile->FileType==ENFFT_UnknownFile){
      if(pfile->Ext32==MakeExt32(0,'T','X','T')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'D','O','C')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'I','N','I')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,0,0,'C')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,0,0,'S')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'C','P','P')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,0,0,'H')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'M','M','L')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'F','R','M')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'H','T','M')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'N','F','O')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'D','I','Z')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'L','R','C')) pfile->FileType=ENFFT_Text;
      if(pfile->Ext32==MakeExt32(0,'S','R','T')) pfile->FileType=ENFFT_Text;
    }
    
    if(pfile->FileType==ENFFT_UnknownFile){
      if(pfile->Ext32==MakeExt32(0,'M','3','U')) pfile->FileType=ENFFT_PlayList;
      if(pfile->Ext32==MakeExt32(0,'W','P','L')) pfile->FileType=ENFFT_PlayList;
    }
    
    if(pfile->FileType==ENFFT_UnknownFile){
      switch(DLLList_isSupportFormatExt32(pfile->Ext32)){
        case EPT_None: break;
        case EPT_Image: pfile->FileType=ENFFT_Image; break;
        case EPT_Sound: pfile->FileType=ENFFT_Sound; break;
      }
    }
    
    switch(pfile->FileType){
      case ENFFT_UnknownFile: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_UnknownFile); break;
      case ENFFT_UpFolder: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_UpFolder); break;
      case ENFFT_Folder: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Folder); break;
      case ENFFT_Sound: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Sound); break;
      case ENFFT_PlayList: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Sound); break;
      case ENFFT_Image: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Image); break;
      case ENFFT_Text: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Text); break;
      case ENFFT_Video: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Video); break;
      case ENFFT_NDSROM: {
          if(pfile->pJpnTitle == NULL)
              pfile->pJpnTitle = GetTextPoolUnicode(0x100) ;
          if(pfile->pEngTitle == NULL)
              pfile->pEngTitle = GetTextPoolUnicode(0x100) ;
      } break; // load after.
      case ENFFT_GBAROM: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_GBAROM); break;
      case ENFFT_Skin: pfile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_Skin); break;
      default: StopFatalError(17005,"Illigal FileType. (%d)\n",pfile->FileType); break;
    }
  }
  
  CglCanvas *pbm=new CglCanvas(&MM_Temp,NULL,1,1,pf15bit);
  pbm->SetCglFont(pCglFontDefault);
  
  // Load after.
  /*if(VerboseDebugLog==true) _consolePrint("Get file info.\n");
  for(s32 idx=0;idx<NDSFilesCount;idx++){
    DLLSound_UpdateLoop(true);
    TNDSFile *pndsf=&pNDSFiles[idx];
    NDSFiles_RefreshCurrentFolder_ins_GetFileInfo(pndsf);
    if(str_isEmpty(pndsf->pFileInfo)==true){
      pndsf->FileInfoWidth=0;
      }else{
      pndsf->FileInfoWidth=pbm->GetTextWidthA(pndsf->pFileInfo)+16;
    }
  }*/
  
  if(VerboseDebugLog==true) _consolePrint("Demiliter long line.\n");
  
  for(s32 idx=0;idx<NDSFilesCount;idx++){
    DLLSound_UpdateLoop(true);
    TNDSFile *pfile=&pNDSFiles[idx];
    
    pfile->pFilenameUnicode_DoubleLine0=NULL;
    pfile->pFilenameUnicode_DoubleLine1=NULL;
    if(Unicode_isEmpty(pfile->pFilenameUnicode)==false){
      bool HideExt=ProcState.FileList.HiddenFilenameExt;
      if(HideExt==true){
        if((pfile->FileType==ENFFT_UpFolder)||(pfile->FileType==ENFFT_Folder)) HideExt=false;
      }
      
      UnicodeChar *psrc=pfile->pFilenameUnicode;
      if((HideExt==true)){
        psrc=Unicode_AllocateCopy(&MM_Temp,psrc);
        u32 dotpos=0;
        u32 idx=0;
        while(psrc[idx]!=0){
          if(psrc[idx]==(UnicodeChar)'.') dotpos=idx;
          idx++;
        }
        if(dotpos!=0) psrc[dotpos]=0;
      }
      u32 srclen=Unicode_GetLength(psrc);
      u32 tx=NDSROMIconXMargin+NDSROMIcon32Width+NDSROMIconXMargin;
      if((tx+pbm->GetTextWidthW(psrc))<=ScreenWidth){
        pfile->pFilenameUnicode_DoubleLine0=GetTextPoolUnicode(srclen);
        Unicode_Copy(pfile->pFilenameUnicode_DoubleLine0,psrc);
        pfile->pFilenameUnicode_DoubleLine1=NULL;
        
	//Load after
        /*if((tx+pbm->GetTextWidthW(pfile->pFilenameUnicode_DoubleLine0))<(ScreenWidth-pfile->FileInfoWidth)){
          pfile->UseDoubleLine=false;
          }else{
          pfile->UseDoubleLine=true;
        }*/
        
        }else{
        
        u32 limlen=0;
        while(1){
          limlen++;
          UnicodeChar ustr[MaxFilenameLength+1];
          for(u32 idx=0;idx<limlen;idx++){
            ustr[idx]=psrc[idx];
          }
          ustr[limlen]=0;
          if(ScreenWidth<(tx+pbm->GetTextWidthW(ustr))){
            limlen--;
            break;
          }
        }
        pfile->pFilenameUnicode_DoubleLine0=GetTextPoolUnicode(limlen);
        pfile->pFilenameUnicode_DoubleLine1=GetTextPoolUnicode(srclen-limlen);
        for(u32 idx=0;idx<limlen;idx++){
          pfile->pFilenameUnicode_DoubleLine0[idx]=psrc[idx];
        }
        pfile->pFilenameUnicode_DoubleLine0[limlen]=0;
        u32 ofs=limlen;
        limlen=srclen-limlen;
        for(u32 idx=0;idx<limlen;idx++){
          pfile->pFilenameUnicode_DoubleLine1[idx]=psrc[ofs+idx];
          pfile->pFilenameUnicode_DoubleLine1[idx+1]=0;
          if((ScreenWidth-pfile->FileInfoWidth)<(tx+pbm->GetTextWidthW(pfile->pFilenameUnicode_DoubleLine1))){
              pfile->pFilenameUnicode_DoubleLine1[idx]=0;
              break;
          }
        }
        //pfile->pFilenameUnicode_DoubleLine1[limlen]=0;
        
        pfile->UseDoubleLine=true;
      }
      if(HideExt==true){
        if(psrc!=NULL){
          safefree(&MM_Temp,psrc); psrc=NULL;
        }
      }
    }
  }
  
  if(pbm!=NULL){
    delete pbm; pbm=NULL;
  }
  
  EndTextPool();
  PrintFreeMem();
  if(VerboseDebugLog==true) _consolePrint("Current folder refreshed.\n");
}

void NDSFiles_LoadNDSIcon(TNDSFile *pNDSFile)
{
  if(pNDSFile->FileType!=ENFFT_NDSROM) return;
  
  if((pNDSFile->pIcon!=NULL)||(pNDSFile->pNDSROMIcon!=NULL)) return;
  
  if(ChangedCurrentPath==true){
    ChangedCurrentPath=false;
    if(PathExistsUnicode(ProcState.FileList.CurrentPathUnicode)==false) StopFatalError(17003,"Can not move current folder.\n");
  }
  
  pNDSFile->pNDSROMIcon=(TNDSROMIcon*)safemalloc_chkmem(&MM_Process,sizeof(TNDSROMIcon));
  
  if((pNDSFile->Ext32==MakeExt32(0,'N','D','S'))|| (pNDSFile->Ext32==MakeExt32(0,'I','D','S'))){
    if(NDSROMIcon_Get(pNDSFile->pFilenameAlias,pNDSFile->pNDSROMIcon,pNDSFile->pJpnTitle,pNDSFile->pEngTitle,&pNDSFile->isNDSCommercialROM)==false){
      safefree(&MM_Process,pNDSFile->pNDSROMIcon); pNDSFile->pNDSROMIcon=NULL;
      pNDSFile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    }
    return;
  }
  
  u32 extlinkidx=ExtLink_GetTargetIndex(pNDSFile->Ext32);
  
  if(extlinkidx==(u32)-1){
    safefree(&MM_Process,pNDSFile->pNDSROMIcon); pNDSFile->pNDSROMIcon=NULL;
    pNDSFile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    return;
  }
  
  const char *pAlias=ConvertFullPath_Unicode2Alias(ExtLink_GetNDSFullPathFilenameUnicode(extlinkidx));
  
  ChangedCurrentPath=true;
  
  if(pAlias==NULL){
    _consolePrint("ExtLink: Not found extlink nds file.\n");
    safefree(&MM_Process,pNDSFile->pNDSROMIcon); pNDSFile->pNDSROMIcon=NULL;
    pNDSFile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
    return;
  }
  
  if(NDSROMIcon_Get(pAlias,pNDSFile->pNDSROMIcon,NULL,NULL,NULL)==false){
    safefree(&MM_Process,pNDSFile->pNDSROMIcon); pNDSFile->pNDSROMIcon=NULL;
    pNDSFile->pIcon=FileListAlpha_GetSkin(EFLSA_Icon_NDSROM);
  }
}

void NDSFiles_LoadFileInfo(TNDSFile *pNDSFile)
{
	if(pNDSFile->FileType==ENFFT_UpFolder || pNDSFile->FileType==ENFFT_Folder) return;
	
	if(pNDSFile->pFileInfo==NULL || pNDSFile->FileInfoWidth!=(u32)-1) return;
	
	CglCanvas *pbm=new CglCanvas(&MM_Temp,NULL,1,1,pf15bit);
	pbm->SetCglFont(pCglFontDefault);
	  
	NDSFiles_RefreshCurrentFolder_ins_GetFileInfo(pNDSFile);
	if(str_isEmpty(pNDSFile->pFileInfo)==true){
		pNDSFile->FileInfoWidth=0;
	}else{
		pNDSFile->FileInfoWidth=pbm->GetTextWidthA(pNDSFile->pFileInfo)+16;
	}
	
	u32 tx=NDSROMIconXMargin+NDSROMIcon32Width+NDSROMIconXMargin;
	if((tx+pbm->GetTextWidthW(pNDSFile->pFilenameUnicode_DoubleLine0))<(ScreenWidth-pNDSFile->FileInfoWidth)){
		pNDSFile->UseDoubleLine=false;
	}else{
		pNDSFile->UseDoubleLine=true;
	}
	
	if(pbm!=NULL){
		delete pbm; pbm=NULL;
	}
}

u32 NDSFiles_GetFilesCount(void)
{
  return(NDSFilesCount);
}

TNDSFile* NDSFiles_GetFileBody(u32 idx)
{
  return(&pNDSFiles[idx]);
}

