
#pragma Ospace

#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "../../ipc6.h"

#include "procstate.h"
#include "datetime.h"
#include "ErrorDialog.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "skin.h"

#include "inifile.h"

#include "msgwin.h"
#include "sndeff.h"
#include "lang.h"
#include "strpcm.h"
#include "dllsound.h"
#include "rect.h"
#include "resume.h"
#include "playlist.h"
#include "strtool.h"
#include "cfont.h"
#include "cctf.h"
#include "euc2unicode.h"

// -----------------------------

DATA_IN_IWRAM_TextView static bool ToCustomMode;

DATA_IN_IWRAM_TextView static CglCanvas *pSubTempBM;
DATA_IN_IWRAM_TextView static CglCanvas *pDrawLineBM;

#include "proc_TextView_Popup.h"

DATA_IN_IWRAM_TextView static u32 ClockTimeOut;

DATA_IN_IWRAM_TextView static u32 ShowID3TagTimeOut;

DATA_IN_IWRAM_TextView static s32 BrightLevel;

DATA_IN_IWRAM_TextView static u32 ResumeSaveTimeVSync;

DATA_IN_IWRAM_TextView static bool ScreenRedrawFlag;

DATA_IN_IWRAM_TextView static bool AutoScrollFlag;
DATA_IN_IWRAM_TextView static s32 AutoScrollSpeed;
DATA_IN_IWRAM_TextView static u32 AutoScrollTimeVSync;

// -----------------------------

#define ScrollBarWidth (12)

#define LeftMargin (6)
#define RightMargin (4+ScrollBarWidth)
#define TopMargin (8)
#define BottomMargin (2)

#define LineWidth (ScreenWidth-LeftMargin-RightMargin)
//#define LineHeight (ScreenHeight-TopMargin-BottomMargin)

#include "proc_TextView_extfont.h"

// -----------------------------

static void MP3Cnt_Exec_Prev(void)
{
	if(PlayList_GetFilesCount()==0) return;
    Sound_Start(WAVFN_Click);
    Popup_Show_MP3Cnt_Prev();
    
    switch(ProcState.FileList.PlayMode){
      	case EPSFLPM_Repeat: {
      		PlayList_Repeat();
      	} break;
      	case EPSFLPM_AllRep:  {
      		PlayList_Prev();
      	} break;
      	case EPSFLPM_Shuffle: {
      		PlayList_PrevShuffle();
      	} break;
      }
}

static void MP3Cnt_Exec_Next(void)
{
	if(PlayList_GetFilesCount()==0) return;
    Popup_Show_MP3Cnt_Next();
  
    switch(ProcState.FileList.PlayMode){
        case EPSFLPM_Repeat: {
            PlayList_Repeat();
        } break;
        case EPSFLPM_AllRep: {
            PlayList_Next();
        } break;
        case EPSFLPM_Shuffle: {
            PlayList_NextShuffle();
        } break;
    }
}

static void MP3Cnt_Exec_ChangePause(void)
{
	if(PlayList_GetFilesCount()==0) return;
    if(PlayList_GetPause()==false) Sound_Start(WAVFN_Click);
    PlayList_TogglePause();
    Popup_Show_MP3Cnt_Pause();
}

//------------------------------

DATA_IN_IWRAM_TextView static bool RequestInterruptBreak;
DATA_IN_IWRAM_TextView static bool RequestPretreatment;
DATA_IN_IWRAM_TextView static bool RequestAllConvert;
DATA_IN_IWRAM_TextView static u32 RequestBKMResumeFileOffset;

DATA_IN_IWRAM_TextView static FAT_FILE *FileHandle;
DATA_IN_IWRAM_TextView static u32 FileSize;

DATA_IN_IWRAM_TextView static s32 ShowLineHeight,ShowLineMargin,ShowLineCount;

DATA_IN_IWRAM_TextView static bool EncodeConvertAllEnd;
DATA_IN_IWRAM_TextView static u32 EncodeConvertFileOffset;
DATA_IN_IWRAM_TextView static u32 EncodeConvertTopOffset;

//------------------------------

#define TextLinesMaxCount (384*1024)
DATA_IN_IWRAM_TextView static u32 TextLinesCount;

DATA_IN_IWRAM_TextView static u32 TotalCharsCount;

//------------------------------

static void Bookmark_SetResumeLineNum(u32 linenum,u32 fileofs);

#include "proc_TextView_ScrollBar.h"

static void TextView_MainDrawText(TScrollBar *psb);
static void TextView_SubDrawText(TScrollBar *psb);

#include "proc_TextView_dfs.h"
#include "proc_TextView_libconv.h"


#include "proc_TextView_bookmark_fileio.h"
#include "proc_TextView_bookmark_ui.h"

#include "proc_TextView_Draw.h"

// -----------------------------

DATA_IN_IWRAM_TextView static u32 KEYS_PressCount=0;
DATA_IN_IWRAM_TextView static u32 KEYS_LastPress=0;

static void CB_KeyReleases(u32 VsyncCount){
    KEYS_PressCount=0;
}

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
    if((Keys&KEY_START)!=0){
        Sound_Start(WAVFN_Click);
        AutoScrollFlag=!AutoScrollFlag;
        return;
        
    }
    
    if((Keys&KEY_SELECT)!=0){
    	if(GlobalINI.System.ChildrenMode)  {
    		Sound_Start(WAVFN_Notify);
    	}else{
    		Sound_Start(WAVFN_Click);
    		ToCustomMode=true;
    		SetNextProc(ENP_TextCustom,EPFE_RightToLeft);
    	}
        return;
    }
    
    if(WaitKeyRelease) return;
    
    s32 v=0;
    TScrollBar *psb=&ScrollBar;
    s32 pagesize=ShowLineCount;
    s32 linecnt=TextLinesCount-ShowLineCount;
    
    if((Keys&(KEY_L|KEY_R))!=0){
        if((Keys&(KEY_X|KEY_Y))!=0){
            if((Keys&KEY_X)!=0) ChangeNextBacklightLevel();
            if((Keys&KEY_Y)!=0) ChangePrevBacklightLevel();
            Popup_Show_BacklightLevel();
            return;
        }
        if((Keys&KEY_B)!=0){
            Sound_Start(WAVFN_Click);
            ToCustomMode=false;
            SetNextProc(ENP_FileList,EPFE_CrossFade);
            return;
        }
        if((Keys&KEY_UP)!=0) v=-1*5;
        if((Keys&KEY_DOWN)!=0) v=1*5;
        if((Keys&(KEY_B|KEY_LEFT))!=0) v=-(pagesize*5);
        if((Keys&(KEY_A|KEY_RIGHT))!=0) v=pagesize*5;
        if(EncodeConvertAllEnd) ShowID3TagTimeOut=2*60;
    }
    
    if((Keys&(KEY_X|KEY_Y))!=0){
        if(AutoScrollFlag){
            Sound_Start(WAVFN_Click);
            if((Keys&KEY_Y)!=0) AutoScrollSpeed+=2;
            if((Keys&KEY_X)!=0) AutoScrollSpeed-=2;
            if(AutoScrollSpeed<0) AutoScrollSpeed=0;
            if(AutoScrollSpeed>10) AutoScrollSpeed=10;
            WaitKeyRelease=true;
            return;
        }
        if(!EncodeConvertAllEnd) {
            RequestAllConvert=true;
            libconv_Convert(FileHandle,FileSize,pCglFontDefault);
            RequestAllConvert=false;
            CallBack_MWin_ProgressHide();
        }
        if((Keys&KEY_Y)!=0) BookmarkUI_Start(false);
        if((Keys&KEY_X)!=0) BookmarkUI_Start(true);
        return;
    }
    
        
    if(v==0 && (Keys&(KEY_A|KEY_B|KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT))!=0){  
        if((Keys&KEY_UP)!=0) v=-1;
        if((Keys&KEY_DOWN)!=0) v=1;
        if((Keys&(KEY_B|KEY_LEFT))!=0) v=-(pagesize);
        if((Keys&(KEY_A|KEY_RIGHT))!=0) v=pagesize; 
        if(ProcState.Text.BButtonToExitText && (Keys&KEY_B)!=0){
        	Sound_Start(WAVFN_Click);
        	ToCustomMode=false;
        	SetNextProc(ENP_FileList,EPFE_CrossFade);
        	return;
        }
    }
    
    if(Keys==KEYS_LastPress){
        if(KEYS_PressCount<(8*80)) {
            KEYS_PressCount+=VsyncCount;
        }else{
            KEYS_PressCount=(8*80);
        }
    }else{
        KEYS_LastPress=Keys;
        KEYS_PressCount=0;
    }
    
    const u32 Timeout=8*1;
    if((psb->CurrentLineIndex==0 && v<0) || (psb->CurrentLineIndex==linecnt && v>0)) {
        if(Timeout<=KEYS_PressCount) {
            if(v>0){
                Popup_Show_MoveToTop();
                ScrollBar_SetCurrentLineIndex(psb,0);
            }else{
                Popup_Show_MoveToLast();
                if(!EncodeConvertAllEnd) {
                    RequestAllConvert=true;
                    libconv_Convert(FileHandle,FileSize,pCglFontDefault);
                    RequestAllConvert=false;
                    CallBack_MWin_ProgressHide();
                }
                linecnt=TextLinesCount-ShowLineCount;
                ScrollBar_SetCurrentLineIndex(psb,linecnt);
            }
            Sound_Start(WAVFN_Open);
            ScreenRedrawFlag=true;
            WaitKeyRelease=true;
            v=0;
        }
        return;
    }
            
    if(v!=0){
        if(2<KEYS_PressCount/8) {
            if(v<0) 
                v-=(KEYS_PressCount/8-2);
            else
                v+=(KEYS_PressCount/8-2);
        }
        if(((psb->CurrentLineIndex+v)<=0) || ((psb->CurrentLineIndex+v)>=linecnt)) WaitKeyRelease=true;
        ScrollBar_SetCurrentLineIndex(psb,psb->CurrentLineIndex+v);
        ScreenRedrawFlag=true;
    }
            
}

static void CB_KeySameLRDown(void)
{
	if((IPC6->PanelOpened==false)&&(ProcState.System.LRKeyLockType==ELRLT_RelationalPanel)||(ProcState.System.LRKeyLockType==ELRLT_AlwayOff)) return;
    MP3Cnt_Exec_ChangePause();
}

DATA_IN_IWRAM_TextView static bool isPressMouseButton;

static void CB_MouseDown(s32 x,s32 y)
{
    if(ProcState.Text.BacklightFlag==false && ProcState.Text.TopScrMode != ETTSM_LightOff){
        ProcState.Text.BacklightFlag=true;
        ProcState_RequestSave=true;
        TextView_SubDrawText(&ScrollBar);
        return;
    }

    if(ProcState.Text.isSwapDisp && ProcState.Text.TopScrMode != ETTSM_LightOff){
        ProcState.Text.BacklightFlag=false;
        ProcState_RequestSave=true;
    }
      
    if(ProcState.Text.isSwapDisp) return;
    
    isPressMouseButton=true;
    if(ScrollBar_MouseDown(&ScrollBar,x,y)==true) return;
    if(TextView_MouseDown(&ScrollBar,x,y)==true) return;
}

static void CB_MouseMove(s32 x,s32 y)
{
    if(isPressMouseButton==false) return;
      
    if(ScrollBar_MouseMove(&ScrollBar,x,y)==true) return;
    if(TextView_MouseMove(&ScrollBar,x,y)==true) return;
}

static void CB_MouseUp(s32 x,s32 y)
{
    if(isPressMouseButton==false) return;
    isPressMouseButton=false;
    
    if(ScrollBar_MouseUp(&ScrollBar,x,y)==true) return;
    if(TextView_MouseUp(&ScrollBar,x,y)==true) return;
  
}

// ------------------------------------------------------------

DATA_IN_IWRAM_TextView static bool Process_SeekNext,Process_SeekPrev;
DATA_IN_IWRAM_TextView static u32 Process_WaitCount;

static void CB_Start(void)
{
    ResumeSaveTimeVSync=0;
  
    ToCustomMode=false;
  
    Bookmark_Init();
  
    Process_SeekNext=false;
    Process_SeekPrev=false;
    Process_WaitCount=0;
  
    pScreenMainOverlay->pCanvas->FillFull(0);
    
    CallBack_MWin_ProgressShow("",0);
  
    u32 FontSize=ProcState.Text.FontSize;
    switch(FontSize){
        case Text_FontSize_Small: ShowLineMargin=2; break;
        case Text_FontSize_Middle: ShowLineMargin=4; break;
        case Text_FontSize_Large: ShowLineMargin=5; break;
        default: StopFatalError(18101,"Illigal font size. (%d)\n",ProcState.Text.FontSize); break;
    }
    
    switch(ProcState.Text.LineSpace){
        case ETLS_Small: ShowLineMargin/=2; break;
        case ETLS_Middle: ShowLineMargin*=1; break;
        case ETLS_Large: ShowLineMargin*=2; break;
    }
  
    ShowLineHeight=FontSize+ShowLineMargin;
  
    ShowLineCount=(ScreenHeight-TopMargin-BottomMargin)/ShowLineHeight;
  
    CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_LoadExtendFont"),"",0,0);
    ExtFont_Init();
  
    libconv_Init();
  
    PrintFreeMem_Accuracy();
  
    FAT_FILE *pfh=Shell_FAT_fopen_Split(RelationalFilePathUnicode,RelationalFileNameUnicode);
    if(pfh==NULL) StopFatalError(18102,"Can not open text file.\n");
  
    FileHandle=pfh;
    FileSize=FAT2_GetFileSize(pfh);
    
    TFAT2_TIME ft=FAT2_GetFileLastWriteTime();
        
    if(FileSize==0){
        libconv_EndConvert();
        CallBack_MWin_ProgressHide();
        Bookmark_Free();
        _consolePrint("Text reader: file not found or size limit error.\n");
        ErrorDialog_Set(EEC_MemoryOverflow_CanRecovery);
        ToCustomMode=false;
        SetNextProc(ENP_FileList,EPFE_CrossFade);
        return;
    }
  
    DFS_Init(((FileSize-(FileSize%512))*2)+(64*1024)); // ワーストケースで(ASCII)1byte->(UNICODE)2byteの二倍になる。64kbyteは念のため余剰。
    
    if(ManualTextEncode_OverrideFlag==true){
        ManualTextEncode_OverrideFlag=false;
        Bookmark_SetTextEncode(ManualTextEncode);
    }
    
    {
        CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_LoadTextFile"),"",0,0);
    
        u32 chkbufsize=128*1024;
        if(FileSize<chkbufsize) chkbufsize=FileSize;
        u8 *pchkbuf=(u8*)safemalloc_chkmem(&MM_Temp,chkbufsize);
        _consolePrintf("Check buffer: ptr=0x%08x, %dbyte.\n",pchkbuf,chkbufsize);
        FAT2_fseek(pfh,0,SEEK_SET);
        FAT2_fread(pchkbuf,1,chkbufsize,pfh);
    
        switch(Bookmark_GetTextEncode()){
            case ETE_Auto: {
                CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_DetectTextEncode"),"",0,0);
                libconv_AutoSelectEncode(pchkbuf,chkbufsize);
                _consolePrintf("Detected text encode is %s.\n",pEncodeID);
            } break;
            case ETE_ANSI: libconv_SelectEncode_ANSI(); break;
            case ETE_EUC: libconv_SelectEncode_EUC(); break;
            case ETE_UTF16BE: libconv_SelectEncode_UTF16BE(); break;
            case ETE_UTF16LE: libconv_SelectEncode_UTF16LE(); break;
            case ETE_UTF8: libconv_SelectEncode_UTF8(); break;
        }
    
        CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_DetectReturnCode"),"",0,0);
        libconv_DetectReturnCode(pchkbuf,chkbufsize);
        switch(ReturnCode){
            case ERC_Unknown: _consolePrint("Detected return code is unknown.\n"); break;
            case ERC_CR: _consolePrint("Detected return code is CR.\n"); break;
            case ERC_LF: _consolePrint("Detected return code is LF.\n"); break;
            case ERC_CRLF: _consolePrint("Detected return code is CR+LF.\n"); break;
            case ERC_LFCR: _consolePrint("Detected return code is LF+CR.\n"); break;
        }
    
        if(pchkbuf!=NULL){
            safefree(&MM_Temp,pchkbuf); pchkbuf=NULL;
        }
    }
    
    RequestInterruptBreak=false;
    
    RequestBKMResumeFileOffset=0;
    {
        u32 fileofs=0;
        if(RelationalFilePos!=0) fileofs=RelationalFilePos;
        if(Bookmark_GetResumeLineNum()!=0){
            fileofs=Bookmark_GetResumeFileOffset();
        }
        RequestBKMResumeFileOffset=fileofs;
    }
  
    CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_ConvertToUnicode"),"",0,0);
    RequestAllConvert=false;
    RequestPretreatment=true;
    libconv_Convert(pfh,FileSize,pCglFontDefault);
    RequestPretreatment=false;
  
    if(RequestInterruptBreak==true){
        _consolePrint("Request interrupt break.\n");
        Sound_Start(WAVFN_Notify);
        SetNextProc(ENP_FileList,EPFE_None);
    }
  
    libconv_EndConvert();
    
    CallBack_MWin_ProgressDraw(Lang_GetUTF8("TV_PRG_LoadExtendFont"),"",0,0);
    ExtFont_LoadBody();    
  
    CallBack_MWin_ProgressHide();
  

    ScrollBar_Free(&ScrollBar);
    ScrollBar_Init(&ScrollBar,ShowLineHeight);

    ScrollBar.TopPos=0;
    ScrollBar.ShowPos=ScrollBar.TopPos;
    ScrollBar.MaxPos=ScrollBar.LineHeight*TextLinesCount;
    
    pSubTempBM=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScreenHeight,pf15bit);
    pDrawLineBM=new CglCanvas(&MM_Process,NULL,ScreenWidth,ScrollBar.LineHeight,pf15bit);
    
    pSubTempBM->SetCglFont(pCglFontDefault);
    
    {
        u32 linenum=0;
        if(RelationalFilePos!=0) linenum=RelationalFilePos;
        if(Bookmark_GetResumeLineNum()!=0){
            linenum=Bookmark_GetResumeLineNum()-1;
            Bookmark_SetResumeLineNum(0,0);
        }
        ScrollBar_SetCurrentLineIndex(&ScrollBar,linenum);
        ScrollBar.ShowPos=ScrollBar.TopPos;
    }
  
    if(RequestInterruptBreak==true){
        Resume_Clear();
        return;
    }
  
    if(ProcState.Text.isSwapDisp==true) {
        REG_POWERCNT = (REG_POWERCNT & ~POWER_SWAP_LCDS) | POWER_SWAP_LCDS;
        if(ProcState.Text.BacklightFlag==false){
            IPC6->LCDPowerControl=LCDPC_ON_TOP;
            BrightLevel=0*0x100;
        }else{
            IPC6->LCDPowerControl=LCDPC_ON_BOTH;
            BrightLevel=16*0x100;
        }
        pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
    }
    
    
    Sound_Start(WAVFN_Open);
    ScreenRedrawFlag=true;
    
    if(ScreenRedrawFlag==true){
        ScreenRedrawFlag=false;
        TextView_SubDrawText(&ScrollBar);
        TextView_MainDrawText(&ScrollBar);
        TextView_MainDrawText(&ScrollBar);
    }
  
    if(PlayList_isOpened()==false){
        if(PlayList_Start(true,NULL,NULL)==false){
            _consolePrint("Play list load error.\n");
        }
    }
  
    Resume_SetResumeMode(ERM_Text);
    Resume_SetFilename(FullPathUnicodeFromSplitItem(RelationalFilePathUnicode,RelationalFileNameUnicode));
    Resume_SetPos(libconv_GetTextFileOffset(ScrollBar.CurrentLineIndex));
    Resume_Save();
  
    ClockTimeOut=0;
    
    ShowID3TagTimeOut=0;
    
    ResumeSaveTimeVSync=1;
    
    AutoScrollFlag=false;
    AutoScrollSpeed=5;
    AutoScrollTimeVSync=1;
    
    //_consoleLogPause();
    
    //videoSetModeSub(MODE_2_2D | DISPLAY_BG2_ACTIVE);
      
   // _consoleSetLogOutFlag(true);
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
    Popup_VsyncUpdate(VsyncCount);
  
    if(ClockTimeOut==0){
        ClockTimeOut=60;
        if(ProcState.Text.TopScrMode==ETTSM_Clock) TextView_SubDrawText(&ScrollBar);
    }else{
        ClockTimeOut--;    
    }
  
    if(ShowID3TagTimeOut!=0) ShowID3TagTimeOut--;
    
    PlayList_UpdateResume(VsyncCount);
    
    if(Bookmark_Enabled==true && VsyncCount!=(u32)-1 && ResumeSaveTimeVSync!=0){
        ResumeSaveTimeVSync+=VsyncCount;
        if(ResumeSaveTimeVSync>(60*5)) {
            if(Bookmark_Enabled==true) Bookmark_SetResumeLineNum(1+ScrollBar.CurrentLineIndex,libconv_GetTextFileOffset(ScrollBar.CurrentLineIndex));
            ResumeSaveTimeVSync=1;
        }
    }
    
    if(AutoScrollFlag){
        if(VsyncCount!=(u32)-1){
            AutoScrollTimeVSync+=VsyncCount;
            if(AutoScrollTimeVSync>AutoScrollSpeed) {
                ScrollBar.TopPos++;
                ScrollBar.ShowPos=ScrollBar.TopPos;
                ScrollBar.CurrentLineIndex=ScrollBar.TopPos/ScrollBar.LineHeight;
                if(ScrollBar.CurrentLineIndex>=TextLinesCount-ShowLineCount) AutoScrollFlag=false;
                ScreenRedrawFlag=true;
                AutoScrollTimeVSync=1;
            }
        }
    }
    
    for(u32 idx=0;idx<VsyncCount;idx++){
        ScrollBar_MouseIdle(&ScrollBar);
        TextView_MouseIdle(&ScrollBar);
    }
    
    if(ScreenRedrawFlag==true){
        ScreenRedrawFlag=false;
        TextView_SubDrawText(&ScrollBar);
        TextView_MainDrawText(&ScrollBar);
    }
    
    if((Process_SeekNext==true)||(Process_SeekPrev==true)){
        if(Process_WaitCount<VsyncCount){
            Process_WaitCount=0;
        }else{
            Process_WaitCount-=VsyncCount;
        }
        if(Process_WaitCount==0){
            Process_WaitCount=8;
            s32 val=0;
            if(Process_SeekPrev==true) val=-1;
             if(Process_SeekNext==true) val=+1;
             if(DLLSound_SeekPer(val)==true){
                 Popup_Show_MP3Cnt_Seek(val);
                 DLLSound_SeekPer(val);
             }
             if(ProcState.Text.TopScrMode==ETTSM_Clock) TextView_SubDrawText(&ScrollBar);
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
    }
    
    if(!EncodeConvertAllEnd) {
    	while(VBlankPassedFlag==false){
    		libconv_Convert(FileHandle,FileSize,pCglFontDefault);
    		ScreenRedrawFlag=true;
    		if(EncodeConvertAllEnd) break;
    	}
    }
    
    if(ProcState.Text.isSwapDisp==false || ProcState.Text.TopScrMode == ETTSM_LightOff) return;
        
      if(ProcState.Text.BacklightFlag==false){
        if(BrightLevel!=(0*0x100)){
          BrightLevel-=VsyncCount*64;
          pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
          if(BrightLevel<=(0*0x100)){
            BrightLevel=0*0x100;
            pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
            IPC6->LCDPowerControl=LCDPC_ON_TOP;
          }
        }
        }else{
        if(BrightLevel!=(16*0x100)){
          IPC6->LCDPowerControl=LCDPC_ON_BOTH;
          BrightLevel+=VsyncCount*128;
          pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
          if((16*0x100)<=BrightLevel){
            BrightLevel=16*0x100;
            pScreenSub->SetBlackOutLevel16(16-(BrightLevel/0x100));
          }
        }
      }
      
}

static void CB_End(void)
{
    if(Bookmark_Enabled==true) Bookmark_SetResumeLineNum(1+ScrollBar.CurrentLineIndex,libconv_GetTextFileOffset(ScrollBar.CurrentLineIndex));

    Bookmark_Free();
  
    Resume_Clear();
  
    if(ToCustomMode==false) RelationalFile_Clear();
  
    if(pSubTempBM!=NULL){
    	delete pSubTempBM; pSubTempBM=NULL;
    }
      
    if(pDrawLineBM!=NULL){
    	delete pDrawLineBM; pDrawLineBM=NULL;
    }
  
    ExtFont_Free();
  
    Popup_Free();
  
    libconv_Free();
  
    DFS_Free();
    
    ScrollBar_Free(&ScrollBar);
    
    FAT2_fclose(FileHandle);
    
    Shell_FAT_fclose_SwapFile();
  
    ProcState_RequestSave=true;
    ProcState_Save();
    
    if(ProcState.Text.isSwapDisp==true){
        REG_POWERCNT = (REG_POWERCNT & ~POWER_SWAP_LCDS); // | POWER_SWAP_LCDS;

        IPC6->LCDPowerControl=LCDPC_ON_BOTH;
        pScreenSub->SetBlackOutLevel16(0);
    }
}

static void CB_MWin_ProgressShow(const char *pTitleStr,s32 Max)
{
    msgwin_Draw(pTitleStr,"",0,0);
}

static void CB_MWin_ProgressSetPos(const char *pTitleStr,s32 pos, s32 max)
{
    char str[64];
    snprintf(str,64,"%d/%d %d%%",pos,max,pos*100/max);
    msgwin_Draw(pTitleStr,str,pos,max);
}

static void CB_MWin_ProgressDraw(const char *pstr0,const char *pstr1,s32 pos,s32 max)
{
    msgwin_Draw(pstr0,pstr1,pos,max);
}

static void CB_MWin_ProgressHide(void)
{
    msgwin_Clear();
}

#include "proc_TextView_Trigger_CallBack.h"

void ProcTextView_SetCallBack(TCallBack *pCallBack)
{
    pCallBack->Start=CB_Start;
    pCallBack->VsyncUpdate=CB_VsyncUpdate;
    pCallBack->End=CB_End;
    pCallBack->KeyPress=CB_KeyPress;
    pCallBack->KeyReleases=CB_KeyReleases;
    pCallBack->KeySameLRDown=CB_KeySameLRDown;
    pCallBack->MouseDown=CB_MouseDown;
    pCallBack->MouseMove=CB_MouseMove;
    pCallBack->MouseUp=CB_MouseUp;
  
    pCallBack->Trigger_ProcStart=CB_Trigger_ProcStart;
    pCallBack->Trigger_ProcEnd=CB_Trigger_ProcEnd;
    pCallBack->Trigger_Down=CB_Trigger_Down;
    pCallBack->Trigger_Up=CB_Trigger_Up;
    pCallBack->Trigger_LongStart=CB_Trigger_LongStart;
    pCallBack->Trigger_LongEnd=CB_Trigger_LongEnd;
    pCallBack->Trigger_SingleClick=CB_Trigger_SingleClick;
    pCallBack->Trigger_SingleLongStart=CB_Trigger_SingleLongStart;
    pCallBack->Trigger_SingleLongEnd=CB_Trigger_SingleLongEnd;
    pCallBack->Trigger_DoubleClick=CB_Trigger_DoubleClick;
    pCallBack->Trigger_DoubleLongStart=CB_Trigger_DoubleLongStart;
    pCallBack->Trigger_DoubleLongEnd=CB_Trigger_DoubleLongEnd;
    pCallBack->Trigger_TripleClick=CB_Trigger_TripleClick;
  
    pCallBack->MWin_ProgressShow=CB_MWin_ProgressShow;
    pCallBack->MWin_ProgressSetPos=CB_MWin_ProgressSetPos;
    pCallBack->MWin_ProgressDraw=CB_MWin_ProgressDraw;
    pCallBack->MWin_ProgressHide=CB_MWin_ProgressHide;
}

