
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "_const.h"

#include "cipk_simple.h"

#include "strtool.h"
#include "memtool.h"

#include "zlibhelp.h"

CIPK::CIPK(FAT_FILE *pf)
{
  prfs=new CStreamFS(pf);
  
  pBodyOffsets=NULL;
  FilesCount=0;
  OffsetEXT0=0;
  
  char ID[IPKID_Size];
  prfs->ReadBuffer(&ID[0],IPKID_Size);
  if(strcmp(ID,IPKID_Data)!=0) return;
  
  prfs->SetOffset(prfs->GetOffset()+IPKStateVersion_Size);
  prfs->SetOffset(prfs->GetOffset()+IPKStateBulk_Size);
  
  FilesCount=prfs->Readu32();
  pBodyOffsets=(u32*)safemalloc_chkmem(&MM_Process,FilesCount*4);
  prfs->ReadBuffer(pBodyOffsets,FilesCount*4);
  
  const u32 MatchEXT0ID=0x30545845; // EXT0
  u32 EXT0ID=prfs->Readu32();
  if(MatchEXT0ID==EXT0ID) OffsetEXT0=prfs->GetOffset();
//  _consolePrintf("EXT0 %x,%x ofs:%d\n",MatchEXT0ID,EXT0ID,OffsetEXT0);
  
  DATA_IN_IWRAM_FileList static char empty[2]={0,0};
  pFileInfoExtExif_Description=empty;
  pFileInfoExtExif_DateTime=empty;
}

CIPK::~CIPK(void)
{
  if(prfs!=NULL){
    delete prfs; prfs=NULL;
  }
  
  if(pBodyOffsets!=NULL){
    safefree(&MM_Process,pBodyOffsets); pBodyOffsets=NULL;
  }
}

u32 CIPK::GetFilesCount(void)
{
  return(FilesCount);
}

bool CIPK::GetCoverImage(TIPKThumbnail *pth)
{
  pth->pBuf=NULL;
  
  if(OffsetEXT0==0) return(false);
  prfs->SetOffset(OffsetEXT0);
  
  pth->Width=prfs->Readu32();
  pth->Height=prfs->Readu32();
  pth->Ratio=0;
  u32 decompsize=prfs->Readu32();
  u32 compsize=prfs->Readu32();
  
  if((decompsize==0)||(compsize==0)) return(false);
  
  TZLIBData z;
  z.SrcSize=compsize;
  z.pSrcBuf=(u8*)safemalloc(&MM_Temp,z.SrcSize);
  z.DstSize=decompsize;
  z.pDstBuf=(u8*)safemalloc(&MM_Process,z.DstSize);
  
  if((z.pSrcBuf==NULL)||(z.pDstBuf==NULL)){
    if(z.pSrcBuf!=NULL){
      safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
    }
    if(z.pDstBuf!=NULL){
      safefree(&MM_Process,z.pDstBuf); z.pDstBuf=NULL;
    }
    return(false);
  }
  
  prfs->ReadBuffer(z.pSrcBuf,z.SrcSize);
  zlibdecompress(&z);
  safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
  
  pth->pBuf=(u16*)z.pDstBuf;
  
  return(true);
}

u32 CIPK::GetBodyOffset(u32 FileIndex)
{
  if(FilesCount<=FileIndex) return((u32)-1);
  return(pBodyOffsets[FileIndex]);
}

void CIPK::GetFilename(u32 FileIndex,UnicodeChar *pstr)
{
  pstr[0]=(UnicodeChar)0;
  
  u32 ofs=GetBodyOffset(FileIndex);
  if(ofs==(u32)-1) return;
  
  prfs->SetOffset(ofs);
  prfs->Readu32(); // length
  
  UnicodeChar uc=prfs->Readu16();
  while(uc!=0){
    *pstr++=uc;
    uc=prfs->Readu16();
  }
  *pstr=0;
}

bool CIPK::GetFileInfoExtExif(u32 FileIndex)
{
	DATA_IN_IWRAM_FileList static char empty[2]={0,0};
  pFileInfoExtExif_Description=empty;
  pFileInfoExtExif_DateTime=empty;
  
  u32 ofs=GetBodyOffset(FileIndex);
  if(ofs==(u32)-1) return(false);
  
  prfs->SetOffset(ofs);
  prfs->Readu32(); // length
  
  // Skip unicode filename
  while(prfs->Readu16()!=0){
  }
  
  prfs->Readu8(); // Dummy
  
  EFileInfoExt FileInfoExt=(EFileInfoExt)(prfs->Readu8());
  if(FileInfoExt==EFIE_None) return(false);
  
  {
	  DATA_IN_IWRAM_FileList static char buf[256];
    char *pbuf=buf;
    pFileInfoExtExif_Description=pbuf;
    
    char c=prfs->Readu8();
    while(c!=0){
      *pbuf++=c;
      c=prfs->Readu8();
    }
    *pbuf=0;
  }
  
  {
	  DATA_IN_IWRAM_FileList static char buf[256];
    char *pbuf=buf;
    pFileInfoExtExif_DateTime=pbuf;
    
    char c=prfs->Readu8();
    while(c!=0){
      *pbuf++=c;
      c=prfs->Readu8();
    }
    *pbuf=0;
  }
  
  if((pFileInfoExtExif_Description[0]==0)&&(pFileInfoExtExif_DateTime[0]==0)) return(false);
  
  return(true);
}

bool CIPK::GetThumbnail(u32 FileIndex,EIPKThumbnailID thID,TIPKThumbnail *pth)
{
  pth->pBuf=NULL;
  
  u32 ofs=GetBodyOffset(FileIndex);
  if(ofs==(u32)-1) return(false);
  prfs->SetOffset(ofs);
  
  u32 len=prfs->Readu32();
  for(;len!=0;len--){
    prfs->Readu8();
  }
  
  for(u32 skip=0;skip<(u32)thID;skip++){
    prfs->Readu32();
    prfs->Readu32();
    prfs->Readu32();
    prfs->Readu32();
    u32 compsize=prfs->Readu32();
    prfs->ReadSkip(compsize);
  }
  
  pth->Width=prfs->Readu32();
  pth->Height=prfs->Readu32();
  pth->Ratio=prfs->Readu32();
  u32 decompsize=prfs->Readu32();
  u32 compsize=prfs->Readu32();
  
  if((decompsize==0)||(compsize==0)) return(false);
  
  TZLIBData z;
  z.SrcSize=compsize;
  z.pSrcBuf=(u8*)safemalloc(&MM_Temp,z.SrcSize);
  z.DstSize=decompsize;
  z.pDstBuf=(u8*)safemalloc(&MM_Process,z.DstSize);
  
  if((z.pSrcBuf==NULL)||(z.pDstBuf==NULL)){
    if(z.pSrcBuf!=NULL){
      safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
    }
    if(z.pDstBuf!=NULL){
      safefree(&MM_Process,z.pDstBuf); z.pDstBuf=NULL;
    }
    return(false);
  }
  
  prfs->ReadBuffer(z.pSrcBuf,z.SrcSize); // 19200us
  zlibdecompress(&z); // 43392us
  safefree(&MM_Temp,z.pSrcBuf); z.pSrcBuf=NULL;
  
  u16 palcnt=*(u16*)z.pDstBuf;
  
  if(0x8000<=palcnt){
    // 15bit bitmap
    pth->pBuf=(u16*)z.pDstBuf;
    }else{
    if(palcnt<=256){
      // 8bit pallete
      u16 pals[256];
      {
        for(u32 idx=0;idx<256;idx++){
          pals[idx]=0;
        }
        u16 *ppaldata=(u16*)z.pDstBuf;
        ppaldata++;
        for(u32 idx=0;idx<palcnt;idx++){
          pals[idx]=ppaldata[idx];
        }
      }
      
      u8 *pb8data=(u8*)z.pDstBuf;
      pb8data+=2+(palcnt*2);
      
      u32 w=pth->Width;
      u32 h=pth->Height;
      u16 *pb15data=(u16*)safemalloc(&MM_Process,w*h*2);
      
      if(pb15data==NULL){
        safefree(&MM_Process,z.pDstBuf); z.pDstBuf=NULL;
        return(false);
      }
      
      u8 *psrcbuf=pb8data;
      u16 *pdstbuf=pb15data;
      for(u32 y=0;y<h;y++){
        for(u32 x=0;x<w;x++){
          pdstbuf[x]=pals[psrcbuf[x]];
        }
        psrcbuf+=w;
        pdstbuf+=w;
      }
      safefree(&MM_Process,z.pDstBuf); z.pDstBuf=NULL;
      pth->pBuf=pb15data;
      }else{
      _consolePrintf("Unknown Palette type = %d\n",palcnt);
      safefree(&MM_Process,z.pDstBuf); z.pDstBuf=NULL;
      pth->Width=0;
      pth->Height=0;
      pth->pBuf=NULL;
      return(false);
    }
  }
  
  return(true);
}

void CIPK::GetImageInfo(u32 FileIndex,TIPKImageInfo *pimginfo)
{
  u32 ofs=GetBodyOffset(FileIndex);
  if(ofs==(u32)-1) return;
  prfs->SetOffset(ofs);
  
  u32 len=prfs->Readu32();
  for(;len!=0;len--){
    prfs->Readu8();
  }
  
  for(u32 skip=0;skip<(u32)EIPKTHID_SKIP;skip++){
    prfs->Readu32();
    prfs->Readu32();
    prfs->Readu32();
    prfs->Readu32();
    u32 compsize=prfs->Readu32();
    prfs->ReadSkip(compsize);
  }
  
  pimginfo->Width=prfs->Readu32();
  pimginfo->Height=prfs->Readu32();
  pimginfo->MCUXCount=prfs->Readu16();
  pimginfo->dummy=prfs->Readu16();
  pimginfo->MCUYCount=prfs->Readu16();
  pimginfo->BodyFormat=(EIPKBodyFormat)prfs->Readu16();
  
  if((pimginfo->BodyFormat==EIPKBF_CustomJpegYUV111)||(pimginfo->BodyFormat==EIPKBF_CustomJpegYUV411)){
    for(u32 idx=0;idx<64;idx++){
      pimginfo->QuantizeTable[idx]=(s16)prfs->Readu16();
    }
  }
}

