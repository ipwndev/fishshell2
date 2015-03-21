
#include <stdio.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"

#include "memtool.h"

#include "internaldrivers.h"

#include "ErrorDialog.h"

#include "fat2.h"
#include "shell.h"

#include "libgif/gif_lib.h"

#ifndef ExceptGif

bool PlugGif_Start(FAT_FILE *_FileHandle)
{
  return(false);
}

void PlugGif_Free(void)
{
}

void PlugGif_GetBitmap24(u32 LineY,u8 *pBM)
{
}

s32 PlugGif_GetWidth(void)
{
  return(0);
}

s32 PlugGif_GetHeight(void)
{
  return(0);
}

int PlugGif_GetInfoIndexCount(void)
{
  return(0);
}

bool PlugGif_GetInfoStrA(int idx,char *str,int len)
{
  return(false);
}

bool PlugGif_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugGif_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else // #ifdef ExceptGif

// ------------------------------------------------------------------------------------

typedef struct {
  u32 Palettes[256];
  u32 PalettesCount;
  u32 Width,Height;
} TGIFInfo;

TGIFInfo GIFInfo;
u8 *pBitmap=NULL;
u32 BitmapWidth=0;

FAT_FILE *readFunc_FileHandle;

static void _PrintGifError(void)
{
  _consolePrintf("GIF error:%s\n",GifLastError());
}

static void _PrintGifErrorStr(char *str)
{
  _consolePrintf("GIF error:%s\n",str);
}

static int readFunc(GifFileType* GifFile, GifByteType* buf, int count)
{
  return(FAT2_fread(buf,1,count,readFunc_FileHandle));
}

#define GAMMA(x)    (x)

bool LoadGIF(FAT_FILE *FileHandle)
{
  readFunc_FileHandle=FileHandle;
  
  if(FileHandle==0){
    _PrintGifErrorStr("SourceData Null.");
    return(false);
  }
  
  {
    GifFileType *GifFile;
    
    _consolePrint("OpenFile.\n");
    if ((GifFile = DGifOpen((void*)FileHandle, readFunc)) == NULL) {
      _PrintGifError();
      return false;
    }
    
    /* Scan the content of the GIF file and load the image(s) in: */
    GifRecordType RecordType;
    
    do {
      _consolePrint("DGifGetRecordType\n");
      if (DGifGetRecordType(GifFile, &RecordType) == GIF_ERROR) {
        _PrintGifError();
        return false;
      }
      
      switch (RecordType) {
        case IMAGE_DESC_RECORD_TYPE: {
          _consolePrint("IMAGE_DESC_RECORD_TYPE\n");
          
          _consolePrint("DGifGetImageDesc\n");
          if (DGifGetImageDesc(GifFile) == GIF_ERROR) {
            _PrintGifError();
            return false;
          }
          
          s32 Width=GifFile->Image.Width;
          s32 Height=GifFile->Image.Height;
          u32 PalettesCount;
          
/*
_consolePrintf("p:GifFile=%x\n",(u32)GifFile);
_consolePrintf("p:&Image.ColorMap=%x\n",&GifFile->Image.ColorMap);
_consolePrintf("p:&SColorMap=%x\n",&GifFile->SColorMap);
*/

          _consolePrint("Update Color map\n");
          
          _consolePrintf("GifFile->Image.ColorMap = %x\n",(int)GifFile->Image.ColorMap);
          _consolePrintf("GifFile->SColorMap = %x\n",(int)GifFile->SColorMap);
          
          ColorMapObject *ColorMap = (GifFile->Image.ColorMap ? GifFile->Image.ColorMap : GifFile->SColorMap);
          
          PalettesCount=ColorMap->ColorCount;
          
          for(u32 idx=0;idx<256;idx++){
            GIFInfo.Palettes[idx]=0 | BIT15;
          }
          
          for(u32 idx=0;idx<PalettesCount;idx++){
            GifColorType* pColor = &ColorMap->Colors[idx];
            
            u32 pal=0;
            
            pal|=(GAMMA(pColor->Blue) & 0xff)<< 0;
            pal|=(GAMMA(pColor->Green) & 0xff) << 8;
            pal|=(GAMMA(pColor->Red) & 0xff) << 16;
            
            GIFInfo.Palettes[idx]=pal;
          }
          
          BitmapWidth=(Width+4+4) & ~3;
          
          _consolePrintf("malloc(%d);\n",BitmapWidth*Height);
          
          if(pBitmap!=NULL) safefree(&MM_DLLImage,pBitmap);
          pBitmap=(u8*)safemalloc(&MM_DLLImage,BitmapWidth*Height);
          if(pBitmap==NULL){
            _PrintGifErrorStr("out of memory.\n");
            return(false);
          }
          MemSet8CPU(GifFile->SBackGroundColor,pBitmap,BitmapWidth*Height);
          
          _consolePrint("GetImage.\n");
          
          if (GifFile->Image.Interlace) {
            /* Need to perform 4 passes on the images: */
            const short InterlacedOffset[] = { 0, 4, 2, 1 }; /* The way Interlaced image should. */
            const short InterlacedJumps[] = { 8, 8, 4, 2 };    /* be read - offsets and jumps... */
            for (u32 InterlaceCount=0; InterlaceCount<4; InterlaceCount++){
              u32 prgdiv=Height/16;
              CallBack_MWin_ProgressShow("Decode interlace image.",Height);
              for (s32 y = InterlacedOffset[InterlaceCount]; y < Height; y += InterlacedJumps[InterlaceCount]) {
                if(prgdiv!=0){
                  prgdiv--;
                  }else{
                  prgdiv=Height/16;
                  CallBack_MWin_ProgressSetPos("Decode image.",y,Height);
                }
                if (DGifGetLine(GifFile, &pBitmap[(Height-1-y)*BitmapWidth], Width) == GIF_ERROR) {
                  _PrintGifError();
                  return false;
                }
              }
              CallBack_MWin_ProgressHide();
            }
            }else{
            u32 prgdiv=Height/16;
            CallBack_MWin_ProgressShow("Decode image.",Height);
            for (s32 y = 0; y < Height; y++) {
              if(prgdiv!=0){
                prgdiv--;
                }else{
                prgdiv=Height/16;
                CallBack_MWin_ProgressSetPos("Decode image.",y,Height);
              }
              if (DGifGetLine(GifFile, &pBitmap[(Height-1-y)*BitmapWidth], Width) == GIF_ERROR) {
                _PrintGifError();
                return false;
              }
            }
            CallBack_MWin_ProgressHide();
          }
          
          _consolePrint("GIFInfo Update.\n");
          
          GIFInfo.Width=Width;
          GIFInfo.Height=Height;
          GIFInfo.PalettesCount=PalettesCount;
          break;
		}
        case EXTENSION_RECORD_TYPE: {
          _consolePrint("EXTENSION_RECORD_TYPE\n");
          /* Skip any extension blocks in file: */
          GifByteType *Extension;
          int ExtCode;
          if (DGifGetExtension(GifFile, &ExtCode, &Extension) == GIF_ERROR) {
            _PrintGifError();
            return false;
          }
          while (Extension != NULL) {
            if (DGifGetExtensionNext(GifFile, &Extension) == GIF_ERROR) {
              _PrintGifError();
              return false;
            }
          }
          break;
		}
        case TERMINATE_RECORD_TYPE:
          _consolePrint("TERMINATE_RECORD_TYPE\n");
          break;
        default:      /* Should be traps by DGifGetRecordType. */
          _consolePrint("unknown RECORD_TYPE\n");
          break;
      }
    }
    while ((RecordType != TERMINATE_RECORD_TYPE)&&(pBitmap==NULL));
    
    _consolePrint("CloseFile.\n");
    /* Close file when done */
    if (DGifCloseFile(GifFile) == GIF_ERROR) {
      _PrintGifError();
      return false;
    }
  }
  
  if(pBitmap==NULL){
    return(false);
    }else{
    return(true);
  }
}
// ------------------------------------------------------------------------------------

bool PlugGif_Start(FAT_FILE *_FileHandle)
{
    if(_FileHandle==0){
        _consolePrint("FileHandle is NULL\n");
        return(false);
    }
      
    if(LoadGIF(_FileHandle)==false){
        _consolePrint("GIF LoadError.\n");
        if(pBitmap!=NULL) safefree(&MM_DLLImage,pBitmap);
        return(false);
    }
      
    _consolePrintf("pBitmap=0x%08x\n",(u32)pBitmap);
    _consolePrintf("PalettesCount=%d\n",GIFInfo.PalettesCount);
    _consolePrintf("Size=(%d,%d) %dpixel\n",GIFInfo.Width,GIFInfo.Height,GIFInfo.Width*GIFInfo.Height);
      
    return(true);
}

void PlugGif_Free(void)
{
    if(pBitmap!=NULL){
        safefree(&MM_DLLImage,pBitmap); pBitmap=NULL;
    }
}

void PlugGif_GetBitmap24(u32 LineY,u8 *pBM)
{
    u32 *PaletteTable=GIFInfo.Palettes;
      
    u8 *pSrcBM=&pBitmap[(GIFInfo.Height-LineY-1)*BitmapWidth];
      
    for(s32 x=0;x<GIFInfo.Width;x++){
        u32 pal=PaletteTable[pSrcBM[x]];
        
        pBM[0]=pal >> 16;
        pBM[1]=pal >> 8;
        pBM[2]=pal >> 0;
        pBM+=3;
    }
}

s32 PlugGif_GetWidth(void)
{
    return(GIFInfo.Width);
}

s32 PlugGif_GetHeight(void)
{
    return(GIFInfo.Height);
}

int PlugGif_GetInfoIndexCount(void)
{
    return(0);
}

bool PlugGif_GetInfoStrL(int idx,char *str,int len)
{
    return(false);
}

bool PlugGif_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
    return(false);
}

bool PlugGif_GetInfoStrUTF8(int idx,char *str,int len)
{
    return(false);
}

#endif // #ifdef ExceptGif
