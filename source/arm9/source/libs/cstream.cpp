
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include "cstream.h"

CStream::CStream(const u8 *_buf,const int _size)
{
  buf=_buf;
  size=_size;
  ofs=0;
}

CStream::~CStream(void)
{
}

int CStream::GetOffset(void) const
{
  return(ofs);
}

void CStream::SetOffset(int _ofs)
{
  ofs=_ofs;
  if(size<ofs) ofs=size;
}

int CStream::GetSize(void) const
{
  return(size);
}

void CStream::OverrideSize(int _size)
{
  size=_size;
}

bool CStream::eof(void) const
{
  if(ofs==size){
    return(true);
    }else{
    return(false);
  }
}

u8 CStream::Readu8(void)
{
  if(eof()==true) return(0);
  
  return(buf[ofs++]);
}

u16 CStream::Readu16(void)
{
  u16 data;
  
  data=(u16)Readu8();
  data=data | ((u16)Readu8() << 8);
  
  return(data);
}

u32 CStream::Readu32(void)
{
  u32 data;
  
  data=(u32)Readu8();
  data=data | ((u32)Readu8() << 8);
  data=data | ((u32)Readu8() << 16);
  data=data | ((u32)Readu8() << 24);
  
  return(data);
}

int CStream::ReadBuffer(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return(0);
  
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
    if(readsize<0) readsize=0;
  }
  
  if(readsize!=0){
    memcpy((u8*)_dstbuf,&buf[ofs],readsize);
    ofs+=readsize;
  }
  
  return(readsize);
}

int CStream::ReadBuffer16bit(void *_dstbuf,const int _readsize)
{
  return(ReadBuffer(_dstbuf,_readsize));
}

static inline void MemCopy32swi256bit(const void *src,void *dst,u32 len)
{
  swiFastCopy((void*)src,dst,COPY_MODE_COPY | (len/4));
}

int CStream::ReadBuffer32bit(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return(0);
  
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
    if(readsize<0) readsize=0;
  }
  
  int copysize=readsize;
  
  int swis=copysize&(~31);
  if(swis!=0){
    MemCopy32swi256bit(&buf[ofs],(u8*)_dstbuf,swis);
    _dstbuf=(void*)&((u8*)_dstbuf)[swis];
    ofs+=swis;
    
    copysize-=swis;
  }
  
  if(copysize!=0){
    memcpy((u8*)_dstbuf,&buf[ofs],copysize);
    ofs+=copysize;
  }
  
  return(readsize);
}

void CStream::ReadSkip(const int _readsize)
{
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
    if(readsize<0) readsize=0;
  }
  
  ofs+=readsize;
}

