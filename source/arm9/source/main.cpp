
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <nds.h>

#include "_const.h"
#include "maindef.h"
#include "_console.h"
#include "_consoleWriteLog.h"

#include "plug_ndsrom.h"

#include "memtool.h"
#include "strpcm.h"
#include "dll.h"
#include "dllsound.h"
#include "lang.h"
#include "VideoResume.h"
#include "resume.h"
#include "procstate.h"
#include "launchstate.h"
#include "shell.h"
#include "skin.h"
#include "extmem.h"
#include "splash.h"
#include "BootROM.h"
#include "ErrorDialog.h"
#include "component.h"
#include "extlink.h"
#include "strtool.h"
#include "playlist.h"
#include "euc2unicode.h"
#include "inifile.h"

#include "../../ipc6.h"

#include "arm9tcm.h"
#include "setarm9_reg_waitcr.h"

#include "fat2.h"
#include "zlibhelp.h"

#include "sndeff.h"
#include "datetime.h"

#include "fpga_helper.h"

#include "OverlayManager.h"

#include "md5.h"

u32 *pDTCMEND,*pMTCMEND;

DATA_IN_AfterSystem CglFont *pCglFontDefault=NULL;

DATA_IN_AfterSystem bool isExistsROMEO2;

DATA_IN_AfterSystem ENextProc NextProc;

DATA_IN_AfterSystem UnicodeChar RelationalFilePathUnicode[MaxFilenameLength];
DATA_IN_AfterSystem UnicodeChar RelationalFileNameUnicode[MaxFilenameLength];
DATA_IN_AfterSystem u32 RelationalFilePos;

DATA_IN_AfterSystem ETextEncode ManualTextEncode=ETE_Auto;
DATA_IN_AfterSystem bool ManualTextEncode_OverrideFlag=false;
DATA_IN_AfterSystem bool isExistsTextEditor=false;

DATA_IN_AfterSystem bool isCustomFromFileList=false;

DATA_IN_AfterSystem bool GBABUS_OwnerARM7=false; 

DATA_IN_AfterSystem bool WaitKeyRelease=false;

DATA_IN_AfterSystem bool LongPressRequest=false;
// ------------------------------------------------------------------

void __StopFatalError__(const char *filename,int linenum,const char *funcname,s32 errnum,const char* format, ...)
{
    _consoleLogResume();
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    videoSetModeSub(MODE_2_2D | DISPLAY_BG2_ACTIVE);
  
    _consoleSetLogOutFlag(true);
  
    _consolePrintf("\n%s:%d %s: #%05d: ",filename,linenum,funcname,errnum);
  
    {
        static char strbuf[126+1];
        va_list args;
        va_start( args, format );
        vsnprintf( strbuf, 126, format, args );
        _consolePrint(strbuf);
    }
  
    if(errnum<0) {
        _consolePrint("\n OK. \n\n");
    }else{
        _consolePrint("\n     Application halted !!\n");
        _consolePrint("Please refer [/fishell2/logbuf.txt]\n\n");
    }
    while(1);
}

// ------------------------------------------------------

__attribute__ ((noinline)) void IlligalWritedNullPointer_Check(u32 state)
{
    u64 *p=(u64*)NULL;
    if(*p!=NULLSTR64bit){
        _consolePrintf("0x%08x: ",NULL);
        u8 *p8=(u8*)p;
        for(u32 idx=0;idx<16;idx++){
            _consolePrintf("0x%02x,",p8[idx]);
        }
        _consolePrint("\n");
        StopFatalError(14004,"Illigal writed on NULL pointer. (%d)",state);
    }
}

// ------------------------------------------------------

static void DrawBootWarn(const void *pSrcBuf,u32 SrcSize,u32 palmode)
{
    TZLIBData zd;
  
    zd.DstSize=ScreenWidth*ScreenHeight;
    zd.pDstBuf=(u8*)safemalloc_chkmem(&MM_Temp,zd.DstSize);
    zd.SrcSize=SrcSize;
    zd.pSrcBuf=(u8*)pSrcBuf;
  
    if((zd.pSrcBuf[0]!=0x78)||(zd.pSrcBuf[1]!=0x01)) StopFatalError(10603,"Boot warn screen unknown compress format.\n");

    if(zlibdecompress(&zd)==false) StopFatalError(10604,"Boot warn screen ZLIB decompress error.\n");
  
    u16 pals[4];
  
    if(palmode==0){
        pals[0]=RGB15(0,0,0)|BIT15; 
        pals[1]=RGB15(31,31,31)|BIT15;
        pals[2]=RGB15(12,0,0)|BIT15;
        pals[3]=RGB15(31,31,0)|BIT15;
    }else{
        pals[0]=RGB15(230/8,235/8,236/8)|BIT15;
        pals[1]=RGB15(24,0,0)|BIT15;
        pals[2]=RGB15(0,0,0)|BIT15;
        pals[3]=0;
    }    
  
    u16 *psrc=(u16*)zd.pDstBuf;
    u32 *pdst=(u32*)pScreenMain->pViewCanvas->GetVRAMBuf();
    for(u32 idx=0;idx<(ScreenWidth*ScreenHeight)/2;idx++){
        u32 pal16=*psrc++;
        u32 col32=pals[pal16&0xff]|(pals[pal16>>8]<<16);
        *pdst++=col32;
    }
  
    zd.DstSize=0;
    safefree(&MM_Temp,zd.pDstBuf); zd.pDstBuf=NULL;
}

// ----------------------------------------

#define CurrentFolderVersion (0x20110605)

#include "versionerror_b8zlib.h"
static void CheckDataFolderVersion(void)
{
    u32 Version=0;
    
    FAT_FILE *pf=Shell_FAT_fopen_Root(FolderVersionFilename);
    
    if(pf!=NULL){
        FAT2_fread(&Version,1,4,pf);
        FAT2_fclose(pf);
        
        if(Version==CurrentFolderVersion) return;
    }
    
    DrawBootWarn(versionerror_b8zlib,versionerror_b8zlib_Size,1);
    StopFatalError(0,"Illigal data folder contents version."); 
}

//------------------------------------------------------

#include "romeo2_ncd.h"
DATA_IN_IWRAM_MainPass static void main_ins_start_ins_StartPass(void)
{
    pCglFontDefault=NULL;
    
    IlligalWritedNullPointer_Check(11);
    LoadINI();
    VerboseDebugLog=GlobalINI.System.DetailsDebugLog;
    
    //Start ARM7 debug log port.
    //_consoleInitPrintServer(GlobalINI.System.ARM7_DebugLogFlag);
    
    IlligalWritedNullPointer_Check(12);
    if(GlobalINI.DiskAdapter.SlowDiskAccess==false) FAT2_Disabled64kClusterMode();
    
    if((0x0b004000-1536)<(u32)pMTCMEND) StopFatalError(0,"MTCM overflow. 0x%08x 0x%08x\n",0x0b000000+1536,pMTCMEND);
    
    IlligalWritedNullPointer_Check(13);
    _consolePrint("Start FPGA Initializer.\n");
    
    FPGA_BusOwnerARM9();
    isExistsROMEO2=false;
    if(FPGA_isExistCartridge()==false){
        _consolePrint("Can not found ROMEO2 cartridge on GBA slot.\n");
    }else{
        bool halt=true;
    
        _consolePrintf("",romeo2_ncd);
        u32 FPGADataSize=OVM_GetRomeo2NCD_Size();
        void *pFPGAData=safemalloc_chkmem(&MM_Temp,FPGADataSize);
        OVM_GetRomeo2NCD_Data(pFPGAData,FPGADataSize);
    
        if(FPGA_CheckBitStreamFormat(pFPGAData,FPGADataSize)==false){
            _consolePrint("ROMEO2 FPGA bit stream unknown format?\n");
        }else{
            if(FPGA_Start(pFPGAData,FPGADataSize)==false){
                _consolePrint("ROMEO2 configration failed.\n");
            }else{
                GBABUS_OwnerARM7=true;
                SetARM9_REG_WaitCR();
                _consolePrint("Wait for ARM7 init.\n");
                IPC6->RequestFPGAInit=true;
                while(IPC6->RequestFPGAInit==true);
                _consolePrint("Initialized.\n");
                halt=false;
                isExistsROMEO2=true;
            }
        }
        safefree(&MM_Temp,pFPGAData); pFPGAData=NULL;
      
        if(halt==true) while(1);
    }
    IlligalWritedNullPointer_Check(14);
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    IlligalWritedNullPointer_Check(15);
    InitInterrupts();
    strpcmSetAudioVolume64(64);
    strpcmSetVideoVolume64(64);
    if(VerboseDebugLog==true) _consolePrint("strpcmSetAudioVolume64(64);\n");
    if(VerboseDebugLog==true) _consolePrint("strpcmSetVideoVolume64(64);\n");
    
    IlligalWritedNullPointer_Check(16);
    if(VerboseDebugLog==true) _consolePrint("Request RTC data from ARM7.\n");
    IPC6->curtimeFlag=true;
    while(IPC6->curtimeFlag==true){
        swiWaitForVBlank();
    }
    
#if 0    
    IlligalWritedNullPointer_Check(17);
    if(false){
        u32 UserLanguage=(u32)-1;
        u32 Timeout=0x10000;
          
        while(UserLanguage==(u32)-1){
            UserLanguage=IPC6->UserLanguage;
            Timeout--;
            if(Timeout==0){
                _consolePrint("NDS farmware language read error. ARM7CPU stopped...?\n");
                while(1);
            }
        }
        _consolePrintf("NDS farmware language ID : %d\n",UserLanguage);
    }
#endif
}

#include "bootwarn_b8zlib.h"

static __attribute__ ((noinline)) void main_ins_start(void)
{
    asm volatile (
		"mov r0,#0 \n\t"
		"MCR p15, 0, r0, c7, c10, 4 @ drain write buffer \n\t"
		:::"r0", "memory"
	);
  
    SetARM9_REG_WaitCR();
  
    powerOn(POWER_ALL_2D); // | POWER_SWAP_LCDS; // SWAPするとファイルリストが下
  
    _consoleInitLogFile();
  
    glSetFuncDebugPrint(_consolePrint);
    glDefaultMemorySetting();
    
    {
    	REG_BG2CNT_SUB = BG_COLOR_256 | BG_RS_64x64 | BG_MAP_BASE(8) | BG_TILE_BASE(0) | BG_PRIORITY_1; // Tile16kb Map2kb(64x32)
    
        BG_PALETTE_SUB[(0*16)+0] = RGB15(0,0,0); // unuse (transparent)
        BG_PALETTE_SUB[(0*16)+1] = RGB15(0,0,2) | BIT(15); // BG color
        BG_PALETTE_SUB[(0*16)+2] = RGB15(0,0,0) | BIT(15); // Shadow color
        BG_PALETTE_SUB[(0*16)+3] = RGB15(16,16,16) | BIT(15); // Text color
    
        u16 XDX=(u16)((8.0/6)*0x100);
        u16 YDY=(u16)((8.0/6)*0x100);
    
        REG_BG2PA_SUB = XDX;
        REG_BG2PB_SUB = 0;
        REG_BG2PC_SUB = 0;
        REG_BG2PD_SUB = YDY;
    
        REG_BG2X_SUB=1;
        REG_BG2Y_SUB=-1;
    
        //consoleInit() is a lot more flexible but this gets you up and running quick
        _consoleInitDefault((u16*)(SCREEN_BASE_BLOCK_SUB(8)), (u16*)(CHAR_BASE_BLOCK_SUB(0)));
    }
  
    {
        extern u32 Image_ER_DTCM_Base;
        extern u32 Image_ER_DTCM_Length;
        u32 exec_base=(u32)&Image_ER_DTCM_Base;
        u32 region_size=(u32)&Image_ER_DTCM_Length;
        _consolePrintf("DTCM top=0x%08x,end=0x%08x,size=0x%04x(%d)byte\n",exec_base,exec_base+region_size,region_size,region_size);
        pDTCMEND=(u32*)(exec_base+region_size);
    }
    
    {
        extern u32 Image_EOVR_MTCM_Base;
        extern u32 Image_EOVR_MTCM_Length;
        u32 exec_base=(u32)&Image_EOVR_MTCM_Base;
        u32 region_size=(u32)&Image_EOVR_MTCM_Length;
        _consolePrintf("MTCM top=0x%08x,end=0x%08x,size=0x%04x(%d)byte\n",exec_base,exec_base+region_size,region_size,region_size);
        pMTCMEND=(u32*)(exec_base+region_size);
        _consolePrintf("Stack top=0x%08x,end=0x%08x,size=0x%04x(%d)byte\n",(u32)pMTCMEND,TCM_StackStart,TCM_StackStart-(u32)pMTCMEND,TCM_StackStart-(u32)pMTCMEND);
    }
    
    {
        // setup stack overflow checker
        u32 *p=pDTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }
    
    MM_Init();
  
    _consolePrintf("boot %s %s\n%s\n%s\n%s\n",ROMTITLE,ROMVERSION,ROMDATE,ROMENV,ROMWEB); //bah, these are not literals any longer.
    _consolePrint("Original MoonShell2 by Moonlight.\n");
	_consolePrint("FishShell2 by Carpfish.\n");
	_consolePrint("ld script / inline asm GCC port by huiminghao.\n");
	_consolePrint("ld script fixed by avenir.\n");
    _consolePrintf("__current pc=0x%08x sp=0x%08x\n\n",__current_pc(),__current_sp());
    PrintFreeMem();
    
    //_consolePrintf("TProcState=%d \n",sizeof(TProcState));
    glDefaultClassCreate();

    {
        u64 *p=(u64*)NULL;
        *p=NULLSTR64bit;
        IlligalWritedNullPointer_Check(0);
        _consolePrintf("Test for NULL ptr. [%s]\n",NULL);
    }
  
    IlligalWritedNullPointer_Check(1);
    DrawBootWarn(bootwarn_b8zlib,bootwarn_b8zlib_Size,1);
    
    IlligalWritedNullPointer_Check(2);
    _consolePrintf("IPC6=0x%08x %dbyte.\n",IPC6,sizeof(TransferRegion6));
    
    IlligalWritedNullPointer_Check(3);
    SetARM9_REG_WaitCR();
    if(FAT2_InitFiles()==false){
        _consolePrint("FAT2_InitFiles() failed. broken disk or not enogth free area.\n");
        while(1);
    }
    SetARM9_REG_WaitCR();
    
    IlligalWritedNullPointer_Check(4);
    if(Shell_FAT_fopen_isExists_Root(LogFilename)==true){
        FAT_FILE *pf=Shell_FAT_fopen_Root(LogFilename);
        if(pf!=NULL){
            _consoleSetLogFile(pf);
            FAT2_fclose(pf);
        }
    }
    
    IlligalWritedNullPointer_Check(5);
    IPC6->DSCT_SDHCFlag=*(u32*)0x2fFFC24;
    _consolePrintf("DSCT_SDHCFlag= 0x%x.\n",IPC6->DSCT_SDHCFlag);
    
    IlligalWritedNullPointer_Check(6);
    {
        const char *pmsg="Unknown";
        switch(IPC6->DSType){
            case DST_DS: pmsg="DS"; break;
            case DST_DSLite: pmsg="DSLite"; break;
            case DST_DSi: pmsg="DSi"; break;
        }
        _consolePrintf("Boot with %s.\n",pmsg);
    }
    
    IlligalWritedNullPointer_Check(7);
    CheckDataFolderVersion();
    	
    IlligalWritedNullPointer_Check(9);
    OVM_Init();
    OVM_LoadAfterSystem();
    OVM_proc_MainPass(); 

    IlligalWritedNullPointer_Check(10);
    main_ins_start_ins_StartPass();
}

CODE_IN_AfterSystem static __attribute__ ((noinline)) void main_ins_end(void)
{
    _consolePrint("Check: FileSystem.\n");
    u32 OpenedCount=FAT2_GetOpenedFileHandlesCount();
    if(OpenedCount!=0) StopFatalError(0,"There is a file handle not closed. [%d]",OpenedCount); 
  
    //_consolePrint("Free: glDefaultClass.\n");
    //glDefaultClassFree();
}

static __attribute__ ((noinline)) void mainloop(void);

int main(void)
{
    REG_IME=0;
 
	REG_POWERCNT &= ~POWER_SWAP_LCDS;
	((char*)OverlayHeader_ID)[0]='O';
	extern u32 fake_heap_end;
	fake_heap_end=0x02394d00;
    main_ins_start();
  
    {
        SetARM9_REG_WaitCR();
        PrintFreeMem();
        mainloop();
        PrintFreeMem();
    }
  
    REG_IME=0;
  
    main_ins_end();
    
    _consolePrint("main terminated.\n");
  
    _consolePrintf("Reboot ROM '%s'\n",BootROM_GetFullPathAlias());
    BootNDSROM();
  
    return(0);
}

// -------------------------------- mainloop

CODE_IN_AfterSystem __attribute__ ((noinline)) void WaitForVBlank(void)
{
    if(VBlankPassedFlag==false){
        swiWaitForVBlank();
    }
    VBlankPassedFlag=false;
}

// ------------------------------------------------------------

DATA_IN_AfterSystem static TCallBack CallBack;

CODE_IN_AfterSystem static void CallBackInit(void)
{
    MemSet32CPU(0,&CallBack,sizeof(TCallBack));
}

CODE_IN_AfterSystem void CallBack_ExecuteVBlankHandler(void)
{
    if(WaitKeyRelease){
        if(IPC6->RequestUpdateIPC==false){
            u32 KEYS_Cur=(~REG_KEYINPUT)&0x3ff | IPC6->XYButtons;
            if(KEYS_Cur==0) WaitKeyRelease=false;
            IPC6->RequestUpdateIPC=true;
        }
    }
    
    if(CallBack.VBlankHandler!=NULL) CallBack.VBlankHandler();
}

CODE_IN_AfterSystem TCallBack* CallBack_GetPointer(void)
{
    return(&CallBack);
}

// ------------------------------------------------------------

DATA_IN_AfterSystem static bool mf;
DATA_IN_AfterSystem static s32 mx,my;

CODE_IN_AfterSystem static void Proc_TouchPad(u32 VsyncCount)
{
    if(IPC6->RequestUpdateIPC==true) return;
    
    bool tpress;
    s32 tx,ty;

    if((IPC6->XYButtons&KEY_TOUCH)==0){
        tpress=false;
        tx=0;
        ty=0;
    }else{
        tpress=true;
        tx=IPC6->touchXpx;
        ty=IPC6->touchYpx;
    }

    IPC6->RequestUpdateIPC=true;

    if(tpress==true){
        if(mf==false){
            mf=true;
            if(CallBack.MouseDown!=NULL) CallBack.MouseDown(tx,ty);
            mx=tx;
            my=ty;
        }else{
            s32 dx=abs(mx-tx);
            s32 dy=abs(my-ty);
            if((1<=dx)||(1<=dy)){
                if(CallBack.MouseMove!=NULL) CallBack.MouseMove(tx,ty);
                mx=tx;
                my=ty;
            }
        }
    }else{
        if(mf==true){
            mf=false;
            if(CallBack.MouseUp!=NULL) CallBack.MouseUp(mx,my);
        }
    }
}

#include "main_keyrepeat.h"

DATA_IN_AfterSystem static u32 KEYS_Last;
DATA_IN_AfterSystem static bool KEYS_PressedLR;
DATA_IN_AfterSystem static u32 KEYS_PressStartCount,KEYS_PressSelectCount;
DATA_IN_AfterSystem static bool KEYS_HPSwitch_Pressed;

CODE_IN_AfterSystem void Proc_KeyInput_Init(void)
{
    KEYS_Last=~0;
    KEYS_PressedLR=false;
    KEYS_PressStartCount=0;
    KEYS_PressSelectCount=0;
    KEYS_HPSwitch_Pressed=false;
}

#include "main_savepreview.h"

CODE_IN_AfterSystem void Proc_KeyInput(u32 VsyncCount)
{
    if(KeyRepeatFlag==true){ cwl();
        if(KeyRepeatCount<=VsyncCount){ cwl();
        KeyRepeatCount=0;
        }else{ cwl();
        KeyRepeatCount-=VsyncCount;
        }
    }
  
    u32 KEYS_Cur=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
    
    if(KEYS_Cur==0){
        if(CallBack.KeyReleases!=NULL) CallBack.KeyReleases(VsyncCount);
    }
    
    {
        const u32 Timeout=60*1;
        if((KEYS_Cur & KEY_START)!=0){
            if(KEYS_PressStartCount<Timeout){
                KEYS_PressStartCount+=VsyncCount;
                if(Timeout<=KEYS_PressStartCount){
                    if(CallBack.KeyLongPress!=NULL) CallBack.KeyLongPress(KEY_START);
                    KEYS_PressStartCount=Timeout;
                }
            }
        }else{
            if((KEYS_PressStartCount!=0)&&(KEYS_PressStartCount!=Timeout)){
                if(CallBack.KeyPress!=NULL) CallBack.KeyPress(VsyncCount,KEY_START);
            }
            KEYS_PressStartCount=0;
        }
        KEYS_Cur&=~KEY_START;
    }
  
    {
        const u32 Timeout=60*1;
        if((KEYS_Cur & KEY_SELECT)!=0){
            if(KEYS_PressSelectCount<Timeout){
                KEYS_PressSelectCount+=VsyncCount;
                if(Timeout<=KEYS_PressSelectCount){
                	if(ProcState.System.EnableScreenCapture){
                		Sound_Start(WAVFN_Notify);
                    	if(CallBack.KeyLongPress!=NULL) CallBack.KeyLongPress(KEY_SELECT);
                    	main_SavePreviewAndHalt();
                    	Sound_Start(WAVFN_Notify);
                    	StopFatalError(-0001,"Saved preview files. successed.\n");
                	}
                }
            }
        }else{
            if((KEYS_PressSelectCount!=0)&&(KEYS_PressSelectCount!=Timeout)){
                if(CallBack.KeyPress!=NULL) CallBack.KeyPress(VsyncCount,KEY_SELECT);
            }
            KEYS_PressSelectCount=0;
        }
        KEYS_Cur&=~KEY_SELECT;
    }
  
    if((KEYS_Cur&(KEY_L|KEY_R))==(KEY_L|KEY_R)){
        if(KEYS_PressedLR==false){
            KEYS_PressedLR=true;
            if(CallBack.KeySameLRDown!=NULL) CallBack.KeySameLRDown();
        }
    }
    if((KEYS_Cur&(KEY_L|KEY_R))!=(KEY_L|KEY_R)){
        if(KEYS_PressedLR==true){
            KEYS_PressedLR=false;
            if(CallBack.KeySameLRUp!=NULL) CallBack.KeySameLRUp();
        }
    }
  
    const u32 DupMask=KEY_A|KEY_B|KEY_X|KEY_Y|KEY_UP|KEY_DOWN|KEY_LEFT|KEY_RIGHT;
  
    if((KEYS_Last&DupMask)!=0) KEYS_Cur&=KEYS_Last;
  
    KEYS_Last=KEYS_Cur;
    KEYS_Cur=KeyRepeat_Proc(KEYS_Cur,VsyncCount);
  
    if(8<VsyncCount) VsyncCount=8;
  
    if(KEYS_Cur!=0){
        //if(isRumbleInserted()) setRumble(true);
        if(CallBack.KeyPress!=NULL) CallBack.KeyPress(VsyncCount,KEYS_Cur);
    }
}

DATA_IN_AfterSystem static bool Proc_PanelOpened_Last;

CODE_IN_AfterSystem static void Proc_PanelOpened(void)
{
    if(Proc_PanelOpened_Last!=IPC6->PanelOpened){
        Proc_PanelOpened_Last=IPC6->PanelOpened;
        if(Proc_PanelOpened_Last==true){
            if(ProcState.System.SpeakerPowerOffWhenPanelClosed) REG_POWERCNT |= POWER_LCD;
            if(CallBack.PanelOpen!=NULL) CallBack.PanelOpen();
        }else{
            if(ProcState.System.SpeakerPowerOffWhenPanelClosed) REG_POWERCNT &= ~POWER_LCD;
            if(CallBack.PanelClose!=NULL) CallBack.PanelClose();
        }
    }
}

#include "main_Trigger.h"

// ------------------------------------------------------------

DATA_IN_AfterSystem EProcFadeEffect ProcFadeEffect;

DATA_IN_AfterSystem_CONST static const u32 HorizontalFadeStepCount=35;
DATA_IN_AfterSystem_CONST static const u32 HorizontalFadeStep[HorizontalFadeStepCount]={2,4,4,6,6,8,8,8,8,10,10,10,10,10,10,10,10,10,10,10,10,8,8,8,8,8,8,6,6,6,4,4,4,2,2,};

DATA_IN_AfterSystem_CONST static const u32 VerticalFadeStepCount=27;
DATA_IN_AfterSystem_CONST static const u32 VerticalFadeStep[VerticalFadeStepCount]={2,4,5,7,8,8,9,9,9,9,9,9,9,9,9,9,9,9,8,8,7,7,6,5,4,3,2,};

CODE_IN_AfterSystem void ScreenMain_Flip_ProcFadeEffect(void)
{
    if(NextProc!=ENP_Loop) return;
  
    if(ProcState.System.EnableFadeEffect==false) ProcFadeEffect=EPFE_None;
  
    u16 *pviewbuf=pScreenMain->pViewCanvas->GetVRAMBuf();
    u16 *pbackbuf=pScreenMain->pBackCanvas->GetVRAMBuf();
  
    switch(ProcFadeEffect){
        case EPFE_None: {
        } break;
        case EPFE_LeftToRight: {
            u32 sx=0;
            for(u32 idx=0;idx<HorizontalFadeStepCount;idx++){
                u32 step=HorizontalFadeStep[idx];
                Splash_Update();
                swiWaitForVBlank();
                u16 tmpbuf[ScreenWidth];
                for(u32 y=0;y<ScreenHeight;y++){
                    u16 *pviewlinebuf=&pviewbuf[y*ScreenWidth];
                    u16 *pbacklinebuf=&pbackbuf[y*ScreenWidth];
                    MemCopy32CPU(&pviewlinebuf[0],tmpbuf,(ScreenWidth-step)*2);
                    MemCopy32CPU(tmpbuf,&pviewlinebuf[step],(ScreenWidth-step)*2);
                    MemCopy32CPU(&pbacklinebuf[ScreenWidth-step-sx],&pviewlinebuf[0],step*2);
                }
                sx+=step;
            }
        } break;
        case EPFE_RightToLeft: {
            u32 sx=0;
            for(u32 idx=0;idx<HorizontalFadeStepCount;idx++){
                u32 step=HorizontalFadeStep[idx];
                Splash_Update();
                swiWaitForVBlank();
                for(u32 y=0;y<ScreenHeight;y++){
                    u16 *pviewlinebuf=&pviewbuf[y*ScreenWidth];
                    u16 *pbacklinebuf=&pbackbuf[y*ScreenWidth];
                    MemCopy32CPU(&pviewlinebuf[step],&pviewlinebuf[0],(ScreenWidth-step)*2);
                    MemCopy32CPU(&pbacklinebuf[sx],&pviewlinebuf[ScreenWidth-step],step*2);
                }
                sx+=step;
            }
        } break;
        case EPFE_UpToDown: {
            u32 sy=0;
            for(u32 idx=0;idx<VerticalFadeStepCount;idx++){
                u32 step=VerticalFadeStep[idx];
                Splash_Update();
                swiWaitForVBlank();
                s32 y;
                y=ScreenHeight-(step*2);
                y=(y/step)*step;
                for(;y>=0;y-=step){
                    u16 *pviewlinebuf=&pviewbuf[y*ScreenWidth];
                    MemCopy32CPU(&pviewlinebuf[0*ScreenWidth],&pviewlinebuf[step*ScreenWidth],(step*ScreenWidth)*2);
                }
                u16 *pviewlinebuf=&pviewbuf[0*ScreenWidth];
                u16 *pbacklinebuf=&pbackbuf[0*ScreenWidth];
                MemCopy32CPU(&pbacklinebuf[(ScreenHeight-step-sy)*ScreenWidth],&pviewlinebuf[0*ScreenWidth],(step*ScreenWidth)*2);
                sy+=step;
            }
        } break;
        case EPFE_DownToUp: {
            u32 sy=0;
            for(u32 idx=0;idx<VerticalFadeStepCount;idx++){
                u32 step=VerticalFadeStep[idx];
                Splash_Update();
                swiWaitForVBlank();
                for(u32 y=0;y<ScreenHeight-step;y+=step){
                    u16 *pviewlinebuf=&pviewbuf[y*ScreenWidth];
                    MemCopy32CPU(&pviewlinebuf[step*ScreenWidth],&pviewlinebuf[0*ScreenWidth],(step*ScreenWidth)*2);
                }
                u16 *pviewlinebuf=&pviewbuf[0*ScreenWidth];
                u16 *pbacklinebuf=&pbackbuf[0*ScreenWidth];
                MemCopy32CPU(&pbacklinebuf[sy*ScreenWidth],&pviewlinebuf[(ScreenHeight-step)*ScreenWidth],(step*ScreenWidth)*2);
                sy+=step;
            }
        } break;
        case EPFE_CrossFade: {
            for(u32 idx=16;idx>0;idx--){
            	Splash_Update();
                WaitForVBlank();
                pScreenMain->SetBlendLevel(idx);
                while(VBlankPassedFlag==false){
                    DLLSound_UpdateLoop(true);
                }
            }
        } break;
        case EPFE_FastCrossFade: {
            for(u32 idx=16;idx>0;idx-=4){
            	Splash_Update();
                WaitForVBlank();
                pScreenMain->SetBlendLevel(idx);
                while(VBlankPassedFlag==false){
                    DLLSound_UpdateLoop(true);
                }
            }
        } break;
    }
    pScreenMain->Flip(true);
  
    if(ProcFadeEffect!=EPFE_None){
        ProcFadeEffect=EPFE_None;
        REG_IME=0;
        VBlankPassedCount=1;
        REG_IME=1;
    }
}

// ------------------------------------------------------------

CODE_IN_AfterSystem static void Proc_ExternalPowerPresent(void)
{
	if(!GlobalINI.FileList.CarSupplyMode) return;
	
	DATA_IN_AfterSystem static bool LastState;
  
	DATA_IN_AfterSystem static bool FirstStart=true;
    if(FirstStart==true){
        FirstStart=false;
        LastState=IPC6->ExternalPowerPresent;
        return;
    }
  
    bool curstate=IPC6->ExternalPowerPresent;
    if(LastState==curstate) return;
    LastState=curstate;
  
    if(curstate==true){
        if(CallBack.ExternalPowerAttach!=NULL) CallBack.ExternalPowerAttach();
    }else{
        if(CallBack.ExternalPowerDetach!=NULL) CallBack.ExternalPowerDetach();
    }
}

// ------------------------------------------------------------

DATA_IN_IWRAM_MainPass static void mainloop_autoboot(const char *pFilename)
{
	UnicodeChar PathUnicode[MaxFilenameLength],FilenameUnicode[MaxFilenameLength];
    UnicodeChar FullPathUnicode[MaxFilenameLength];
    StrConvert_Ank2Unicode(pFilename,FullPathUnicode);
    SplitItemFromFullPathUnicode(FullPathUnicode,PathUnicode,FilenameUnicode);
    if(FileExistsUnicode(PathUnicode,FilenameUnicode)==true){
        _consolePrintf("Auto booting... [%s]\n",pFilename);
        BootROM_SetInfo_NoLaunch(PathUnicode,FilenameUnicode,true);
    }
}

// ------------------------------------------------------------

CODE_IN_AfterSystem static __attribute__ ((noinline)) void mainloop_ins_DLLSound_Update(void)
{
    if(DLLSound_isOpened()==true){
        u32 vsynccount=VBlankPassedCount;
        DLLSound_UpdateLoop(false);
        if(strpcmRequestStop==true){
            if(CallBack.strpcmRequestStop!=NULL) CallBack.strpcmRequestStop();
        }
    }
}

// ------------------------------------------------------------

DATA_IN_IWRAM_MainPass static __attribute__ ((noinline)) void mainloop_ins_start_ins_LangInitAndLoadFont(void)
{
    Shell_FAT_fopen_LanguageInit();
    Splash_Update();
    
    FAT_FILE *pf=Shell_FAT_fopen_Language_chrglyph();
    if(pf==NULL) StopFatalError(10602,"Not found default font file.\n");

    u32 bufsize=FAT2_GetFileSize(pf);
    void *pdummy=(u8*)safemalloc_chkmem(&MM_Temp,(bufsize*2)+(192*1024));
    u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
    Splash_Update();
    extern bool ShowSplashUpdate;
    ShowSplashUpdate=true;
    FAT2_fread_fast(pbuf,1,bufsize,pf);
    FAT2_fclose(pf);
    ShowSplashUpdate=false;
    Splash_Update();
    
    safefree(&MM_Temp,pdummy); pdummy=NULL;
  
    if(pCglFontDefault!=NULL){
        delete pCglFontDefault; pCglFontDefault=NULL;
    }
    pCglFontDefault=new CglFont(&MM_System,(const u8*)pbuf,bufsize);
  
    safefree(&MM_Temp,pbuf); pbuf=NULL;
  
    pScreenMain->pBackCanvas->SetCglFont(pCglFontDefault);
    pScreenMain->pViewCanvas->SetCglFont(pCglFontDefault);
    pScreenMainOverlay->pCanvas->SetCglFont(pCglFontDefault);
    pScreenSub->pCanvas->SetCglFont(pCglFontDefault);
    
    _consolePrint("Loaded font for your language.\n");
    Splash_Update();
}

// ------------------------------------------------------------
  
DATA_IN_IWRAM_MainPass static __attribute__ ((noinline)) void mainloop_ins_start(void)
{
    {
        Sound_Open();
        Splash_Init();
        mainloop_ins_start_ins_LangInitAndLoadFont();
        
        extmem_Init();
        extmem_ShowMemoryInfo();
        {
            Lang_Load();
            Splash_Update();
            const char *pfmt=Lang_GetUTF8("DateTimeFormat");
            if(isStrEqual_NoCaseSensitive(pfmt,"YYYYMMDD")==true) Date_SetDateFormat(EDF_YMD);
            if(isStrEqual_NoCaseSensitive(pfmt,"DDMMYYYY")==true) Date_SetDateFormat(EDF_DMY);
            if(isStrEqual_NoCaseSensitive(pfmt,"MMDDYYYY")==true) Date_SetDateFormat(EDF_MDY);

        }
    
        //Splash_DrawInfo();
    }
  
    ProcState_Init();
    ProcState_Load();
  
    {
        
        _consolePrint("Get RTC data from ARM7.\n");
        DateTime_ResetNow();
        TDateTime dt=DateTime_GetNow();
        _consolePrintf("Current: %d/%d/%d ",dt.Date.Year,dt.Date.Month,dt.Date.Day);
        _consolePrintf("%s %d:%02d:%02d.\n",dt.Time.pAMPMStr,dt.Time.Hour12h,dt.Time.Min,dt.Time.Sec);
        
        TFAT2_TIME ft;
        ft.Year=dt.Date.Year;
        ft.Month=dt.Date.Month;
        ft.Day=dt.Date.Day;
        ft.Hour=dt.Time.Hour;
        ft.Minuts=dt.Time.Min;
        ft.Second=dt.Time.Sec;
        _consolePrint("Current date/time send to file system.\n");
        FAT2_SetSystemDateTime(ft);
        
        _consolePrint("Initialize random seed.\n");
        u32 cnt=((dt.Time.Min*60)+dt.Time.Sec)&0xff;
        for(u32 idx=0;idx<cnt;idx++){
            rand();
        }
    }
  
    EUC2Unicode_Init();
    EUC2Unicode_Load();
  
    ErrorDialog_Init();
  
    Component_SetFont(pCglFontDefault);
  
    if(!GlobalINI.System.ChildrenMode) {
        u32 keys=(~REG_KEYINPUT)&0x3ff;
        if(keys==(KEY_A|KEY_B|KEY_L|KEY_R)){
            ProcState_Clear();
            ProcState_RequestSave=true;
            ProcState_Save();
            ProcState_Load();
      
            LaunchState_Open();
            LaunchState_Clear();
            LaunchState_Save();
            LaunchState_Close();
                        
            VideoResume_Open();
            VideoResume_Clear();
            VideoResume_Save();
            VideoResume_Close();
                        
            Skin_ClearCustomBG_FileBody();
            Resume_Load();
            Resume_Clear();
            Resume_Save();
            PlayList_MakeBlank();
            
            pScreenSub->pCanvas->SetFontTextColor(RGB15(16,16,16)|BIT15);
            pScreenSub->pCanvas->TextOutA(8-1,96-1,"All settings were initialized.");
            pScreenSub->pCanvas->TextOutA(8-1,96+16-1,"Please release all key.");
            
            pScreenSub->pCanvas->SetFontTextColor(RGB15(31,31,31)|BIT15);
            pScreenSub->pCanvas->TextOutA(8+0,96+0,"All settings were initialized.");
            pScreenSub->pCanvas->TextOutA(8+0,96+16+0,"Please release all key.");
            while(1){
            	if(IPC6->RequestUpdateIPC==false){
            		u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
            		if(keys==0) break;
            		IPC6->RequestUpdateIPC=true;
            	}
            }
        }
    }
  
    Resume_Load();
  
    ApplyCurrentBacklightLevel();
    strpcmSetAudioVolume64(ProcState.System.AudioVolume64);
    strpcmSetVideoVolume64(ProcState.System.VideoVolume64);
    if(VerboseDebugLog==true) _consolePrintf("strpcmSetAudioVolume64(%d);\n",ProcState.System.AudioVolume64);
    if(VerboseDebugLog==true) _consolePrintf("strpcmSetVideoVolume64(%d);\n",ProcState.System.VideoVolume64);
  
    BootROM_Init();
  
    Shell_Init_SwapFile();
    Shell_Init_SwapFile_PrgJpeg();
  
    mf=false;
    mx=0;
    my=0;
  
    KEYS_Last=0;
    KeyRepeatLastKey=0;
    KeyRepeatFlag=false;
    KeyRepeatCount=0;
  
    DLLList_Init();
    Splash_Update();
    
    {
        UnicodeChar *pcpu=ProcState.FileList.CurrentPathUnicode;
        _consolePrintf("Check current path. [%s]\n",StrConvert_Unicode2Ank_Test(pcpu));
        const char *pPathAlias=ConvertFull_Unicode2Alias(pcpu,NULL);

        if(pPathAlias==NULL){
            _consolePrint("Set to default path.\n");
            pcpu[0]=(UnicodeChar)'/';
            pcpu[1]=(UnicodeChar)0;
        }else{
            if(FAT2_chdir_Alias(pPathAlias)==false){
                _consolePrint("Set to default path.\n");
                pcpu[0]=(UnicodeChar)'/';
                pcpu[1]=(UnicodeChar)0;
            }else{
                _consolePrintf("finded. [%s]\n",pPathAlias);
            }
        }
    }
  
    PrintFreeMem();
    Splash_Update();

    VBlankPassedFlag=false;
    VBlankPassedCount=0;
  
    _consolePrint("Set NextProc.\n");
  
    RelationalFile_Clear();
  
    NextProc=ENP_Loop;
  
    {
        u32 KEYS_Cur=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
        
        if((NextProc==ENP_Loop)&&(KEYS_Cur==0)) mainloop_autoboot(DefaultNFilename);
        if((NextProc==ENP_Loop)&&(KEYS_Cur==KEY_X)) mainloop_autoboot(DefaultXFilename);
        if((NextProc==ENP_Loop)&&(KEYS_Cur==KEY_Y)) mainloop_autoboot(DefaultYFilename);
        if(NextProc!=ENP_Loop) Splash_Free();
    }
    
    
    ExtLink_Init();
    Splash_Update();
  
    MM_Compact();
    //MM_ShowAllocated(&MM_System);
    MM_CheckOverRange();
    MM_CheckMemoryLeak(&MM_Temp);
    MM_CheckMemoryLeak(&MM_SystemAfter);
    MM_CheckMemoryLeak(&MM_Skin);
    MM_CheckMemoryLeak(&MM_DLLImage);
    MM_CheckMemoryLeak(&MM_DLLSound);
    MM_CheckMemoryLeak(&MM_PlayList);
    MM_CheckMemoryLeak(&MM_DLLDPG);
    MM_CheckMemoryLeak(&MM_Skin);
    MM_CheckMemoryLeak(&MM_Process);
  
    if(Skin_SetFilename(ProcState.System.SkinFilenameUnicode)==false){
        _consolePrint("Set default skin.\n");
        StrConvert_Ank2Unicode(DefaultSkinPath "/default.skn",ProcState.System.SkinFilenameUnicode);
        if(Skin_SetFilename(ProcState.System.SkinFilenameUnicode)==false) StopFatalError(10606,"Not found skin package.\n");
    }
    
    PlayList_Init();
    Splash_Update();
  
    if(NextProc==ENP_Loop){
    	if(!ProcState.System.EnableResumeFunction) Resume_SetResumeMode(ERM_None); // Disabled resume function.
        if(Resume_GetResumeMode()!=ERM_None){
            _consolePrint("Restart resume.\n");
            UnicodeChar PathUnicode[MaxFilenameLength],FilenameUnicode[MaxFilenameLength];
            SplitItemFromFullPathUnicode(Resume_GetFilename(),PathUnicode,FilenameUnicode);
            if(FileExistsUnicode(PathUnicode,FilenameUnicode)==true){
                Unicode_Copy(RelationalFilePathUnicode,PathUnicode);
                Unicode_Copy(RelationalFileNameUnicode,FilenameUnicode);
                RelationalFilePos=Resume_GetPos();
                Splash_Free();
                switch(Resume_GetResumeMode()){
                    case ERM_None: break;
                    case ERM_Video: {
                        pScreenSub->pCanvas->FillFull(ColorTable.Video.InitBG);
                        SetNextProc(ENP_DPGPlay,EPFE_None);
                    } break;
                    case ERM_Image: {
                        SetNextProc(ENP_ImageView,EPFE_None);
                    } break;
                    case ERM_Text: {
                        SetNextProc(ENP_TextView,EPFE_None);
                    } break;
                }
            }
            Resume_Clear();
        }
    }
    
    if(NextProc==ENP_Loop){
        if(ProcState.System.BootCheckDisk==false || GlobalINI.DiskAdapter.AlwaysDisabledCheckDisk==true){
            if(ProcState.System.SkipSetup==false){
                SetNextProc(ENP_Setup,EPFE_CrossFade);
            }else{
                switch(ProcState.System.LastState){
                    case ELS_FileList: SetNextProc(ENP_FileList,EPFE_CrossFade); break;
                    case ELS_Launch: SetNextProc(ENP_Launch,EPFE_CrossFade); break;
                }
            }
        }else{
            SetNextProc(ENP_ChkDsk,EPFE_CrossFade);
        }
    }
  
    //SetNextProc(ENP_AudioCustom,EPFE_CrossFade);
  
    Proc_PanelOpened_Last=IPC6->PanelOpened;
}

// ------------------------------------------------------------

CODE_IN_AfterSystem static __attribute__ ((noinline)) void mainloop_ins_end(void)
{
    _consolePrint("mainloop terminate...\n");
  
    _consolePrint("Free: Skin.\n");
    Skin_Free();
    Skin_CloseFile();
  
    _consolePrint("Free: Lang.\n");
    Lang_Free();
  
    _consolePrint("Free: Font.\n");
    if(pCglFontDefault!=NULL){
        delete pCglFontDefault; pCglFontDefault=NULL;
    }
  
    _consolePrint("Free: EUC2Unicode.\n");
    EUC2Unicode_Free();
    
  
    _consolePrint("Free: PlayList.\n");
    PlayList_Free();
  
    _consolePrint("Free: DLLList.\n");
    DLLList_Free();
  
    _consolePrint("Free: ExtLink.\n");
    ExtLink_Free();
  
    _consolePrint("Free: Sound.\n");
    Sound_Close();
    
    _consolePrint("Free: ProcState.\n");
    ProcState_Free();
    
    _consolePrint("Load: NDSROM loader.\n");
    OVM_libglobal_ndsrom();
    
    _consolePrint("Free: OverlayManager.\n");
    OVM_Free();
    
    _consolePrint("Free: ARM7_DebugLogBuf.\n");
    _consoleFreePrintServer();
    
    _consolePrint("Check: Memory.\n");
    MM_Compact();
    MM_CheckOverRange();
    MM_CheckMemoryLeak(&MM_Temp);
    MM_CheckMemoryLeak(&MM_System);
    MM_CheckMemoryLeak(&MM_SystemAfter);
    MM_CheckMemoryLeak(&MM_Skin);
    MM_CheckMemoryLeak(&MM_DLLImage);
    MM_CheckMemoryLeak(&MM_DLLSound);
    MM_CheckMemoryLeak(&MM_DLLDPG);
    MM_CheckMemoryLeak(&MM_PlayList);
    MM_CheckMemoryLeak(&MM_Process);
}

// ------------------------------------------------------------

DATA_IN_AfterSystem static bool chkstack;

CODE_IN_AfterSystem static __attribute__ ((noinline)) void mainloop_ins_loopstart(void)
{
    if(NextProc!=ENP_BootROM && NextProc!=ENP_TextMenu && !LongPressRequest){
        _consolePrintf("Wait for key releases. (NextProc=%d)\n",NextProc);
        while(1){
            if(IPC6->RequestUpdateIPC==false){
                u32 keys=((~REG_KEYINPUT)&0x3ff) | IPC6->XYButtons;
                if(keys==0) break;
                IPC6->RequestUpdateIPC=true;
            }
        }
    }
    
    LongPressRequest=false;
    
    chkstack=true;
    if(NextProc==ENP_DPGPlay) chkstack=false;
    
    if(chkstack==true){ // setup stack overflow checker
        u32 *p=pDTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }else{
        u32 *p=pMTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }
    
    CallBackInit();
    OVM_proc_Start(NextProc);
    
    switch(NextProc){
        case ENP_Loop: StopFatalError(10607,"Illigal process error! NextProc==ENP_Loop\n"); break;
        case ENP_ChkDsk: Skin_Load_ChkDsk(); ProcChkDsk_SetCallBack(&CallBack); break;
        case ENP_Setup: Skin_Load_Setup(); ProcSetup_SetCallBack(&CallBack); break;
        case ENP_FileList: Skin_Load_FileList(); ProcFileList_SetCallBack(&CallBack); break;
        case ENP_SysMenu: Skin_Load_SysMenu(); ProcSysMenu_SetCallBack(&CallBack); break;
        case ENP_DPGCustom: Skin_Load_DPGCustom(); ProcDPGCustom_SetCallBack(&CallBack); break;
        case ENP_DPGPlay: Skin_Load_DPGPlay(); ProcDPGPlay_SetCallBack(&CallBack); break;
        case ENP_ImageCustom: Skin_Load_ImageCustom(); ProcImageCustom_SetCallBack(&CallBack); break;
        case ENP_ImageView: Skin_Load_ImageView(); ProcImageView_SetCallBack(&CallBack); break;
        case ENP_TextCustom: Skin_Load_TextCustom(); ProcTextCustom_SetCallBack(&CallBack); break;
        case ENP_TextMenu: Skin_Load_TextMenu(); ProcTextMenu_SetCallBack(&CallBack); break;
        case ENP_TextView: Skin_Load_TextView(); ProcTextView_SetCallBack(&CallBack); break;
        case ENP_BinView: Skin_Load_BinView(); ProcBinView_SetCallBack(&CallBack); break;
        case ENP_Launch: Skin_Load_Launch(); ProcLaunch_SetCallBack(&CallBack); break;
        case ENP_Custom: Skin_Load_Custom(); ProcCustom_SetCallBack(&CallBack); break;
        case ENP_BootROM: Skin_Load_BootROM(); ProcBootROM_SetCallBack(&CallBack); break;
        case ENP_MemoEdit: Skin_Load_MemoEdit(); ProcMemoEdit_SetCallBack(&CallBack); break;
        case ENP_MemoList: Skin_Load_MemoList(); ProcMemoList_SetCallBack(&CallBack); break;
        case ENP_AudioCustom: Skin_Load_AudioCustom(); ProcAudioCustom_SetCallBack(&CallBack); break;
        default: StopFatalError(10608,"Unknown process error! NextProc==%d\n",NextProc); break;
    }
    
    if(NextProc!=ENP_ChkDsk) Splash_Free();
    
    if(chkstack==true){ // fast stack overflow checker
        DTCM_StackCheck(-1);
    }else{
        MTCM_StackCheck(-1);
    }
    
    NextProc=ENP_Loop;
    
    if(chkstack==true){ // setup stack overflow checker
        u32 *p=pDTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }else{
        u32 *p=pMTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }
    
    if(CallBack.Start!=NULL) CallBack.Start();
    
    if(chkstack==true){ // fast stack overflow checker
        DTCM_StackCheck(1000);
    }else{
        MTCM_StackCheck(1000);
    }
    
    if(chkstack==true){ // setup stack overflow checker
        u32 *p=pDTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }else{
        u32 *p=pMTCMEND;
        for(;p<(u32*)__current_sp();p++){
            *p=(u32)p;
        }
    }
    
    ProcState_Save();
    
    MM_CheckOverRange();
    
    PrintFreeMem();
    
    VBlankPassedFlag=false;
    VBlankPassedCount=0;
    Proc_TouchPad(0);
    Proc_KeyInput_Init();
    Proc_Trigger(true,0);
    
    REG_IME=0;
    VBlankPassedCount=0;
    REG_IME=1;
}

// ------------------------------------------------------------

CODE_IN_AfterSystem static __attribute__ ((noinline)) void mainloop_ins_loopend(void)
{
    if((GetNextProc()!=ENP_FileList)&&(GetNextProc()!=ENP_ImageView)&&(GetNextProc()!=ENP_TextView)){
        PlayList_Stop(false);
        PlayList_Free();
    }
    
    VBlankPassedFlag=false;
    VBlankPassedCount=0;
    if(CallBack.End!=NULL) CallBack.End();
    
    IPC6->LCDPowerControl=LCDPC_ON_BOTH;
    
    Skin_Free();
    
    MM_Compact();
    MM_CheckOverRange();
    MM_CheckMemoryLeak(&MM_Temp);
    MM_CheckMemoryLeak(&MM_Skin);
    MM_CheckMemoryLeak(&MM_DLLImage);
    //MM_CheckMemoryLeak(&MM_DLLSound);
    MM_CheckMemoryLeak(&MM_DLLDPG);
    //MM_CheckMemoryLeak(&MM_PlayList);
    MM_CheckMemoryLeak(&MM_Process);
    
    if(chkstack==true){
        DTCM_StackCheck(1001);
    }else{
        MTCM_StackCheck(1001);
    }
}

// ------------------------------------------------------------

CODE_IN_AfterSystem static __attribute__ ((noinline)) void mainloop(void)
{
    _consolePrint("mainloop.\n");
  
    mainloop_ins_start();
    
    OVM_LoadAfterSystem2();
    
    DTCM_StackCheck(0);
  
    _consolePrint("Start event loop...\n");
  
    while(1){
        strpcm_ExclusivePause=true;
        mainloop_ins_loopstart();
        DLLSound_UpdateLoop(false);
        strpcm_ExclusivePause=false;
    
        while(NextProc==ENP_Loop){
            mainloop_ins_DLLSound_Update();
      
            WaitForVBlank();
      
            REG_IME=0;
            u32 vsynccount=VBlankPassedCount;
            VBlankPassedCount=0;
            REG_IME=1;
            
            if(WaitKeyRelease){
                u32 KEYS_Cur=(~REG_KEYINPUT)&0x3ff | IPC6->XYButtons;
                if(KEYS_Cur==0) WaitKeyRelease=false;
            }
            
            if(CallBack.VsyncUpdate!=NULL) CallBack.VsyncUpdate(vsynccount);
            
            Proc_TouchPad(vsynccount);
            Proc_KeyInput(vsynccount);
            Proc_PanelOpened();
            Proc_Trigger(false,vsynccount);
            Proc_ExternalPowerPresent();
      
            if(chkstack==true){ // fast stack overflow checker
                DTCM_StackCheck(-1);
            }else{
                MTCM_StackCheck(-1);
            }
        }
    
        strpcm_ExclusivePause=true;
        mainloop_ins_loopend();
    
        if(BootROM_GetExecuteFlag()==true) break;
    }
  
    mainloop_ins_end();
  
    _consolePrint("mainloop terminated.\n");
}
