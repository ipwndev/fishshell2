
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

#ifndef ExceptBmp

bool PlugBmp_Start(FAT_FILE *_FileHandle)
{
  return(false);
}

void PlugBmp_Free(void)
{
}

void PlugBmp_GetBitmap24(u32 LineY,u8 *pBM)
{
}

s32 PlugBmp_GetWidth(void)
{
  return(0);
}

s32 PlugBmp_GetHeight(void)
{
  return(0);
}

int PlugBmp_GetInfoIndexCount(void)
{
  return(0);
}

bool PlugBmp_GetInfoStrA(int idx,char *str,int len)
{
  return(false);
}

bool PlugBmp_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugBmp_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else // #ifdef ExceptBmp

// ------------------------------------------------------------------------------------

#define BI_RGB (0)
#define BI_RLE8 (1)
#define BI_RLE4 (2)
#define BI_Bitfields (3)

typedef struct {
  u8 bfType[2];
  u32 bfSize;
  u16 bfReserved1;
  u16 bfReserved2;
  u32 bfOffset;
  u32 biSize;
  u32 biWidth;
  u32 biHeight;
  u16 biPlanes;
  u16 biBitCount;
  u32 biCopmression;
  u32 biSizeImage;
  u32 biXPixPerMeter;
  u32 biYPixPerMeter;
  u32 biClrUsed;
  u32 biCirImportant;
  
  u32 Palette[256];
  
  u32 DataWidth;
} TBMPHeader;

static TBMPHeader *pBMPHeader=NULL;

static FAT_FILE *FileHandle;

static u8 *pReadBuf=NULL;

// ------------------------------------------------------------------------------------

static CODE_IN_ITCM_BMP u8 Get8bit(void)
{
  u8 res;
  FAT2_fread(&res,1,1,FileHandle);
  return(res);
}

static CODE_IN_ITCM_BMP u16 Get16bit(void)
{
  u16 res;
  FAT2_fread(&res,2,1,FileHandle);
  return(res);
}

static CODE_IN_ITCM_BMP u32 Get32bit(void)
{
  u32 res;
  FAT2_fread(&res,4,1,FileHandle);
  return(res);
}

static CODE_IN_ITCM_BMP __attribute__ ((noinline))  bool GetBMPHeader(TBMPHeader *pBMPHeader)
{
  FAT2_fseek(FileHandle,0,SEEK_SET);
  
  pBMPHeader->bfType[0]=Get8bit();
  pBMPHeader->bfType[1]=Get8bit();
  pBMPHeader->bfSize=Get32bit();
  pBMPHeader->bfReserved1=Get16bit();
  pBMPHeader->bfReserved2=Get16bit();
  pBMPHeader->bfOffset=Get32bit();
  pBMPHeader->biSize=Get32bit();
  pBMPHeader->biWidth=Get32bit();
  pBMPHeader->biHeight=Get32bit();
  pBMPHeader->biPlanes=Get16bit();
  pBMPHeader->biBitCount=Get16bit();
  pBMPHeader->biCopmression=Get32bit();
  pBMPHeader->biSizeImage=Get32bit();
  pBMPHeader->biXPixPerMeter=Get32bit();
  pBMPHeader->biYPixPerMeter=Get32bit();
  pBMPHeader->biClrUsed=Get32bit();
  pBMPHeader->biCirImportant=Get32bit();
  
  if((pBMPHeader->biBitCount==4)||(pBMPHeader->biBitCount==8)){
    for(int idx=0;idx<256;idx++){
      pBMPHeader->Palette[idx]=Get32bit();
    }
    }else{
    for(int idx=0;idx<256;idx++){
      pBMPHeader->Palette[idx]=0;
    }
  }
  
  if((pBMPHeader->bfType[0]!='B')||(pBMPHeader->bfType[1]!='M')){
    _consolePrint("Error MagicID!=BM");
    return(false);
  }
  
  if(pBMPHeader->biCopmression!=BI_RGB){
    _consolePrint("Error notsupport Compression");
    return(false);
  }
  
  if(pBMPHeader->biHeight>=0x80000000){
    _consolePrint("Error notsupport OS/2 format");
    return(false);
  }
  
  if(pBMPHeader->biPlanes!=1){
    _consolePrint("Error notsupport Planes!=1");
    return(false);
  }
  
  pBMPHeader->DataWidth=0;
  
  switch(pBMPHeader->biBitCount){
    case 1:
      pBMPHeader->DataWidth=(pBMPHeader->biWidth+7)/8;
      break;
    case 4:
      pBMPHeader->DataWidth=(pBMPHeader->biWidth+1)/2;
      break;
    case 8:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*1;
      break;
    case 16:
      _consolePrint("Error notsupport 16bitcolor.");
      return(false);
    case 24:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*3;
      break;
    case 32:
      _consolePrint("Error notsupport 32bitcolor.");
      return(false);
    default:
      _consolePrint("Error Unknown xxBitColor.");
      return(false);
  }
  
  pBMPHeader->DataWidth=(pBMPHeader->DataWidth+3)&~3;
  
  return(true);
}

void CODE_IN_ITCM_BMP __attribute__ ((noinline)) BMP_ShowFrameInfo(TBMPHeader *pBMPHeader)
{
  _consolePrintf("FileSize=%d\n",pBMPHeader->bfSize);
  _consolePrintf("Size=(%d,%d) %dpixel\n",pBMPHeader->biWidth,pBMPHeader->biHeight,pBMPHeader->biWidth*pBMPHeader->biHeight);
  _consolePrintf("Planes=%d\n",pBMPHeader->biPlanes);
  _consolePrintf("BitCount=%d\n",pBMPHeader->biBitCount);
  _consolePrintf("Compression=%d\n",pBMPHeader->biCopmression);
  _consolePrintf("BitmapOffset=0x%x\n",pBMPHeader->bfOffset);
  _consolePrintf("DataWidth=%d\n",pBMPHeader->DataWidth);
}

// ------------------------------------------------------------------------------------

bool PlugBmp_Start(FAT_FILE *_FileHandle)
{
    if(_FileHandle==0){
        _consolePrint("FileHandle is NULL\n");
        return(false);
    }
      
    FileHandle=_FileHandle;
      
    pBMPHeader=(TBMPHeader*)safemalloc_chkmem(&MM_DLLImage,sizeof(TBMPHeader));
    if(pBMPHeader==NULL){
        _consolePrint("pBMPHeader malloc error.\n");
        return(false);
    }
      
    MemSet32CPU(0,pBMPHeader,sizeof(TBMPHeader));
      
    if(GetBMPHeader(pBMPHeader)==false){
        _consolePrint("BMP LoadError.\n");
        return(false);
    }
      
    BMP_ShowFrameInfo(pBMPHeader);
      
    pReadBuf=(u8*)safemalloc_chkmem(&MM_DLLImage,pBMPHeader->DataWidth);
      
    return(true);
}

void PlugBmp_Free(void)
{
    if(pReadBuf!=NULL){
        safefree(&MM_DLLImage,pReadBuf); pReadBuf=NULL;
    }
    if(pBMPHeader!=NULL){
        safefree(&MM_DLLImage,pBMPHeader); pBMPHeader=NULL;
    }
      
    FileHandle=0;
}

void PlugBmp_GetBitmap24(u32 LineY,u8 *pBM)
{
  if(pBMPHeader->biHeight<=LineY) return;
  
  {
    u32 ofs=pBMPHeader->bfOffset+((pBMPHeader->biHeight-LineY-1)*pBMPHeader->DataWidth);
    if(FAT2_ftell(FileHandle)!=ofs) FAT2_fseek(FileHandle,ofs,SEEK_SET);
  }
  
  FAT2_fread(pReadBuf,1,pBMPHeader->DataWidth,FileHandle);
  
  u32 Width=pBMPHeader->biWidth;
  
  switch(pBMPHeader->biBitCount){
    case 1: {
      for(u32 x=0;x<Width;x+=8){
        u32 data=pReadBuf[x/8];
        for(u32 b=0;b<8;b++){
          if((x+b)<Width){
            u32 bit=(data>>(7-b))&1;
            u32 col=(bit==1) ? 0 : 0xff;
            pBM[((x+b)*3)+0]=col;
            pBM[((x+b)*3)+1]=col;
            pBM[((x+b)*3)+2]=col;
          }
        }
      }
    } break;
    case 4: {
      u8 *PaletteTable=(u8*)pBMPHeader->Palette;
      for(u32 x=0;x<Width;x+=2){
        u32 data=pReadBuf[x/2];
        
        u32 pal32=*(u32*)&PaletteTable[(data>>4)*4];
        pBM[(x*3)+0]=(pal32>>16)&0xff;
        pBM[(x*3)+1]=(pal32>>8)&0xff;
        pBM[(x*3)+2]=(pal32>>0)&0xff;
        
        if((x+1)<Width){
          u32 pal32=*(u32*)&PaletteTable[(data&0x0f)*4];
          pBM[(x*3)+3]=(pal32>>16)&0xff;
          pBM[(x*3)+4]=(pal32>>8)&0xff;
          pBM[(x*3)+5]=(pal32>>0)&0xff;
        }
      }
    } break;
    case 8: {
      u8 *PaletteTable=(u8*)pBMPHeader->Palette;
      for(u32 x=0;x<Width;x++){
        u32 pal32=*(u32*)&PaletteTable[pReadBuf[x]*4];
        pBM[(x*3)+0]=(pal32>>16)&0xff;
        pBM[(x*3)+1]=(pal32>>8)&0xff;
        pBM[(x*3)+2]=(pal32>>0)&0xff;
      }
    } break;
    case 24: {
      for(u32 x=0;x<Width;x++){
        pBM[(x*3)+0]=pReadBuf[x*3+2];
        pBM[(x*3)+1]=pReadBuf[x*3+1];
        pBM[(x*3)+2]=pReadBuf[x*3+0];
      }
    } break;
  }
}

s32 PlugBmp_GetWidth(void)
{
    return(pBMPHeader->biWidth);
}

s32 PlugBmp_GetHeight(void)
{
    return(pBMPHeader->biHeight);
}

int PlugBmp_GetInfoIndexCount(void)
{
    return(4);
}

bool PlugBmp_GetInfoStrL(int idx,char *str,int len)
{
    switch(idx){
        case 0: snprintf(str,len,"Pixels=%dx%dx%dbitColor",pBMPHeader->biWidth,pBMPHeader->biHeight,pBMPHeader->biBitCount); return(true); 
        case 1: snprintf(str,len,"Planes=%d",pBMPHeader->biPlanes); return(true);
        case 2: snprintf(str,len,"Copmression=%d",pBMPHeader->biCopmression); return(true); 
        case 3: snprintf(str,len,"PixelsPerMeter=%dx%d",pBMPHeader->biXPixPerMeter,pBMPHeader->biYPixPerMeter); return(true);
    }
    return(false);
}

bool PlugBmp_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugBmp_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif // #ifdef ExceptBmp
