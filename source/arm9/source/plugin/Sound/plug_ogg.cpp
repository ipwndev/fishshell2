/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggVorbis 'TREMOR' CODEC SOURCE CODE.   *
 *                                                                  *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE OggVorbis 'TREMOR' SOURCE CODE IS (C) COPYRIGHT 1994-2002    *
 * BY THE Xiph.Org FOUNDATION http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

 function: simple example decoder using vorbisidec

 ********************************************************************/

/* Takes a vorbis bitstream from stdin and writes raw stereo PCM to
   stdout using vorbisfile. Using vorbisfile is much simpler than
   dealing with libvorbis. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds.h>
#include "_console.h"
#include "memtool.h"
#include "maindef.h"

#include "plug_ogg.h"

#include "libogg/ivorbiscodec.h"
#include "libogg/ivorbisfile.h"

//--------------------------------

size_t callbacks_read_func  (void *ptr, size_t size, size_t nmemb, void *datasource)
{
  int len;
  
  len=FAT2_fread(ptr,size,nmemb,(FAT_FILE *)datasource);
  
  return(len);
}

int    callbacks_seek_func  (void *datasource, ogg_int64_t offset, int whence)
{
  u32 pos;
  
  pos=FAT2_fseek((FAT_FILE *)datasource,offset,whence);
  
  return(pos);
}

int    callbacks_close_func (void *datasource)
{
  return(0);
}

long   callbacks_tell_func  (void *datasource)
{
  return(FAT2_ftell((FAT_FILE *)datasource));
}

static ov_callbacks callbacks = {callbacks_read_func,callbacks_seek_func,callbacks_close_func,callbacks_tell_func};

//--------------------------------

#define PCMReadBufCount (640)
static s16 *PCMReadBuf=NULL;

typedef struct {
  FAT_FILE * FileHandle;
  OggVorbis_File vf;
  u32 RemainSampleCount;
  u32 SampleRate,ChannelCount;
  s32 SampleCount,SampleOffset;
} TOGGInfo;

static TOGGInfo OGGInfo;

//--------------------------------

bool PlugOGG_Start(FAT_FILE * _FileHandle)
{
  OGGInfo.FileHandle=_FileHandle;
  PCMReadBuf=(s16*)malloc(PCMReadBufCount*4); // 16bit2ch
  
  {
    int ret=ov_open_callbacks((void*)OGGInfo.FileHandle,&OGGInfo.vf,NULL,0,callbacks);
    
    switch(ret){
      case OV_EREAD: _consolePrint("ret=OVEREAD A read from media returned an error.\n"); break;
      case OV_ENOTVORBIS: _consolePrint("ret=OV_ENOTVORBIS Bitstream is not Vorbis data.\n"); break;
      case OV_EVERSION: _consolePrint("ret=OV_EVERSION Vorbis version mismatch.\n"); break;
      case OV_EBADHEADER: _consolePrint("ret=OV_EBADHEADER Invalid Vorbis bitstream header.\n"); break;
      case OV_EFAULT: _consolePrint("ret=OV_EFAULT Internal logic fault.\n"); break;
    }
    
    if(ret<0){
      _consolePrint("Input does not appear to be an Ogg bitstream.\n");
      PlugOGG_Free();
      return(false);
    }
  }
  
  {
/*
    char **ptr=ov_comment(&OGGInfo.vf,-1)->user_comments;
    
    if(*ptr!=NULL){
      _consolePrint("UserComments:\n");
      while(*ptr){
        _consolePrintf("%s\n",ptr);
        ++ptr;
      }
    }
*/
    
    _consolePrintf("Encoded by: %s\n",ov_comment(&OGGInfo.vf,-1)->vendor);
    _consolePrint("\n");
  }
  
  {
    vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
    
    OGGInfo.SampleRate=vi->rate;
    OGGInfo.ChannelCount=vi->channels;
    OGGInfo.SampleCount=(s32)(ov_pcm_total(&OGGInfo.vf,-1)/PCMReadBufCount);
//    OGGInfo.SampleCount=(s32)(44100/PCMReadBufCount);
    OGGInfo.SampleOffset=0;
    
    _consolePrintf("SampleRate %dHz\n",OGGInfo.SampleRate);
    _consolePrintf("ChannelCount %dch\n",OGGInfo.ChannelCount);
    _consolePrintf("SampleCount %dsample\n",OGGInfo.SampleCount*PCMReadBufCount);
    _consolePrintf("\n");
    if(vi->bitrate_lower!=0) _consolePrintf("bitrate lower =%dkbps\n",(s32)(vi->bitrate_lower/1000));
    if(vi->bitrate_nominal!=0) _consolePrintf("bitrate normal=%dkbps\n",(s32)(vi->bitrate_nominal/1000));
    if(vi->bitrate_upper!=0) _consolePrintf("bitrate upper =%dkbps\n",(s32)(vi->bitrate_upper/1000));
    if(vi->bitrate_window!=0) _consolePrintf("bitrate window=%dkbps\n",(s32)(vi->bitrate_window/1000));
    
    if((128000<vi->bitrate_lower)||(128000<vi->bitrate_nominal)||(128000<vi->bitrate_upper)){
      _consolePrint("\n");
      _consolePrint("bitrate exceeds 128kbps.\nPossibility with noise.\n");
    }
  }
  
//  _consolePrint("warrning:In the OGG interface, there is a memory leak bug.\n");
  
  return(true);
}

void PlugOGG_Free(void)
{
  _consolePrint("FreeOGG();\n");
  
  ov_clear(&OGGInfo.vf);
  
  if(PCMReadBuf!=NULL){
    free(PCMReadBuf);
    PCMReadBuf=NULL;
  }
  
  OGGInfo.FileHandle=0;
  OGGInfo.RemainSampleCount=0;
  
  OGGInfo.SampleRate=0;
  OGGInfo.ChannelCount=0;
  OGGInfo.SampleCount=0;
  OGGInfo.SampleOffset=0;
}

u32 PlugOGG_Update(u32 *plrbuf)
{
  u32 unitsize;
  
  if(OGGInfo.ChannelCount==1){
    unitsize=2;
    }else{
    unitsize=4;
  }
  
  u32 SamplePos=0;
  
  {
    u8 *WriteBuf;
    int current_section;
    long ret;
    
    WriteBuf=(u8*)PCMReadBuf;
    WriteBuf+=SamplePos*unitsize;
    
    while(SamplePos<PCMReadBufCount){
      ret=ov_read(&OGGInfo.vf,WriteBuf,(PCMReadBufCount-SamplePos)*unitsize,&current_section);
      
      if(ret==0){
        _consolePrint("Done.\n");
        break;
      }
      if(ret<0){
        _consolePrintf("ov_read() decode error:%d. skip.\n",ret);
      }
      if(ret<unitsize){
        _consolePrintf("ov_read() fatal error:%d\n",ret);
        while(1);
        //return(0);
      }
      if(0<ret){
        WriteBuf+=ret;
        SamplePos+=ret/unitsize;
      }
    }
  }
  
  if(SamplePos>PCMReadBufCount){
    _consolePrintf("Fatal error. SamplePos=%d\n",SamplePos);
    _consolePrintf("Fatal error. PCMReadBufCount=%d\n",PCMReadBufCount);
    while(1);
    //return(0);
  }
  
  if(plrbuf!=NULL){
    if(OGGInfo.ChannelCount==1){
      s16 *pcmbuf=(s16*)PCMReadBuf;
      u32 idx;
      for(idx=SamplePos;idx>0;idx--){
        s16 sample=*pcmbuf++;
        *plrbuf++=(sample&0xffff)|(sample<<16);
      }
      }else{
      s16 *pcmbuf=(s16*)PCMReadBuf;
      u32 idx;
      for(idx=SamplePos;idx>0;idx--){
        s16 l=*pcmbuf++;
        s16 r=*pcmbuf++;
        *plrbuf++=(l&0xffff)|(r<<16);
      }
    }
  }
  
  OGGInfo.SampleOffset+=SamplePos/PCMReadBufCount;
  
  return(SamplePos);
}

s32 PlugOGG_GetPosMax(void)
{
  return(OGGInfo.SampleCount);
}

s32 PlugOGG_GetPosOffset(void)
{
  return(OGGInfo.SampleOffset);
}

void PlugOGG_SetPosOffset(s32 ofs)
{
  ov_pcm_seek(&OGGInfo.vf,((long)ofs)*PCMReadBufCount);
  OGGInfo.SampleOffset=((s32)ov_pcm_tell(&OGGInfo.vf))/PCMReadBufCount;
}

u32 PlugOGG_GetChannelCount(void)
{
  return(OGGInfo.ChannelCount);
}

u32 PlugOGG_GetSampleRate(void)
{
  return(OGGInfo.SampleRate);
}

u32 PlugOGG_GetSamplePerFrame(void)
{
  return(PCMReadBufCount);
}

u32 PlugOGG_GetPlayTimeSec(void)
{
    return(0);
}

int PlugOGG_GetInfoIndexCount(void)
{
//  vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
  vorbis_comment *vc=ov_comment(&OGGInfo.vf,-1);
  
  return(vc->comments+2);
}

bool PlugOGG_GetInfoStrL(int idx,char *str,int len)
{
  vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
  vorbis_comment *vc=ov_comment(&OGGInfo.vf,-1);
  
  if(idx<vc->comments){
    return(false);
  }
  
  idx-=vc->comments;
  
  switch(idx){
    case 0: snprintf(str,len,"oggver%d %dHz %s",vi->version,(int)(vi->rate),(vi->channels==2) ? "stereo" : "mono"); return(true); 
    case 1: snprintf(str,len,"up=%d nom=%d low=%d win=%d kbps",(int)(vi->bitrate_upper/1000),(int)(vi->bitrate_nominal/1000),(int)(vi->bitrate_lower/1000),(int)(vi->bitrate_window/1000)); return(true); 
  }
  
  return(false);
}

bool PlugOGG_GetInfoStrW(int idx,UnicodeChar *str,int len)
{
  return(false);
}

bool PlugOGG_GetInfoStrUTF8(int idx,char *str,int len)
{
//  vorbis_info *vi=ov_info(&OGGInfo.vf,-1);
  vorbis_comment *vc=ov_comment(&OGGInfo.vf,-1);
  
  if(idx<vc->comments){
    snprintf(str,len,"%s",vc->user_comments[idx]);
    return(true);
  }
  
  return(false);
}

