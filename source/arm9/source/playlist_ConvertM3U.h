
#include "playlist_ConvertM3U_CP2Unicode.h"

static inline void PlayList_ConvertM3U_Body_EUCToUnicode(const char *psrcstr,UnicodeChar *pdststrw)
{
  EUC2Unicode_Convert(psrcstr,pdststrw,0);
}

static inline void PlayList_ConvertM3U_Body_UTF8ToUnicode(const char *psrcstr,UnicodeChar *pdststrw)
{
  StrConvert_UTF82Unicode(psrcstr,pdststrw);
}

static inline void PlayList_ConvertM3U_Body_CP1252ToUnicode(const char *psrcstr,UnicodeChar *pdststrw)
{
  CP12522Unicode_Convert(psrcstr,pdststrw);
}

static inline void PlayList_ConvertM3U_Body_CP437ToUnicode(const char *psrcstr,UnicodeChar *pdststrw)
{
  CP4372Unicode_Convert(psrcstr,pdststrw);
}

static inline void PlayList_ConvertM3U_Body_CP850ToUnicode(const char *psrcstr,UnicodeChar *pdststrw)
{
  CP8502Unicode_Convert(psrcstr,pdststrw);
}

FAT_FILE* Shell_FAT_fopen_VariableCodepageToUnicodeTable(u32 cp);
static __attribute__ ((noinline)) bool PlayList_ConvertM3U_Body_Analize(const UnicodeChar *pBasePathW,UnicodeChar *preadstrw,UnicodeChar *pPathW,UnicodeChar *pFilenameW)
{
  {
    u32 pos=0;
    while(1){
      UnicodeChar wch=preadstrw[pos];
      if(wch==0) break;
      if(wch==(UnicodeChar)'\\') preadstrw[pos]=(UnicodeChar)'/';
      pos++;
    }
  }
  
  while(1){
    UnicodeChar wch=*preadstrw;
    if(wch==0) break;
    if(wch!=(UnicodeChar)' ') break;
    preadstrw++;
  }
  
  {
    bool tag=true;
    if(preadstrw[0]!=(UnicodeChar)'<') tag=false;
    if(preadstrw[1]!=(UnicodeChar)'m') tag=false;
    if(preadstrw[2]!=(UnicodeChar)'e') tag=false;
    if(preadstrw[3]!=(UnicodeChar)'d') tag=false;
    if(preadstrw[4]!=(UnicodeChar)'i') tag=false;
    if(preadstrw[5]!=(UnicodeChar)'a') tag=false;
    if(preadstrw[6]!=(UnicodeChar)' ') tag=false;
    if(preadstrw[7]!=(UnicodeChar)'s') tag=false;
    if(preadstrw[8]!=(UnicodeChar)'r') tag=false;
    if(preadstrw[9]!=(UnicodeChar)'c') tag=false;
    if(preadstrw[10]!=(UnicodeChar)'=') tag=false;
    if(preadstrw[11]!=(UnicodeChar)'\"') tag=false;
    if(tag==true){
      preadstrw+=12;
      u32 pos=0;
      while(1){
        if(preadstrw[pos]==0){
          _consolePrint("Not found end tag. Ignore item.\n");
          return(false);
        }
        bool endtag=true;
        if(preadstrw[pos+0]!=(UnicodeChar)'\"') endtag=false;
        if(preadstrw[pos+1]!=(UnicodeChar)'/') endtag=false;
        if(preadstrw[pos+2]!=(UnicodeChar)'>') endtag=false;
        if(endtag==true){
          preadstrw[pos]=0;
          break;
        }
        pos++;
      }
    }
  }
  
  {
    UnicodeChar wch=*preadstrw;
    if(wch==(UnicodeChar)'#') return(false);
    if(wch==(UnicodeChar)'<') return(false);
  }
  
  {
    UnicodeChar tmpw[512];
    u32 rpos=0,wpos=0;
    while(1){
      UnicodeChar wch=preadstrw[rpos++];
      if(wch==0) break;
      if(wch!=(UnicodeChar)'&'){
        tmpw[wpos++]=wch;
        continue;
      }
      {
        bool ex=true;
        if(preadstrw[rpos+0]!=(UnicodeChar)'a') ex=false;
        if(preadstrw[rpos+1]!=(UnicodeChar)'m') ex=false;
        if(preadstrw[rpos+2]!=(UnicodeChar)'p') ex=false;
        if(preadstrw[rpos+3]!=(UnicodeChar)';') ex=false;
        if(ex==true){
          rpos+=4;
          tmpw[wpos++]='&';
          continue;
        }
      }
      {
        bool ex=true;
        if(preadstrw[rpos+0]!=(UnicodeChar)'a') ex=false;
        if(preadstrw[rpos+1]!=(UnicodeChar)'p') ex=false;
        if(preadstrw[rpos+2]!=(UnicodeChar)'o') ex=false;
        if(preadstrw[rpos+3]!=(UnicodeChar)'s') ex=false;
        if(preadstrw[rpos+4]!=(UnicodeChar)';') ex=false;
        if(ex==true){
          rpos+=5;
          tmpw[wpos++]='\'';
          continue;
        }
      }
      tmpw[wpos++]=wch;
    }
    tmpw[wpos]=0;
    Unicode_Copy(preadstrw,tmpw);
  }
  
  if(DLLList_isSupportFormatFromFilenameUnicode(preadstrw)!=EPT_Sound) return(false);
  
  if(preadstrw[1]==(UnicodeChar)':') preadstrw+=2; // skip drive.
  
  if(preadstrw[0]==(UnicodeChar)'/'){
    SplitItemFromFullPathUnicode(preadstrw,pPathW,pFilenameW);
    if(FileExistsUnicode(pPathW,pFilenameW)==true) return(true);
//    _consolePrintf("Not found %s\n",StrConvert_Unicode2Ank_Test(preadstrw));
    return(false);
  }
  
  {
    UnicodeChar tmpw[512];
    Unicode_Copy(tmpw,pBasePathW);
    if(tmpw[Unicode_GetLength(tmpw)-1]!=(UnicodeChar)'/'){
      const UnicodeChar sw[2]={(UnicodeChar)'/',0};
      Unicode_Add(tmpw,sw);
    }
    SplitItemFromFullPathUnicode(preadstrw,pPathW,pFilenameW);
    if(pPathW[0]!=(UnicodeChar)'/'){
      Unicode_Add(tmpw,pPathW);
      }else{
      Unicode_Add(tmpw,&pPathW[1]);
    }
    Unicode_Copy(pPathW,tmpw);
  }
  
  if(FileExistsUnicode(pPathW,pFilenameW)==true) return(true);
//  _consolePrintf("Not found %s\n",StrConvert_Unicode2Ank_Test(preadstrw));
  return(false);
}

static __attribute__ ((noinline)) u32 PlayList_ConvertM3U_Body(const UnicodeChar *pBasePathW,char *pm3ubuf,u32 m3ubufsize,FAT_FILE *pfdst)
{
  u32 FilesCount=0;
  
  u32 m3ubufpos=0;
  
  const u32 dstbufsize=8*1024;
  u32 dstbufpos=0;
  u8 *pdstbuf=(u8*)safemalloc_chkmem(&MM_Temp,dstbufsize);
  
  char readstr[512];
  u32 readstrpos=0;
  
  u32 LastEncMode=0; // 0=Auto, 1=S-JIS, 2=UTF-8, 3=CP1252, 4=CP437, 5=CP850
  
  while(m3ubufpos<m3ubufsize){
    char ch=pm3ubuf[m3ubufpos++];
    if(0x20<=ch){
      if(readstrpos<(512-1)) readstr[readstrpos++]=ch;
      }else{
      if(readstrpos!=0){
        for(u32 idx=readstrpos;idx<512;idx++){
          readstr[idx]=0;
        }
        
        bool chkflag=false;
        UnicodeChar PathW[256],FilenameW[256];
        
        UnicodeChar readstrw[512];
        switch(LastEncMode){
          case 0: break; // Auto
          case 1: { // S-JIS
            PlayList_ConvertM3U_Body_EUCToUnicode(readstr,readstrw);
            if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true) chkflag=true;
          } break;
          case 2: { // UTF-8
            PlayList_ConvertM3U_Body_UTF8ToUnicode(readstr,readstrw);
            if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true) chkflag=true;
          } break;
          case 3: { // CP1252
            PlayList_ConvertM3U_Body_CP1252ToUnicode(readstr,readstrw);
            if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true) chkflag=true;
          } break;
          case 4: { // CP437
            PlayList_ConvertM3U_Body_CP437ToUnicode(readstr,readstrw);
            if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true) chkflag=true;
          } break;
          case 5: { // CP850
            PlayList_ConvertM3U_Body_CP850ToUnicode(readstr,readstrw);
            if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true) chkflag=true;
          } break;
        }
        
        if(chkflag==false){
          PlayList_ConvertM3U_Body_EUCToUnicode(readstr,readstrw);
          if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true){
            chkflag=true;
            _consolePrint("Set encode mode to EUC/S-JIS.\n");
            LastEncMode=1;
          }
        }
        if(chkflag==false){
          PlayList_ConvertM3U_Body_UTF8ToUnicode(readstr,readstrw);
          if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true){
            LastEncMode=2;
            _consolePrint("Set encode mode to UTF-8.\n");
            chkflag=true;
          }
        }
        if(chkflag==false){
          PlayList_ConvertM3U_Body_CP1252ToUnicode(readstr,readstrw);
          if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true){
            LastEncMode=3;
            _consolePrint("Set encode mode to CP1252.\n");
            chkflag=true;
          }
        }
        if(chkflag==false){
          PlayList_ConvertM3U_Body_CP437ToUnicode(readstr,readstrw);
          if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true){
            LastEncMode=4;
            _consolePrint("Set encode mode to CP437.\n");
            chkflag=true;
          }
        }
        if(chkflag==false){
          PlayList_ConvertM3U_Body_CP850ToUnicode(readstr,readstrw);
          if(PlayList_ConvertM3U_Body_Analize(pBasePathW,readstrw,PathW,FilenameW)==true){
            LastEncMode=5;
            _consolePrint("Set encode mode to CP850.\n");
            chkflag=true;
          }
        }
        
        DATA_IN_AfterSystem static u32 lostmsgwait=0;
        
        if(chkflag==true){
          lostmsgwait=0;
          
          FilesCount++;
          if((FilesCount&63)==0){
            char msg0[64],msg1[64];
            snprintf(msg0,64,"Converting... %d%%",(m3ubufpos*100)/m3ubufsize);
            snprintf(msg1,64,"%d / %d %d files.",m3ubufpos,m3ubufsize,FilesCount);
            CallBack_MWin_ProgressDraw(msg0,msg1,m3ubufpos,m3ubufsize);
          }
          
          if((PathW[0]==0)||(FilenameW[0]==0)) StopFatalError(11302,"Check flag error.\n");
          
          if((dstbufsize-2048)<=dstbufpos){
            FAT2_fwrite(pdstbuf,1,dstbufpos,pfdst);
            dstbufpos=0;
          }
          u32 pathlen=Unicode_GetLength(PathW)+1;
          u32 filenamelen=Unicode_GetLength(FilenameW)+1;
          pdstbuf[dstbufpos++]=pathlen;
          pdstbuf[dstbufpos++]=filenamelen;
          MemCopy16CPU(PathW,&pdstbuf[dstbufpos],pathlen*2);
          dstbufpos+=pathlen*2;
          MemCopy16CPU(FilenameW,&pdstbuf[dstbufpos],filenamelen*2);
          dstbufpos+=filenamelen*2;
          
          }else{
          lostmsgwait++;
          if((lostmsgwait&63)==0){
            char msg0[64],msg1[64];
            snprintf(msg0,64,"Not found file. %d%%",(m3ubufpos*100)/m3ubufsize);
            snprintf(msg1,64,"%d / %d %d files.",m3ubufpos,m3ubufsize,FilesCount);
            CallBack_MWin_ProgressDraw(msg0,msg1,m3ubufpos,m3ubufsize);
          }
        }
        
        readstrpos=0;
      }
    }
  }
  
  if(dstbufpos!=0) FAT2_fwrite(pdstbuf,1,dstbufpos,pfdst);
  
  if(pdstbuf!=NULL){
    safefree(&MM_Temp,pdstbuf); pdstbuf=NULL;
  }
  
  _consolePrintf("Found files count= %d.\n",FilesCount);
  
  return(FilesCount);
}

void PlayList_ConvertM3U(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pPlayListFilenameUnicode)
{
  FileHeaderSectorNum=0;
  
  char *pm3ubuf=NULL;
  u32 m3ubufsize=0;
  
  {
    CallBack_MWin_ProgressShow("Loading...",0);
    
    FAT_FILE *pf=FAT2_fopen_AliasForRead(ConvertFull_Unicode2Alias(pBasePathUnicode,pPlayListFilenameUnicode));
    if(pf==NULL) StopFatalError(11303,"Play list file open error.\n");
    m3ubufsize=FAT2_GetFileSize(pf);
    pm3ubuf=(char*)safemalloc_chkmem(&MM_Temp,m3ubufsize+32);
    FAT2_fread(pm3ubuf,1,m3ubufsize,pf);
    FAT2_fclose(pf);
    for(u32 idx=0;idx<32;idx++){
      pm3ubuf[m3ubufsize+idx]=0;
    }
    
    CallBack_MWin_ProgressHide();
  }
  
  FAT_FILE *pfdst=Shell_FAT_fopenwrite_Internal(ResumePlayListFilename);
  if(pfdst==NULL) StopFatalError(11305,"Resume play list file write error.\n");
  
  FileHeader.ID=FileHeader_ID;
  FileHeader.Position=0;
  FileHeader.Pause=false;
  FileHeader.Error=false;
  FileHeader.FullPathFilenameW[0]=0;
  FAT2_fwrite(&FileHeader,1,sizeof(FileHeader),pfdst);
  
  CP2Unicode_Init();
  CP2Unicode_Load();
  
  CallBack_MWin_ProgressShow("Converting...",0);
  u32 FilesCount=PlayList_ConvertM3U_Body(pBasePathUnicode,pm3ubuf,m3ubufsize,pfdst);
  CallBack_MWin_ProgressHide();
  
  CP2Unicode_Free();
  
  u16 Terminater=0;
  FAT2_fwrite(&Terminater,1,2,pfdst);
  
  while((FAT2_GetFileSize(pfdst)&3)!=0){
    u32 dummy=0;
    FAT2_fwrite(&dummy,1,1,pfdst);
  }
  
  FAT2_fwrite(&FilesCount,4,1,pfdst);
  
  FAT2_fclose(pfdst);
  
  if(pm3ubuf!=NULL){
    safefree(&MM_Temp,pm3ubuf); pm3ubuf=NULL;
  }
  
  CallBack_MWin_ProgressHide();
}
