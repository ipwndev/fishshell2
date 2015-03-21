
#include <stdio.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"

#include "memtool.h"

#include "internaldrivers.h"

#include "libjpeg/jpeglib.h"
#include "libjpeg/jpegint.h"
#include "ErrorDialog.h"

#include "fat2.h"
#include "shell.h"

#ifndef ExceptJpeg

bool PlugJpeg_Start(FAT_FILE *_FileHandle,bool EnabledAutoFitting)
{
  return(false);
}

void PlugJpeg_Free(void)
{
}

void PlugJpeg_GetBitmap24(u32 LineY,u8 *pBM)
{
}

s32 PlugJpeg_GetWidth(void)
{
  return(0);
}

s32 PlugJpeg_GetHeight(void)
{
  return(0);
}

int PlugJpeg_GetInfoIndexCount(void)
{
  return(0);
}

bool PlugJpeg_GetInfoStrA(int idx,char *str,int len)
{
  return(false);
}

bool PlugJpeg_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugJpeg_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else // #ifdef ExceptJpeg

// ------------------------------------------------------------------------------------

void volatile CODE_IN_ITCM_JPEG LIBJPEG_ITCM_DUMMY(){} //to avoid stripping this section lol

static FAT_FILE *FileHandle;

static bool cinfoInit;
static struct jpeg_decompress_struct cinfo;

static bool ProgressiveJPEG;
static int MasterImageWidth,MasterImageHeight;
static int ScalingFactor;

static int imgWidth;
static int imgHeight;
static int imgBits;
static u8 *imgCmpBuf=NULL;

static struct jpeg_error_mgr jerr;

static u32 LastOffsetY;

// ------------------------------------------------------------------------------------

static bool JpegGetMasterImageSize(FAT_FILE *FileHandle)
{
  MemSet32CPU(0,&cinfo,sizeof(struct jpeg_decompress_struct));
  MemSet32CPU(0,&jerr,sizeof(struct jpeg_error_mgr));
  
  FAT2_fseek(FileHandle,0,SEEK_SET);
  
  // エラーのハンドリング
  cinfo.err = jpeg_std_error(&jerr);

  // 以降の jpeg ライブラリ内でエラーが生じた場合、資源を開放して終わる。

  // 構造体の初期設定
  jpeg_create_decompress(&cinfo);
  
  // ファイル入力ハンドルの設定
  jpeg_stdio_src(&cinfo, (FILE*)FileHandle);

  // ファイルの情報ヘッダの読込み
  jpeg_read_header(&cinfo, TRUE);
  
  if(cinfo.progressive_mode==TRUE){
    ProgressiveJPEG=true;
    }else{
    ProgressiveJPEG=false;
  }
  
  MasterImageWidth=cinfo.image_width;
  MasterImageHeight=cinfo.image_height;
  
  // 即解放
  jpeg_destroy_decompress(&cinfo);
  
  _consolePrint("JpegGetMasterImageSize: free.\n");
  return(true);
}

static void progress_monitor(j_common_ptr cinfo)
{
//  _consolePrintf("Called progress_monitor. %d/%d, %d/%d\n",cinfo->progress->pass_counter,cinfo->progress->pass_limit,cinfo->progress->completed_passes,cinfo->progress->total_passes);
  
  u32 pos=cinfo->progress->pass_counter;
  u32 max=cinfo->progress->pass_limit;
  
  if((pos&31)==0) CallBack_MWin_ProgressSetPos("Decode progressive jpeg...",pos,max);
}

static bool JpegStart(FAT_FILE *FileHandle)
{
  MemSet32CPU(0,&cinfo,sizeof(struct jpeg_decompress_struct));
  MemSet32CPU(0,&jerr,sizeof(struct jpeg_error_mgr));
  
  FAT2_fseek(FileHandle,0,SEEK_SET);
  
  // エラーのハンドリング
  cinfo.err = jpeg_std_error(&jerr);

  // 以降の jpeg ライブラリ内でエラーが生じた場合、資源を開放して終わる。

  // 構造体の初期設定
  jpeg_create_decompress(&cinfo);
  
  // ファイル入力ハンドルの設定
  jpeg_stdio_src(&cinfo, (FILE*)FileHandle);

  // ファイルの情報ヘッダの読込み
  jpeg_read_header(&cinfo, TRUE);
  
  cinfo.scale_num=1;
  cinfo.scale_denom=ScalingFactor;
  
  cinfo.progress=NULL;
  struct jpeg_progress_mgr progress; /* Progress monitor, or NULL if none */\
  if(ProgressiveJPEG==true){
    cinfo.progress=&progress;
    cinfo.progress->progress_monitor=progress_monitor;
    CallBack_MWin_ProgressShow("Start progressive jpeg...",0);
  }
  
  // 解凍の開始
  jpeg_start_decompress(&cinfo);
  
  if(ProgressiveJPEG==true){
    cinfo.progress=NULL;
    CallBack_MWin_ProgressHide();
  }
  
  imgWidth=cinfo.image_width/ScalingFactor;
  imgHeight=cinfo.image_height/ScalingFactor;
  
  imgBits=cinfo.output_components*8;
  
  imgCmpBuf=(u8*)safemalloc(&MM_DLLImage,cinfo.output_width*4);
  if(imgCmpBuf==NULL){
    _consolePrintf("can not allocate DecompressBuffer. out of memory. (imgCmpWidth=%d)\n",cinfo.output_width);
    ErrorDialog_Set(EEC_MemoryOverflow_CanRecovery);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    cinfoInit=false;
    return(false);
  }
  
  cinfoInit=true;
  
  _consolePrint("Jpeg decoder initialized.\n");
  
  return(true);
}

static bool JpegNextLine(u8 *dstbuf)
{
  if(cinfo.output_height<=cinfo.output_scanline){
    _consolePrintf("out of scanline. %d<=%d\n",cinfo.output_height,cinfo.output_scanline);
    return(false);
  }
  
  int res=jpeg_read_scanlines(&cinfo, &imgCmpBuf, 1);
  
  if(res!=1){
    _consolePrint("error:jpeg_read_scanlines();\n");
    return(false);
  }
  
  u8 *src=imgCmpBuf;
  
  if(imgBits==8){
    for(int x=imgWidth;x!=0;x--){
      u32 b=*src++;
      *dstbuf++=b;
      *dstbuf++=b;
      *dstbuf++=b;
    }
    }else{
    u32 size=imgWidth*3;
    u32 wordsize=size&~3;
    MemCopy32CPU(src,dstbuf,wordsize);
    if((size&3)!=0){
      src+=wordsize;
      dstbuf+=wordsize;
      size&=3;
      while(size!=0){
        *dstbuf++=*src++;
        size--;
      }
    }
  }
  
  return(true);
}

static void JpegFree(void)
{
  if(cinfoInit==true){
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    cinfoInit=false;
  }
  
  if(imgCmpBuf!=NULL){
    safefree(&MM_DLLImage,imgCmpBuf); imgCmpBuf=NULL;
  }
}

// ------------------------------------------------------------------------------------

#include "plug_jpeg_prgjpeg_dfs.h"

bool PlugJpeg_Start(FAT_FILE *_FileHandle,bool EnabledAutoFitting)
{
  cinfoInit=false;
  
  if(_FileHandle==0){
    _consolePrint("FileHandle is NULL\n");
    return(false);
  }
  
  FileHandle=_FileHandle;
  
  _consolePrint("\n");
  _consolePrint("The Independent JPEG Groups JPEG software\n");
  _consolePrint("Copyright (C) 1994-1998, Thomas G. Lane.\n");
  _consolePrint("jpeglib version 6b of 27-Mar-1998.\n");
  _consolePrint("\n");
  
  if(JpegGetMasterImageSize(FileHandle)==false){
    ErrorDialog_Set(EEC_NotSupportFileFormat);
    return(false);
  }
  
  _consolePrintf("MasterImageSize: %d,%dpixels\n",MasterImageWidth,MasterImageHeight);
  
  if(ProgressiveJPEG==true){
    _consolePrint("Start progressive JPEG mode.\n");
    FAT_FILE *pf=Shell_FAT_fopen_SwapFile_PrgJpeg(MasterImageWidth*MasterImageHeight*2*3+(64*1024)); // 64kbyteは念のため余剰
    if(pf==NULL){
      ProgressiveJPEG=false;
      _consolePrint("Not found PrgJpeg swap file.\n");
      ErrorDialog_Set(EEC_ProgressiveJpeg);
      return(false);
    }
    DFS_PrgJpeg_Init(pf);
    FAT2_fclose(pf);
  }
  
  if(EnabledAutoFitting==false){
    ScalingFactor=1;
    }else{
    u32 limw=ScreenWidth*8,limh=ScreenHeight*8;
    u32 w,h;
    ScalingFactor=1;
    w=MasterImageWidth/ScalingFactor;
    h=MasterImageHeight/ScalingFactor;
    if((limw<w)||(limh<h)){
      ScalingFactor=2;
      w=MasterImageWidth/ScalingFactor;
      h=MasterImageHeight/ScalingFactor;
      if((limw<w)||(limh<h)){
        ScalingFactor=4;
        w=MasterImageWidth/ScalingFactor;
        h=MasterImageHeight/ScalingFactor;
        if((limw<w)||(limh<h)){
          ScalingFactor=8;
        }
      }
    }
  }
  
  
  if(JpegStart(FileHandle)==false){
    ErrorDialog_Set(EEC_NotSupportFileFormat);
    return(false);
  }
  
  LastOffsetY=(u32)-1;
  
  return(true);
}

void PlugJpeg_Free(void)
{
  JpegFree();

  FileHandle=0;
  
  if(ProgressiveJPEG==true){
    DFS_PrgJpeg_Free();
    Shell_FAT_fclose_SwapFile_PrgJpeg();
  }
}

void PlugJpeg_GetBitmap24(u32 LineY,u8 *pBM)
{
  if(imgHeight<=LineY) return;
  
  if((LastOffsetY+1)!=LineY) StopFatalError(14801,"Jpeg decode: Not support seek function. (%d,%d)\n",LastOffsetY,LineY);
  LastOffsetY=LineY;
  
  if(JpegNextLine(pBM)==false) StopFatalError(14802,"Jpeg decode: Decode error.\n");
}

s32 PlugJpeg_GetWidth(void)
{
  return(imgWidth);
}

s32 PlugJpeg_GetHeight(void)
{
  return(imgHeight);
}

int PlugJpeg_GetInfoIndexCount(void)
{
  if(ScalingFactor==1){
    return(1);
    }else{
    return(2);
  }
}

bool PlugJpeg_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"%d x %d pixels.",MasterImageWidth,MasterImageHeight); return(true); break;
    case 1: snprintf(str,len,"Auto scaling %d%%.",1*100/ScalingFactor); return(true); break;
  }
  return(false);
}

bool PlugJpeg_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugJpeg_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif // #ifdef ExceptJpeg
