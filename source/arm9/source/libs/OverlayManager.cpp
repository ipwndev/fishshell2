
#include <nds.h>

#include "_console.h"
#include "_const.h"
#include "maindef.h"
#include "memtool.h"
#include "strtool.h"
#include "shell.h"
#include "fat2.h"
#include "disc_io.h"

#include "OverlayManager.h"

#include "OverlayManager_DFS.h"

#include "setarm9_reg_waitcr.h"

void OVM_Init(void)
{
  _consolePrintf("",OverlayHeader_ID_CanNotAccess);
  
  FAT_FILE *pf=Shell_FAT_fopen_Internal(OverlayDLLFilename);
  OVR_DFS_Init(pf);
  FAT2_fclose(pf);
  
  char str[64];
  OVR_DFS_Read32bit(str,64);
  if(isStrEqual(str,OverlayHeader_ID)==false) StopFatalError(0,"overlay.dll file ID error. ('%s'!='%s')\n",OverlayHeader_ID,str);
}

CODE_IN_AfterSystem2 void OVM_Free(void)
{
  OVR_DFS_Free();
}

void ICache_AllFlush(void)
{
	asm volatile (
	"mov r0,#0 \n\t"
	"MCR p15, 0, r0, c7, c5, 0 \n\t"
	"bx lr \n\t"
	:::"memory"
	);
}

DATA_IN_IWRAM_MainPass u32 OVM_GetRomeo2NCD_Size(void)
{
  extern u32 Image_EOVR_Romeo2NCD_Length;
  u32 region_size=(u32)&Image_EOVR_Romeo2NCD_Length;
  return(region_size);
}

DATA_IN_IWRAM_MainPass void OVM_GetRomeo2NCD_Data(void *pdstbuf,u32 dstbufsize)
{
  extern u32 Load_EOVR_Romeo2NCD_Base;
  u32 load_base=(u32)&Load_EOVR_Romeo2NCD_Base;
  
  extern u32 Load_EOVR_Header_Base;
  load_base-=(u32)&Load_EOVR_Header_Base;
  
  _consolePrintf("romeo2.ncd: load=0x%08x, size=0x%04x.\n",load_base,dstbufsize);
  
  OVR_DFS_SetOffset(load_base);
  OVR_DFS_Read32bit(pdstbuf,dstbufsize);
}


static void OVM_Load(const char *pfuncname,u32 load_base,u32 exec_base,u32 region_size)
{
  if((load_base==0)||(exec_base==0)) StopFatalError(0,"%s: Overlay region error.\n",pfuncname);
  
  if(region_size==0){
    _consolePrintf("%s: This region is blank.\n",pfuncname);
    return;
  }
  extern u32 Load_EOVR_Header_Base;
  _consolePrintf("%s: load=0x%08x, exec=0x%08x, size=0x%04x.\n",pfuncname,load_base,exec_base,region_size);
  

  load_base-=(u32)&Load_EOVR_Header_Base;
//  if(VerboseDebugLog==true) _consolePrintf("ICache_AllFlush();\n");
  //ICache_AllFlush(); //bah, will crash fs2
	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();
//  if(VerboseDebugLog==true) _consolePrintf("DCache_FlushRangeOverrun(...);\n");
  u8 *pdst=(u8*)exec_base;
  //DCache_FlushRangeOverrun(pdst,region_size);

//  if(VerboseDebugLog==true) _consolePrintf("OVR_DFS_SetOffset(...);\n");
  OVR_DFS_SetOffset(load_base);
//  if(VerboseDebugLog==true) _consolePrintf("OVR_DFS_Read32bit(...);\n");
  OVR_DFS_Read32bit(pdst,region_size); // *** warn: memUncached() method will kill ITCM loading... ***

  if(false){
    u8 *ptmp=(u8*)malloc(region_size);
    OVR_DFS_SetOffset(load_base);
    OVR_DFS_Read32bit(ptmp,region_size);
    for(u32 idx=0;idx<region_size;idx++){
      if(ptmp[idx]!=pdst[idx]){
        StopFatalError(0,"EOVR: Read error. ofs=0x%x, %x,%x",load_base+idx,ptmp[idx],pdst[idx]);
      }
    }
    free(ptmp);
  }
  
//  if(VerboseDebugLog==true) _consolePrintf("DCache_CleanRangeOverrun(...);\n");
  //DCache_CleanRangeOverrun(pdst,region_size);
  
//  if(VerboseDebugLog==true) _consolePrintf("ICache_AllFlush();\n");
  //ICache_AllFlush();
  	IC_InvalidateAll();
	DC_FlushAll();
	DC_InvalidateAll();

	if((((u32)pdst)&0xff000000)==0x01000000){
		//if(op!=*(u32*)pdst)
		//	StopFatalError(0,"%s: Instruction %08x is different from ITCM written value %08x. PLEASE RECOMPILE(Developer Fault).\n",pfuncname,op,*(u32*)pdst);
		extern u32 __itcm_end;
		if((u32)(&__itcm_end)>(u32)pdst)
			StopFatalError(0,"Load address of %s: exec=%08x kills __itcm_end=%08x. PLEASE RECOMPILE(Developer Fault).\n",pfuncname,pdst,&__itcm_end);
	}

  _consolePrint("Overlay region loaded.\n");
}

DATA_IN_AfterSystem static ENextProc OVM_proc_mode=ENP_Loop;

CODE_IN_AfterSystem void OVM_proc_MainPass(void)
{
  OVM_proc_mode=ENP_Loop;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_proc_MainPass_Base;
  extern u32 Image_EOVR_proc_MainPass_Base;
  extern u32 Image_EOVR_proc_MainPass_Length;
  load_base=(u32)&Load_EOVR_proc_MainPass_Base;
  exec_base=(u32)&Image_EOVR_proc_MainPass_Base;
  region_size=(u32)&Image_EOVR_proc_MainPass_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_proc_Start(ENextProc NextProc)
{
  if(OVM_proc_mode==NextProc) return;
  OVM_proc_mode=NextProc;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  switch(NextProc){
    case ENP_Loop: StopFatalError(0,"OVM_proc_Start: Illigal process error! NextProc==ENP_Loop\n"); break;
    case ENP_ChkDsk: {
      extern u32 Load_EOVR_proc_ChkDsk_Base;
      extern u32 Image_EOVR_proc_ChkDsk_Base;
      extern u32 Image_EOVR_proc_ChkDsk_Length;
      load_base=(u32)&Load_EOVR_proc_ChkDsk_Base;
      exec_base=(u32)&Image_EOVR_proc_ChkDsk_Base;
      region_size=(u32)&Image_EOVR_proc_ChkDsk_Length;
    } break;
    case ENP_Setup: {
      extern u32 Load_EOVR_proc_Setup_Base;
      extern u32 Image_EOVR_proc_Setup_Base;
      extern u32 Image_EOVR_proc_Setup_Length;
      load_base=(u32)&Load_EOVR_proc_Setup_Base;
      exec_base=(u32)&Image_EOVR_proc_Setup_Base;
      region_size=(u32)&Image_EOVR_proc_Setup_Length;
    } break;
    case ENP_FileList: {
      extern u32 Load_EOVR_proc_FileList_Base;
      extern u32 Image_EOVR_proc_FileList_Base;
      extern u32 Image_EOVR_proc_FileList_Length;
      load_base=(u32)&Load_EOVR_proc_FileList_Base;
      exec_base=(u32)&Image_EOVR_proc_FileList_Base;
      region_size=(u32)&Image_EOVR_proc_FileList_Length;
    } break;
    case ENP_SysMenu: {
      extern u32 Load_EOVR_proc_SysMenu_Base;
      extern u32 Image_EOVR_proc_SysMenu_Base;
      extern u32 Image_EOVR_proc_SysMenu_Length;
      load_base=(u32)&Load_EOVR_proc_SysMenu_Base;
      exec_base=(u32)&Image_EOVR_proc_SysMenu_Base;
      region_size=(u32)&Image_EOVR_proc_SysMenu_Length;
    } break;
    case ENP_DPGCustom: {
      extern u32 Load_EOVR_proc_DPGCustom_Base;
      extern u32 Image_EOVR_proc_DPGCustom_Base;
      extern u32 Image_EOVR_proc_DPGCustom_Length;
      load_base=(u32)&Load_EOVR_proc_DPGCustom_Base;
      exec_base=(u32)&Image_EOVR_proc_DPGCustom_Base;
      region_size=(u32)&Image_EOVR_proc_DPGCustom_Length;
    } break;
    case ENP_DPGPlay: {
      extern u32 Load_EOVR_proc_DPGPlay_Base;
      extern u32 Image_EOVR_proc_DPGPlay_Base;
      extern u32 Image_EOVR_proc_DPGPlay_Length;
      load_base=(u32)&Load_EOVR_proc_DPGPlay_Base;
      exec_base=(u32)&Image_EOVR_proc_DPGPlay_Base;
      region_size=(u32)&Image_EOVR_proc_DPGPlay_Length;
    } break;
    case ENP_ImageCustom: {
      extern u32 Load_EOVR_proc_ImageCustom_Base;
      extern u32 Image_EOVR_proc_ImageCustom_Base;
      extern u32 Image_EOVR_proc_ImageCustom_Length;
      load_base=(u32)&Load_EOVR_proc_ImageCustom_Base;
      exec_base=(u32)&Image_EOVR_proc_ImageCustom_Base;
      region_size=(u32)&Image_EOVR_proc_ImageCustom_Length;
    } break;
    case ENP_ImageView: {
      extern u32 Load_EOVR_proc_ImageView_Base;
      extern u32 Image_EOVR_proc_ImageView_Base;
      extern u32 Image_EOVR_proc_ImageView_Length;
      load_base=(u32)&Load_EOVR_proc_ImageView_Base;
      exec_base=(u32)&Image_EOVR_proc_ImageView_Base;
      region_size=(u32)&Image_EOVR_proc_ImageView_Length;
    } break;
    case ENP_TextCustom: {
      extern u32 Load_EOVR_proc_TextCustom_Base;
      extern u32 Image_EOVR_proc_TextCustom_Base;
      extern u32 Image_EOVR_proc_TextCustom_Length;
      load_base=(u32)&Load_EOVR_proc_TextCustom_Base;
      exec_base=(u32)&Image_EOVR_proc_TextCustom_Base;
      region_size=(u32)&Image_EOVR_proc_TextCustom_Length;
    } break;
    case ENP_TextMenu: {
    	extern u32 Load_EOVR_proc_TextMenu_Base;
    	extern u32 Image_EOVR_proc_TextMenu_Base;
    	extern u32 Image_EOVR_proc_TextMenu_Length;
    	load_base=(u32)&Load_EOVR_proc_TextMenu_Base;
    	exec_base=(u32)&Image_EOVR_proc_TextMenu_Base;
    	region_size=(u32)&Image_EOVR_proc_TextMenu_Length;
    } break;
    case ENP_TextView: {
      extern u32 Load_EOVR_proc_TextView_Base;
      extern u32 Image_EOVR_proc_TextView_Base;
      extern u32 Image_EOVR_proc_TextView_Length;
      load_base=(u32)&Load_EOVR_proc_TextView_Base;
      exec_base=(u32)&Image_EOVR_proc_TextView_Base;
      region_size=(u32)&Image_EOVR_proc_TextView_Length;
    } break;
    case ENP_BinView: {
    	extern u32 Load_EOVR_proc_BinView_Base;
    	extern u32 Image_EOVR_proc_BinView_Base;
    	extern u32 Image_EOVR_proc_BinView_Length;
    	load_base=(u32)&Load_EOVR_proc_BinView_Base;
    	exec_base=(u32)&Image_EOVR_proc_BinView_Base;
    	region_size=(u32)&Image_EOVR_proc_BinView_Length;
    } break;
    case ENP_Launch: {
      extern u32 Load_EOVR_proc_Launch_Base;
      extern u32 Image_EOVR_proc_Launch_Base;
      extern u32 Image_EOVR_proc_Launch_Length;
      load_base=(u32)&Load_EOVR_proc_Launch_Base;
      exec_base=(u32)&Image_EOVR_proc_Launch_Base;
      region_size=(u32)&Image_EOVR_proc_Launch_Length;
    } break;
    case ENP_Custom: {
      extern u32 Load_EOVR_proc_Custom_Base;
      extern u32 Image_EOVR_proc_Custom_Base;
      extern u32 Image_EOVR_proc_Custom_Length;
      load_base=(u32)&Load_EOVR_proc_Custom_Base;
      exec_base=(u32)&Image_EOVR_proc_Custom_Base;
      region_size=(u32)&Image_EOVR_proc_Custom_Length;
    } break;
    case ENP_BootROM: {
      extern u32 Load_EOVR_proc_BootROM_Base;
      extern u32 Image_EOVR_proc_BootROM_Base;
      extern u32 Image_EOVR_proc_BootROM_Length;
      load_base=(u32)&Load_EOVR_proc_BootROM_Base;
      exec_base=(u32)&Image_EOVR_proc_BootROM_Base;
      region_size=(u32)&Image_EOVR_proc_BootROM_Length;
    } break;
    case ENP_MemoEdit: {
      extern u32 Load_EOVR_proc_MemoEdit_Base;
      extern u32 Image_EOVR_proc_MemoEdit_Base;
      extern u32 Image_EOVR_proc_MemoEdit_Length;
      load_base=(u32)&Load_EOVR_proc_MemoEdit_Base;
      exec_base=(u32)&Image_EOVR_proc_MemoEdit_Base;
      region_size=(u32)&Image_EOVR_proc_MemoEdit_Length;
    } break;
    case ENP_AudioCustom: {
      extern u32 Load_EOVR_proc_AudioCustom_Base;
      extern u32 Image_EOVR_proc_AudioCustom_Base;
      extern u32 Image_EOVR_proc_AudioCustom_Length;
      load_base=(u32)&Load_EOVR_proc_AudioCustom_Base;
      exec_base=(u32)&Image_EOVR_proc_AudioCustom_Base;
      region_size=(u32)&Image_EOVR_proc_AudioCustom_Length;
    } break;
    /*case ENP_MemoList: {
      extern u32 Load_EOVR_proc_MemoList_Base;
      extern u32 Image_EOVR_proc_MemoList_Base;
      extern u32 Image_EOVR_proc_MemoList_Length;
      load_base=(u32)&Load_EOVR_proc_MemoList_Base;
      exec_base=(u32)&Image_EOVR_proc_MemoList_Base;
      region_size=(u32)&Image_EOVR_proc_MemoList_Length;
    } break;
    case ENP_AudioPlay: {
      extern u32 Load_EOVR_proc_AudioPlay_Base;
      extern u32 Image_EOVR_proc_AudioPlay_Base;
      extern u32 Image_EOVR_proc_AudioPlay_Length;
      load_base=(u32)&Load_EOVR_proc_AudioPlay_Base;
      exec_base=(u32)&Image_EOVR_proc_AudioPlay_Base;
      region_size=(u32)&Image_EOVR_proc_AudioPlay_Length;
    } break;*/
    default: StopFatalError(0,"OVM_proc_Start: Unknown process error! NextProc==%d\n",NextProc); break;
  }
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

DATA_IN_AfterSystem2 static u32 OVM_libsnd_mode=0;

CODE_IN_AfterSystem2 void OVM_libsnd_mp3(void)
{
  if(OVM_libsnd_mode==1) return;
  OVM_libsnd_mode=1;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libsnd_mp3_Base;
  extern u32 Image_EOVR_libsnd_mp3_Base;
  extern u32 Image_EOVR_libsnd_mp3_Length;
  load_base=(u32)&Load_EOVR_libsnd_mp3_Base;
  exec_base=(u32)&Image_EOVR_libsnd_mp3_Base;
  region_size=(u32)&Image_EOVR_libsnd_mp3_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libsnd_mp3_Base;
  extern u32 Image_EOVR_ITCM_libsnd_mp3_Base;
  extern u32 Image_EOVR_ITCM_libsnd_mp3_Length;
  load_base=(u32)&Load_EOVR_ITCM_libsnd_mp3_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libsnd_mp3_Base;
  region_size=(u32)&Image_EOVR_ITCM_libsnd_mp3_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libsnd_midi(void)
{
  if(OVM_libsnd_mode==2) return;
  OVM_libsnd_mode=2;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libsnd_midi_Base;
  extern u32 Image_EOVR_libsnd_midi_Base;
  extern u32 Image_EOVR_libsnd_midi_Length;
  load_base=(u32)&Load_EOVR_libsnd_midi_Base;
  exec_base=(u32)&Image_EOVR_libsnd_midi_Base;
  region_size=(u32)&Image_EOVR_libsnd_midi_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libsnd_midi_Base;
  extern u32 Image_EOVR_ITCM_libsnd_midi_Base;
  extern u32 Image_EOVR_ITCM_libsnd_midi_Length;
  load_base=(u32)&Load_EOVR_ITCM_libsnd_midi_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libsnd_midi_Base;
  region_size=(u32)&Image_EOVR_ITCM_libsnd_midi_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libsnd_gme(void)
{
  if(OVM_libsnd_mode==3) return;
  OVM_libsnd_mode=3;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libsnd_gme_Base;
  extern u32 Image_EOVR_libsnd_gme_Base;
  extern u32 Image_EOVR_libsnd_gme_Length;
  load_base=(u32)&Load_EOVR_libsnd_gme_Base;
  exec_base=(u32)&Image_EOVR_libsnd_gme_Base;
  region_size=(u32)&Image_EOVR_libsnd_gme_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libsnd_gme_Base;
  extern u32 Image_EOVR_ITCM_libsnd_gme_Base;
  extern u32 Image_EOVR_ITCM_libsnd_gme_Length;
  load_base=(u32)&Load_EOVR_ITCM_libsnd_gme_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libsnd_gme_Base;
  region_size=(u32)&Image_EOVR_ITCM_libsnd_gme_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libsnd_ogg(void)
{
  if(OVM_libsnd_mode==4) return;
  OVM_libsnd_mode=4;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libsnd_ogg_Base;
  extern u32 Image_EOVR_libsnd_ogg_Base;
  extern u32 Image_EOVR_libsnd_ogg_Length;
  load_base=(u32)&Load_EOVR_libsnd_ogg_Base;
  exec_base=(u32)&Image_EOVR_libsnd_ogg_Base;
  region_size=(u32)&Image_EOVR_libsnd_ogg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libsnd_ogg_Base;
  extern u32 Image_EOVR_ITCM_libsnd_ogg_Base;
  extern u32 Image_EOVR_ITCM_libsnd_ogg_Length;
  load_base=(u32)&Load_EOVR_ITCM_libsnd_ogg_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libsnd_ogg_Base;
  region_size=(u32)&Image_EOVR_ITCM_libsnd_ogg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libsnd_wave(void)
{
  if(OVM_libsnd_mode==5) return;
  OVM_libsnd_mode=5;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libsnd_wave_Base;
  extern u32 Image_EOVR_libsnd_wave_Base;
  extern u32 Image_EOVR_libsnd_wave_Length;
  load_base=(u32)&Load_EOVR_libsnd_wave_Base;
  exec_base=(u32)&Image_EOVR_libsnd_wave_Base;
  region_size=(u32)&Image_EOVR_libsnd_wave_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libsnd_wave_Base;
  extern u32 Image_EOVR_ITCM_libsnd_wave_Base;
  extern u32 Image_EOVR_ITCM_libsnd_wave_Length;
  load_base=(u32)&Load_EOVR_ITCM_libsnd_wave_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libsnd_wave_Base;
  region_size=(u32)&Image_EOVR_ITCM_libsnd_wave_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

DATA_IN_AfterSystem2 static u32 OVM_libimg_mode=0;

CODE_IN_AfterSystem2 void OVM_libimg_jpeg(void)
{
  if(OVM_libimg_mode==1) return;
  OVM_libimg_mode=1;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libimg_jpeg_Base;
  extern u32 Image_EOVR_libimg_jpeg_Base;
  extern u32 Image_EOVR_libimg_jpeg_Length;
  load_base=(u32)&Load_EOVR_libimg_jpeg_Base;
  exec_base=(u32)&Image_EOVR_libimg_jpeg_Base;
  region_size=(u32)&Image_EOVR_libimg_jpeg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libimg_jpeg_Base;
  extern u32 Image_EOVR_ITCM_libimg_jpeg_Base;
  extern u32 Image_EOVR_ITCM_libimg_jpeg_Length;
  load_base=(u32)&Load_EOVR_ITCM_libimg_jpeg_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libimg_jpeg_Base;
  region_size=(u32)&Image_EOVR_ITCM_libimg_jpeg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libimg_png(void)
{
  if(OVM_libimg_mode==2) return;
  OVM_libimg_mode=2;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libimg_png_Base;
  extern u32 Image_EOVR_libimg_png_Base;
  extern u32 Image_EOVR_libimg_png_Length;
  load_base=(u32)&Load_EOVR_libimg_png_Base;
  exec_base=(u32)&Image_EOVR_libimg_png_Base;
  region_size=(u32)&Image_EOVR_libimg_png_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libimg_png_Base;
  extern u32 Image_EOVR_ITCM_libimg_png_Base;
  extern u32 Image_EOVR_ITCM_libimg_png_Length;
  load_base=(u32)&Load_EOVR_ITCM_libimg_png_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libimg_png_Base;
  region_size=(u32)&Image_EOVR_ITCM_libimg_png_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libimg_bmp(void)
{
  if(OVM_libimg_mode==3) return;
  OVM_libimg_mode=3;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libimg_bmp_Base;
  extern u32 Image_EOVR_libimg_bmp_Base;
  extern u32 Image_EOVR_libimg_bmp_Length;
  load_base=(u32)&Load_EOVR_libimg_bmp_Base;
  exec_base=(u32)&Image_EOVR_libimg_bmp_Base;
  region_size=(u32)&Image_EOVR_libimg_bmp_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libimg_bmp_Base;
  extern u32 Image_EOVR_ITCM_libimg_bmp_Base;
  extern u32 Image_EOVR_ITCM_libimg_bmp_Length;
  load_base=(u32)&Load_EOVR_ITCM_libimg_bmp_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libimg_bmp_Base;
  region_size=(u32)&Image_EOVR_ITCM_libimg_bmp_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libimg_gif(void)
{
  if(OVM_libimg_mode==4) return;
  OVM_libimg_mode=4;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libimg_gif_Base;
  extern u32 Image_EOVR_libimg_gif_Base;
  extern u32 Image_EOVR_libimg_gif_Length;
  load_base=(u32)&Load_EOVR_libimg_gif_Base;
  exec_base=(u32)&Image_EOVR_libimg_gif_Base;
  region_size=(u32)&Image_EOVR_libimg_gif_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libimg_gif_Base;
  extern u32 Image_EOVR_ITCM_libimg_gif_Base;
  extern u32 Image_EOVR_ITCM_libimg_gif_Length;
  load_base=(u32)&Load_EOVR_ITCM_libimg_gif_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libimg_gif_Base;
  region_size=(u32)&Image_EOVR_ITCM_libimg_gif_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libimg_psd(void)
{
  if(OVM_libimg_mode==5) return;
  OVM_libimg_mode=5;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libimg_psd_Base;
  extern u32 Image_EOVR_libimg_psd_Base;
  extern u32 Image_EOVR_libimg_psd_Length;
  load_base=(u32)&Load_EOVR_libimg_psd_Base;
  exec_base=(u32)&Image_EOVR_libimg_psd_Base;
  region_size=(u32)&Image_EOVR_libimg_psd_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libimg_psd_Base;
  extern u32 Image_EOVR_ITCM_libimg_psd_Base;
  extern u32 Image_EOVR_ITCM_libimg_psd_Length;
  load_base=(u32)&Load_EOVR_ITCM_libimg_psd_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libimg_psd_Base;
  region_size=(u32)&Image_EOVR_ITCM_libimg_psd_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libglobal_dpg(void)
{
  OVM_libsnd_mode=0;
  OVM_libimg_mode=0;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libglobal_dpg_Base;
  extern u32 Image_EOVR_libglobal_dpg_Base;
  extern u32 Image_EOVR_libglobal_dpg_Length;
  load_base=(u32)&Load_EOVR_libglobal_dpg_Base;
  exec_base=(u32)&Image_EOVR_libglobal_dpg_Base;
  region_size=(u32)&Image_EOVR_libglobal_dpg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libglobal_dpg_Base;
  extern u32 Image_EOVR_ITCM_libglobal_dpg_Base;
  extern u32 Image_EOVR_ITCM_libglobal_dpg_Length;
  load_base=(u32)&Load_EOVR_ITCM_libglobal_dpg_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libglobal_dpg_Base;
  region_size=(u32)&Image_EOVR_ITCM_libglobal_dpg_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_MTCM_Base;
  extern u32 Image_EOVR_MTCM_Base;
  extern u32 Image_EOVR_MTCM_Length;
  load_base=(u32)&Load_EOVR_MTCM_Base;
  exec_base=(u32)&Image_EOVR_MTCM_Base;
  region_size=(u32)&Image_EOVR_MTCM_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

CODE_IN_AfterSystem2 void OVM_libglobal_ndsrom(void)
{
  OVM_libsnd_mode=0;
  OVM_libimg_mode=0;
  
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_libglobal_ndsrom_Base;
  extern u32 Image_EOVR_libglobal_ndsrom_Base;
  extern u32 Image_EOVR_libglobal_ndsrom_Length;
  load_base=(u32)&Load_EOVR_libglobal_ndsrom_Base;
  exec_base=(u32)&Image_EOVR_libglobal_ndsrom_Base;
  region_size=(u32)&Image_EOVR_libglobal_ndsrom_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_ITCM_libglobal_ndsrom_Base;
  extern u32 Image_EOVR_ITCM_libglobal_ndsrom_Base;
  extern u32 Image_EOVR_ITCM_libglobal_ndsrom_Length;
  load_base=(u32)&Load_EOVR_ITCM_libglobal_ndsrom_Base;
  exec_base=(u32)&Image_EOVR_ITCM_libglobal_ndsrom_Base;
  region_size=(u32)&Image_EOVR_ITCM_libglobal_ndsrom_Length;
    
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  extern u32 Load_EOVR_libglobal_RebootLoader_Base;
  extern u32 Image_EOVR_libglobal_RebootLoader_Base;
  extern u32 Image_EOVR_libglobal_RebootLoader_Length;
  load_base=(u32)&Load_EOVR_libglobal_RebootLoader_Base;
  exec_base=(u32)&Image_EOVR_libglobal_RebootLoader_Base;
  region_size=(u32)&Image_EOVR_libglobal_RebootLoader_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
}

// ---------------------------------------------

void OVM_LoadAfterSystem(void)
{
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_AfterSystem_Base;
  extern u32 Image_EOVR_AfterSystem_Base;
  extern u32 Image_EOVR_AfterSystem_Length;
  load_base=(u32)&Load_EOVR_AfterSystem_Base;
  exec_base=(u32)&Image_EOVR_AfterSystem_Base;
  region_size=(u32)&Image_EOVR_AfterSystem_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  OVM_proc_mode=ENP_Loop;
  OVM_libsnd_mode=0;
  OVM_libimg_mode=0;
}

DATA_IN_IWRAM_MainPass void OVM_LoadAfterSystem2(void)
{
  u32 load_base=0,exec_base=0,region_size=0;
  
  extern u32 Load_EOVR_AfterSystem2_Base;
  extern u32 Image_EOVR_AfterSystem2_Base;
  extern u32 Image_EOVR_AfterSystem2_Length;
  load_base=(u32)&Load_EOVR_AfterSystem2_Base;
  exec_base=(u32)&Image_EOVR_AfterSystem2_Base;
  region_size=(u32)&Image_EOVR_AfterSystem2_Length;
  
  OVM_Load(__FUNCTION__,load_base,exec_base,region_size);
  
  OVM_proc_mode=ENP_Loop;
  OVM_libsnd_mode=0;
  OVM_libimg_mode=0;
}

