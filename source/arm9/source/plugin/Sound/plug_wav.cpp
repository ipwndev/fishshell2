
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>
#include "_console.h"
#include "memtool.h"
#include "maindef.h"

#include "plug_wav.h"

// --------------------------------------------------------------------

// -----------------
typedef struct {
  u16 wFormatTag;
  u16 nChannels;
  u32 nSamplesPerSec;
  u32 nAvgBytesPerSec;
  u16 nBlockAlign;
  u16 wBitsPerSample;
  u16 cbSize;
} WAVEFORMATEX;

CODE_IN_ITCM_WAVE static FAT_FILE *FileHandle;
CODE_IN_ITCM_WAVE static WAVEFORMATEX wfex;
CODE_IN_ITCM_WAVE static int DataTopOffset;
CODE_IN_ITCM_WAVE static int SampleCount;
CODE_IN_ITCM_WAVE static int SampleOffset;
CODE_IN_ITCM_WAVE static int BytePerSample;

CODE_IN_ITCM_WAVE static int SamplePerFrame;
CODE_IN_ITCM_WAVE static u32 *ReadBuffer;

// --------------------------------------------------------------------

CODE_IN_ITCM_WAVE static bool WaveFile_ReadWaveChunk(void)
{
    FAT2_fseek(FileHandle,0x14,SEEK_SET);
    FAT2_fread((void*)&wfex,sizeof(wfex),1,FileHandle);
  
  if(wfex.wFormatTag!=0x0001){
    _consolePrintf("Illigal CompressFormat Error. wFormatTag=0x%x\n",wfex.wFormatTag);
    return(false);
  }
  
  if((wfex.nChannels!=1)&&(wfex.nChannels!=2)){
    _consolePrintf("Channels Error. nChannels=%d\n",wfex.nChannels);
    return(false);
  }
  
  if((wfex.wBitsPerSample!=8)&&(wfex.wBitsPerSample!=16)){
    _consolePrintf("Bits/Sample Error. wBitsPerSample=%d\n",wfex.wBitsPerSample);
    return(false);
  }
  
  _consolePrintf("fmt:0x%x chns:%d\n",wfex.wFormatTag,wfex.nChannels);
  _consolePrintf("Smpls/Sec:%d AvgBPS:%d\n",wfex.nSamplesPerSec,wfex.nAvgBytesPerSec);
  _consolePrintf("BlockAlign:%d Bits/Smpl:%d\n",wfex.nBlockAlign,wfex.wBitsPerSample);
  
  return(true);
}

CODE_IN_ITCM_WAVE static bool WaveFile_SeekDataChunk(void)
{
    FAT2_fseek(FileHandle,0,SEEK_SET);
  
  // find "data"
  {
    char *readbuf=(char*)safemalloc(&MM_DLLSound,256);
    int size=0;
    int ofs=0;
    
    size=FAT2_fread(readbuf,1,256,FileHandle);
    if(size<4){
      safefree(&MM_DLLSound,readbuf);
      _consolePrint("can not find data chunk.\n");
      return(false);
    }
    
    while(true){
      if(readbuf[ofs]=='d'){
        if((readbuf[ofs+1]=='a')&&(readbuf[ofs+2]=='t')&&(readbuf[ofs+3]=='a')){
          safefree(&MM_DLLSound,readbuf);
          FAT2_fseek(FileHandle,ofs+4,SEEK_SET);
          break;
        }
      }
      ofs++;
      if(ofs==(size-4)){
        safefree(&MM_DLLSound,readbuf);
        _consolePrint("can not find data chunk.\n");
        return(false);
      }
    }
  }
  
  u32 DataSize;
  FAT2_fread(&DataSize,4,1,FileHandle);
  
  if(DataSize==0){
    _consolePrint("DataSize is NULL\n");
    return(false);
  }
  
  BytePerSample=0;
  
  if((wfex.nChannels==1)&&(wfex.wBitsPerSample==8)) BytePerSample=1;
  if((wfex.nChannels==2)&&(wfex.wBitsPerSample==8)) BytePerSample=2;
  if((wfex.nChannels==1)&&(wfex.wBitsPerSample==16)) BytePerSample=2;
  if((wfex.nChannels==2)&&(wfex.wBitsPerSample==16)) BytePerSample=4;
  
  if(BytePerSample==0){
    _consolePrint("Illigal Channels or Bits/Sample or no data\n");
    return(false);
  }
  
  SampleCount=DataSize/BytePerSample;
  
  DataTopOffset=FAT2_ftell(FileHandle);
  
  _consolePrintf("DataTop:%d DataSize:%dbyte\n",DataTopOffset,DataSize);
  
  return(true);
}

CODE_IN_ITCM_WAVE static bool WaveFile_Open(void)
{
    FAT2_fseek(FileHandle,0,SEEK_SET);
  
  u32 RIFFID;
  FAT2_fread(&RIFFID,4,1,FileHandle);
  
  if(RIFFID!=0x46464952){ // check "RIFF"
    _consolePrint("no RIFFWAVEFILE error.");
    _consolePrintf("topdata:0x%04x\n",RIFFID);
    return(false);
  }
  
  if(WaveFile_ReadWaveChunk()==false) return(false);
  if(WaveFile_SeekDataChunk()==false) return(false);
  
  return(true);
}

// ------------------------------------------------------------------------------------

bool PlugWAVE_Start(FAT_FILE *_FileHandle)
{
  FileHandle=_FileHandle;
  
  if(WaveFile_Open()==false){
    return(false);
  }
  
  SampleOffset=0;
  SamplePerFrame=1024;
  
  ReadBuffer=(u32*)safemalloc(&MM_DLLSound,SamplePerFrame*BytePerSample);
  if(ReadBuffer==NULL){
    _consolePrint("out of memory.\n");
    return(false);
  }
  
  return(true);
}

void PlugWAVE_Free(void)
{
  if(ReadBuffer!=NULL){
    safefree(&MM_DLLSound,ReadBuffer); ReadBuffer=NULL;
  }
}

u32 PlugWAVE_Update(u32 *plrbuf)
{
  int SampleCount;
  
  SampleCount=FAT2_fread(ReadBuffer,1,SamplePerFrame*BytePerSample,FileHandle)/BytePerSample;
  
  if(wfex.wBitsPerSample==8){
    if(wfex.nChannels==1){ // 8bit mono
      s8 *readbuf=(s8*)ReadBuffer;
      for(u32 idx=SampleCount;idx!=0;idx--){
        s16 sample=(((s16)*readbuf++)-128)<<8;
        *plrbuf++=(sample&0xffff)|(sample<<16);
      }
      }else{ // 8bit stereo
      s8 *readbuf=(s8*)ReadBuffer;
      for(u32 idx=SampleCount;idx!=0;idx--){
        s16 l=(((s16)*readbuf++)-128)<<8;
        s16 r=(((s16)*readbuf++)-128)<<8;
        *plrbuf++=(l&0xffff)|(r<<16);
      }
    }
    }else{
    if(wfex.nChannels==1){ // 16bit mono
      s16 *readbuf=(s16*)ReadBuffer;
      for(u32 idx=SampleCount;idx!=0;idx--){
        s16 sample=*readbuf++;
        *plrbuf++=(sample&0xffff)|(sample<<16);
      }
      }else{ // 16bit stereo
      s16 *readbuf=(s16*)ReadBuffer;
      for(u32 idx=SampleCount;idx!=0;idx--){
          s16 l=*readbuf++;
          s16 r=*readbuf++;
          *plrbuf++=(l&0xffff)|(r<<16);
      }
    }
  }
  
  SampleOffset+=SampleCount;
  
  return(SampleCount);
}

s32 PlugWAVE_GetPosMax(void)
{
  return(SampleCount);
}

s32 PlugWAVE_GetPosOffset(void)
{
  return(SampleOffset);
}

void PlugWAVE_SetPosOffset(s32 ofs)
{
  if(ofs<0) ofs=0;
  ofs&=~3;
  
  SampleOffset=ofs;
  
  FAT2_fseek(FileHandle,DataTopOffset+(SampleOffset*BytePerSample),SEEK_SET);
}

u32 PlugWAVE_GetChannelCount(void)
{
  return(wfex.nChannels);
}

u32 PlugWAVE_GetSampleRate(void)
{
  return(wfex.nSamplesPerSec);
}

u32 PlugWAVE_GetSamplePerFrame(void)
{
  return(SamplePerFrame);
}

u32 PlugWAVE_GetPlayTimeSec(void)
{
    return(SampleCount/wfex.nSamplesPerSec);
}

int PlugWAVE_GetInfoIndexCount(void)
{
  return(3);
}

bool PlugWAVE_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"wFormatTag=0x%x",wfex.wFormatTag); return(true); break;
    case 1: snprintf(str,len,"%dHz %dbit %dChannels",wfex.nSamplesPerSec,wfex.wBitsPerSample,wfex.nChannels); return(true); break;
    case 2: snprintf(str,len,"Length=%d:%02d",(SampleCount/wfex.nSamplesPerSec)/60,(SampleCount/wfex.nSamplesPerSec)%60); return(true); break;
  }
  return(false);
}

bool PlugWAVE_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugWAVE_GetInfoStrUTF8(int idx,char *str,int len)
{
  return(false);
}
