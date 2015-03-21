
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

#ifndef ExceptPsd

bool PlugPsd_Start(FAT_FILE *_FileHandle)
{
  return(false);
}

void PlugPsd_Free(void)
{
}

void PlugPsd_GetBitmap24(u32 LineY,u8 *pBM)
{
}

s32 PlugPsd_GetWidth(void)
{
  return(0);
}

s32 PlugPsd_GetHeight(void)
{
  return(0);
}

int PlugPsd_GetInfoIndexCount(void)
{
  return(0);
}

bool PlugPsd_GetInfoStrA(int idx,char *str,int len)
{
  return(false);
}

bool PlugPsd_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugPsd_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#else // #ifdef ExceptPsd

// ------------------------------------------------------------------------------------

enum EPSDMODE {EPM_Bitmap=0,EPM_Grayscale=1,EPM_Indexed=2,EPM_RGB=3,EPM_CMYK=4,EPM_Unknown5=5,EPM_Unknown6=6,EPM_Multichannel=7,EPM_Duotone=8,EPM_Lab=9};

#define PSDCompType_Deflate (0)
#define PSDCompType_RLE (1)

typedef struct {
  char Signature[4];
  u16 Version;
  u16 Channels;
  u32 Height;
  u32 Width;
  u16 BitPerChannel;
  EPSDMODE Mode;
  u32 Palette[256];
  u16 Compression;
  u32 ImageDataOffset;
  u32 DstSize;
} TPSDHead;

typedef struct {
  u32 TableCount;
  u32 *pOffsetTable;
  u32 *pSizeTable;
  u32 DataOffset;
  u32 SizeMax;
} TPSDRLE;

static TPSDHead PSDHead;
static TPSDRLE PSDRLE;

static FAT_FILE *FileHandle;

u8 *pSrcBuf,*pDstBuf[4];

// ------------------------------------------------------------------------------------

static void rfsSetPos(u32 ofs)
{
  FAT2_fseek(FileHandle,ofs,SEEK_SET);
}

static u32 rfsGetPos(void)
{
  return((u32)FAT2_ftell(FileHandle));
}

static void rfsskip(u32 size)
{
  u32 pos;
  
  pos=rfsGetPos()+size;
  
  rfsSetPos(pos);
}

static u8 rfs8bit(void)
{
  u8 res;
  
  FAT2_fread(&res,1,1,FileHandle);
  
  return(res);
}

static u16 rfs16bit(void)
{
  u16 res=0;
  
  res|=rfs8bit() << 8;
  res|=rfs8bit() << 0;
  
  return(res);
}

static u32 rfs32bit(void)
{
  u32 res=0;
  
  res|=rfs8bit() << 24;
  res|=rfs8bit() << 16;
  res|=rfs8bit() << 8;
  res|=rfs8bit() << 0;
  
  return(res);
}

static bool GetPSDHead(TPSDHead *pPH)
{
    FAT2_fseek(FileHandle,0,SEEK_SET);
  
  for(int idx=0;idx<4;idx++){
    pPH->Signature[idx]=(char)(rfs8bit());
  }
  
  pPH->Version=rfs16bit();
  rfsskip(6); // reserved
  pPH->Channels=rfs16bit();
  pPH->Height=rfs32bit();
  pPH->Width=rfs32bit();
  pPH->BitPerChannel=rfs16bit();
  pPH->Mode=(EPSDMODE)rfs16bit();
  
  if((pPH->Signature[0]!='8')||(pPH->Signature[1]!='B')||(pPH->Signature[2]!='P')||(pPH->Signature[3]!='S')){
    _consolePrint("Signature != '8BPS' error.\n");
    return(false);
  }
  
  if((pPH->Channels==0)||(pPH->Height==0)||(pPH->Width==0)||(pPH->BitPerChannel==0)){
    _consolePrint("Channels or height or width or bpc is zero.\n");
    return(false);
  }
  
  if(((u32)EPM_Lab<(u32)pPH->Mode)||(pPH->Mode==EPM_Unknown5)||(pPH->Mode==EPM_Unknown6)||(pPH->Mode==EPM_Lab)){
    _consolePrint("Unknown or unsupport mode.\n");
    return(false);
  }
  
  switch(pPH->Mode){
    case EPM_Bitmap: {
    } break;
    case EPM_Grayscale: case EPM_Duotone: {
    } break;
    case EPM_Indexed: {
    } break;
    case EPM_RGB: case EPM_Multichannel: {
      if(pPH->Channels<3){
        _consolePrint("Request 3channels for RGB or Multichannel.\n");
        return(false);
      }
    } break;
    case EPM_CMYK: {
      if(pPH->Channels<4){
        _consolePrint("Request 4channels for CMYK.\n");
        return(false);
      }
    } break;
    case EPM_Unknown5: case EPM_Unknown6: case EPM_Lab: {
    } break;
  }
  
  // Mode Data
  {
    u32 len=rfs32bit();
    u32 *pPalette=pPH->Palette;
    
    for(int idx=0;idx<256;idx++){
      pPalette[idx]=0;
    }
    
    if(len!=768){
      rfsskip(len);
      }else{
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 16; // R
      }
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 8; // G
      }
      for(int idx=0;idx<256;idx++){
        pPalette[idx]|=rfs8bit() << 0; // B
      }
    }
  }
  
  // Image Resources
  {
    u32 len=rfs32bit();
    rfsskip(len);
  }
  
  // Reserved Data
  {
    u32 len=rfs32bit();
    rfsskip(len);
  }
  
  pPH->Compression=rfs16bit();
  
  if((pPH->Compression!=PSDCompType_Deflate)&&(pPH->Compression!=PSDCompType_RLE)){
    _consolePrintf("Not support compression type(%d).\n",pPH->Compression);
    return(false);
  }
  
  pPH->ImageDataOffset=rfsGetPos();
  
  if(pPH->Mode==EPM_Bitmap){
    pPH->DstSize=(pPH->Width+7)/8;
    }else{
    pPH->DstSize=pPH->Width;
  }
  
  return(true);
}

static bool GetPSDRLE(u32 ImageDataOffset,EPSDMODE Mode,u32 Width,u32 Height,u32 Channels,TPSDRLE *pPR)
{
  pPR->TableCount=Height*Channels;
  
  pPR->pSizeTable=(u32*)safemalloc(&MM_DLLImage,pPR->TableCount*4);
  
  rfsSetPos(ImageDataOffset);
  for(u32 idx=0;idx<pPR->TableCount;idx++){
    pPR->pSizeTable[idx]=(u32)rfs16bit();
  }
  
  pPR->DataOffset=rfsGetPos();
  
  pPR->pOffsetTable=(u32*)safemalloc(&MM_DLLImage,pPR->TableCount*4);
  
  pPR->pOffsetTable[0]=pPR->DataOffset;
  for(u32 idx=1;idx<pPR->TableCount;idx++){
    pPR->pOffsetTable[idx]=pPR->pOffsetTable[idx-1]+pPR->pSizeTable[idx-1];
  }
  
  pPR->SizeMax=0;
  for(u32 idx=1;idx<pPR->TableCount;idx++){
    if(pPR->SizeMax<pPR->pSizeTable[idx]) pPR->SizeMax=pPR->pSizeTable[idx];
  }
  
  return(true);
}

void PSD_ShowFrameInfo(TPSDHead *pPH)
{
  _consolePrintf("Signature=%c%c%c%c\n",pPH->Signature[0],pPH->Signature[1],pPH->Signature[2],pPH->Signature[3]);
  _consolePrintf("Version=$%0.4x\n",pPH->Version);
  _consolePrintf("Channels=%d\n",pPH->Channels);
  _consolePrintf("Height=%d\n",pPH->Height);
  _consolePrintf("Width=%d\n",pPH->Width);
  _consolePrintf("BitPerChannel=%d\n",pPH->BitPerChannel);
  _consolePrintf("Mode=%d\n",pPH->Mode);
  _consolePrintf("Compression=%d\n",pPH->Compression);
  _consolePrintf("ImageDataOffset=$%0.8x\n",pPH->ImageDataOffset);
}

void DeflatePSDRLE(u8 *pSrcBuf,u8 *pDstBuf,u32 DstBufSize)
{
  u32 idx=0;
  
  while(idx<DstBufSize){
    u32 data=*pSrcBuf++;
    if(data<128){
      u32 len=data+1;
      for(u32 cnt=0;cnt<len;cnt++){
        *pDstBuf++=*pSrcBuf++;
        idx++;
      }
      }else{
      u32 len=256-data+1;
      u8 data=*pSrcBuf++;
      for(u32 cnt=0;cnt<len;cnt++){
        *pDstBuf++=data;
        idx++;
      }
    }
  }
}

void PSD_GetBMBuf(u32 LineY,u8 *pBM)
{
  TPSDHead *pPH=&PSDHead;
  
  u8 *pSrcBM=pBM;
  
  u32 ChannelsCount;
  
  if(pPH->Channels<=4){
    ChannelsCount=pPH->Channels;
    }else{
    ChannelsCount=4;
  }
  
  TPSDRLE *pPR=&PSDRLE;
  
  switch(pPH->Compression){
    case PSDCompType_Deflate: {
      for(u32 idx=0;idx<ChannelsCount;idx++){
        rfsSetPos(pPH->ImageDataOffset+(((pPH->Height*idx)+LineY)*pPH->DstSize));
        FAT2_fread(pDstBuf[idx],1,pPH->DstSize,FileHandle);
      }
    } break;
    case PSDCompType_RLE: {
      for(u32 idx=0;idx<ChannelsCount;idx++){
        rfsSetPos(pPR->pOffsetTable[(pPH->Height*idx)+LineY]);
        FAT2_fread(pSrcBuf,1,pPR->pSizeTable[(pPH->Height*idx)+LineY],FileHandle);
        DeflatePSDRLE(pSrcBuf,pDstBuf[idx],pPH->DstSize);
      }
    } break;
  }
  
  u32 Width=pPH->Width;
  
  switch(pPH->Mode){
    case EPM_Bitmap: {
      u8 *pBufR=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 r=*pBufR;
        if((x&7)==7) pBufR++;
        
        r=(r >> (7-(x&7))) & 1;
        
        if(r==0){
          *pSrcBM++=0xff;
          *pSrcBM++=0xff;
          *pSrcBM++=0xff;
          }else{
          *pSrcBM++=0x00;
          *pSrcBM++=0x00;
          *pSrcBM++=0x00;
        }
      }
    } break;
    case EPM_Grayscale: case EPM_Duotone: {
      u8 *pBufR=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 r=*pBufR++;
        *pSrcBM++=r;
        *pSrcBM++=r;
        *pSrcBM++=r;
      }
    } break;
    case EPM_Indexed: {
      u32 *pPalette=pPH->Palette;
      u8 *pBufP=pDstBuf[0];
      for(u32 x=0;x<Width;x++){
        u32 col=pPalette[*pBufP++];
        *pSrcBM++=(col >> 16) & 0xff;
        *pSrcBM++=(col >> 8) & 0xff;
        *pSrcBM++=(col >> 0) & 0xff;
      }
    } break;
    case EPM_RGB: case EPM_Multichannel: {
      u8 *pBufR=pDstBuf[0];
      u8 *pBufG=pDstBuf[1];
      u8 *pBufB=pDstBuf[2];
      for(u32 x=0;x<Width;x++){
        *pSrcBM++=*pBufR++;
        *pSrcBM++=*pBufG++;
        *pSrcBM++=*pBufB++;
      }
    } break;
    case EPM_CMYK: {
      u8 *pBufC=pDstBuf[0];
      u8 *pBufM=pDstBuf[1];
      u8 *pBufY=pDstBuf[2];
      u8 *pBufK=pDstBuf[3];
      for(u32 x=0;x<Width;x++){
        u32 r,g,b,k;
        
        r=*pBufC++;
        g=*pBufM++;
        b=*pBufY++;
        k=*pBufK++;
        k=0xff-k;
        
        *pSrcBM++=(r<k) ? 0 : (r-k);
        *pSrcBM++=(g<k) ? 0 : (g-k);
        *pSrcBM++=(b<k) ? 0 : (b-k);
      }
    } break;
    case EPM_Unknown5: case EPM_Unknown6: case EPM_Lab: {
    } break;
  }
}

#undef Gravity

// ------------------------------------------------------------------------------------
void PlugPsd_Free(void);

bool PlugPsd_Start(FAT_FILE *_FileHandle)
{
    if(_FileHandle==0){
        _consolePrint("FileHandle is NULL\n");
        return(false);
      }
      
      FileHandle=_FileHandle;
      
      MemSet32CPU(0,&PSDHead,sizeof(TPSDHead));
      
      PSDRLE.pOffsetTable=NULL;
      PSDRLE.pSizeTable=NULL;
      
      pSrcBuf=NULL;
      for(int idx=0;idx<4;idx++){
        pDstBuf[idx]=NULL;
      }
      
      TPSDHead *pPH=&PSDHead;
      TPSDRLE *pPR=&PSDRLE;
      
      if(GetPSDHead(&PSDHead)==false){
        _consolePrint("PSD LoadError.\n");
        PlugPsd_Free();
        return(false);
      }
      
      if(PSDHead.Compression==PSDCompType_RLE){
        if(GetPSDRLE(pPH->ImageDataOffset,pPH->Mode,pPH->Width,pPH->Height,pPH->Channels,&PSDRLE)==false){
          _consolePrint("PSD RLE LoadError.\n");
          PlugPsd_Free();
          return(false);
        }
      }
      
      PSD_ShowFrameInfo(&PSDHead);
      
      pSrcBuf=(u8*)safemalloc(&MM_DLLImage,pPR->SizeMax);
      for(int idx=0;idx<4;idx++){
        pDstBuf[idx]=(u8*)safemalloc(&MM_DLLImage,pPH->DstSize);
      }
      
      return(true);
}

void PlugPsd_Free(void)
{
    TPSDRLE *pPR=&PSDRLE;
    
    if(pPR->pSizeTable!=NULL){
      safefree(&MM_DLLImage,pPR->pSizeTable); pPR->pSizeTable=NULL;
    }
    if(pPR->pOffsetTable!=NULL){
      safefree(&MM_DLLImage,pPR->pOffsetTable); pPR->pOffsetTable=NULL;
    }
    
    if(pSrcBuf!=NULL){
      safefree(&MM_DLLImage,pSrcBuf); pSrcBuf=NULL;
    }
    
    for(int idx=0;idx<4;idx++){
      if(pDstBuf[idx]!=NULL){
        safefree(&MM_DLLImage,pDstBuf[idx]); pDstBuf[idx]=NULL;
      }
    }
    
    FileHandle=0;
}

void PlugPsd_GetBitmap24(u32 LineY,u8 *pBM)
{
    if(PSDHead.Height<=LineY) return;
    PSD_GetBMBuf(LineY,pBM);  
}

s32 PlugPsd_GetWidth(void)
{
    return(PSDHead.Width);
}

s32 PlugPsd_GetHeight(void)
{
    return(PSDHead.Height);
}

int PlugPsd_GetInfoIndexCount(void)
{
    return(4);
}

char PSDMODE_NAME[][16]={"Bitmap","Grayscale","Indexed","RGB","CMYK","Unknown5","Unknown6","Multichannel","Duotone","Lab"};

bool PlugPsd_GetInfoStrL(int idx,char *str,int len)
{
    TPSDHead *pPH=&PSDHead;
      
    switch(idx){
        case 0: snprintf(str,len,"Pixels=%dx%d",pPH->Width,pPH->Height); return(true); break;
        case 1: snprintf(str,len,"%dchannels %dbit/channel",pPH->Channels,pPH->BitPerChannel); return(true); break;
        case 2: snprintf(str,len,"Copmression=%d",pPH->Compression); return(true); break;
        case 3: snprintf(str,len,"Mode=%d(%s)",pPH->Mode,PSDMODE_NAME[pPH->Mode]); return(true); break;
    }
    return(false);
}

bool PlugPsd_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugPsd_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}

#endif // #ifdef ExceptPsd
