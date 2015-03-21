
#include <stdio.h>

#include <nds.h>

#include "_const.h"

#include "plugin.h"

#include "fat2.h"

// -----------------

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

DATA_IN_AfterSystem static TBMPHeader *pBMPHeader=NULL;

DATA_IN_AfterSystem static FAT_FILE *FileHandle;
DATA_IN_AfterSystem static u8 *pReadBuf=NULL;

// ------------------------------------------------------------------------------------

static u8 Get8bit(void)
{
  u8 res;
  FAT2_fread(&res,1,1,FileHandle);
  return(res);
}

static u16 Get16bit(void)
{
  u16 res;
  FAT2_fread(&res,2,1,FileHandle);
  return(res);
}

static u32 Get32bit(void)
{
  u32 res;
  FAT2_fread(&res,4,1,FileHandle);
  return(res);
}

static bool GetBMPHeader(TBMPHeader *pBMPHeader)
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
    _consolePrintf("Error MagicID!=BM");
    return(false);
  }
  
  if(pBMPHeader->biCopmression!=BI_RGB){
    _consolePrintf("Error notsupport Compression");
    return(false);
  }
  
  if(pBMPHeader->biHeight>=0x80000000){
    _consolePrintf("Error notsupport OS/2 format");
    return(false);
  }
  
  if(pBMPHeader->biPlanes!=1){
    _consolePrintf("Error notsupport Planes!=1");
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
      _consolePrintf("Error notsupport 16bitcolor.");
      return(false);
    case 24:
      pBMPHeader->DataWidth=pBMPHeader->biWidth*3;
      break;
    case 32:
      _consolePrintf("Error notsupport 32bitcolor.");
      return(false);
    default:
      _consolePrintf("Error Unknown xxBitColor.");
      return(false);
  }
  
  pBMPHeader->DataWidth=(pBMPHeader->DataWidth+3)&~3;
  
  return(true);
}

static void BMP_ShowFrameInfo(TBMPHeader *pBMPHeader)
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

bool BMPReader_Start(FAT_FILE *_FileHandle)
{
  if(_FileHandle==NULL){
    _consolePrintf("FileHandle is NULL\n");
    return(false);
  }
  
  FileHandle=_FileHandle;
  
  pBMPHeader=(TBMPHeader*)safemalloc_chkmem(&MM_Temp,sizeof(TBMPHeader));
  MemSet8CPU(0,pBMPHeader,sizeof(TBMPHeader));
  
  if(GetBMPHeader(pBMPHeader)==false){
    _consolePrintf("BMP LoadError.\n");
    return(false);
  }
  
  BMP_ShowFrameInfo(pBMPHeader);
  
  pReadBuf=(u8*)safemalloc_chkmem(&MM_Temp,pBMPHeader->DataWidth);
  
  return(true);
}

void BMPReader_Free(void)
{
  if(pReadBuf!=NULL){
    safefree(&MM_Temp,pReadBuf); pReadBuf=NULL;
  }
  if(pBMPHeader!=NULL){
    safefree(&MM_Temp,pBMPHeader); pBMPHeader=NULL;
  }
  
  FileHandle=0;
}

void BMPReader_GetBitmap32(u32 LineY,u32 *pBM)
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
            pBM[x+b]=col|(col<<8)|(col<<16);
          }
        }
      }
    } break;
    case 4: {
      u8 *PaletteTable=(u8*)pBMPHeader->Palette;
      for(u32 x=0;x<Width;x+=2){
        u32 data=pReadBuf[x/2];
        
        u32 pal32=*(u32*)&PaletteTable[(data>>4)*4];
        u32 r=(pal32>>16)&0xff;
        u32 g=(pal32>>8)&0xff;
        u32 b=(pal32>>0)&0xff;
        pBM[x]=b|(g<<8)|(r<<16);
        
        if((x+1)<Width){
          u32 pal32=*(u32*)&PaletteTable[(data&0x0f)*4];
          u32 r=(pal32>>16)&0xff;
          u32 g=(pal32>>8)&0xff;
          u32 b=(pal32>>0)&0xff;
          pBM[x+1]=b|(g<<8)|(r<<16);
        }
      }
    } break;
    case 8: {
      u8 *PaletteTable=(u8*)pBMPHeader->Palette;
      for(u32 x=0;x<Width;x++){
        u32 pal32=*(u32*)&PaletteTable[pReadBuf[x]*4];
        u32 r=(pal32>>16)&0xff;
        u32 g=(pal32>>8)&0xff;
        u32 b=(pal32>>0)&0xff;
        pBM[x]=b|(g<<8)|(r<<16);
      }
    } break;
    case 24: {
      for(u32 x=0;x<Width;x++){
        u32 r=pReadBuf[x*3+2];
        u32 g=pReadBuf[x*3+1];
        u32 b=pReadBuf[x*3+0];
        pBM[x]=b|(g<<8)|(r<<16);
      }
    } break;
  }
}

s32 BMPReader_GetWidth(void)
{
  return(pBMPHeader->biWidth);
}

s32 BMPReader_GetHeight(void)
{
  return(pBMPHeader->biHeight);
}

