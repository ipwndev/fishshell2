
#include <stdlib.h>
#include <string.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglstream.h"

CglStream::CglStream(const u8 *_buf,const int _size)
{
  buf=_buf;
  size=_size;
  ofs=0;
}

CglStream::~CglStream(void)
{
}

int CglStream::GetOffset(void) const
{
  return(ofs);
}

void CglStream::SetOffset(int _ofs)
{
  ofs=_ofs;
  if(size<ofs) ofs=size;
}

int CglStream::GetSize(void) const
{
  return(size);
}

bool CglStream::eof(void) const
{
  if(ofs==size){
    return(true);
    }else{
    return(false);
  }
}

u8 CglStream::Readu8(void)
{
  if(eof()==true) return(0);
  
  return(buf[ofs++]);
}

u16 CglStream::Readu16(void)
{
  u16 data;
  
  data=(u16)Readu8();
  data=data | ((u16)Readu8() << 8);
  
  return(data);
}

u32 CglStream::Readu32(void)
{
  u32 data;
  
  data=(u32)Readu8();
  data=data | ((u32)Readu8() << 8);
  data=data | ((u32)Readu8() << 16);
  data=data | ((u32)Readu8() << 24);
  
  return(data);
}

void CglStream_ReadBuffer_fastcopy(const void *psrc,void *pdst,u32 size)
{
	asm volatile (
	"psrc .req r0 \n\t"
	"pdst .req r1 \n\t"
	"size .req r2 \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6} \n\t"
	" \n\t"
	"cmp size,#4*4 \n\t"
	"blo copy8bit \n\t"
	" \n\t"
	"copy32bitx4: \n\t"
	"ldmia psrc!,{r3,r4,r5,r6} \n\t"
	"stmia pdst!,{r3,r4,r5,r6} \n\t"
	"subs size,size,#4*4 \n\t"
	"cmp size,#4*4 \n\t"
	"bhs copy32bitx4 \n\t"
	" \n\t"
	"cmp size,#0 \n\t"
	"beq copyend \n\t"
	" \n\t"
	"copy8bit: \n\t"
	"ldrb r3,[psrc],#1 \n\t"
	"subs size,size,#1 \n\t"
	"strb r3,[pdst],#1 \n\t"
	"bne copy8bit \n\t"
	" \n\t"
	"copyend: \n\t"
	"ldmfd sp!, {r4,r5,r6} \n\t"
	"bx lr       \n\t"
	:::"memory"
	);
}

void CglStream::ReadBuffer(void *_dstbuf,const int _readsize)
{
  if(eof()==true) return;
  
  int readsize;
  
  if((ofs+_readsize)<=size){
    readsize=_readsize;
    }else{
    readsize=size-ofs;
  }
  
  if(readsize==0) return;
  
  const void *_srcbuf=&buf[ofs];
  
  if( (((u32)_dstbuf&3)==0) && (((u32)_srcbuf&3)==0) ){
    CglStream_ReadBuffer_fastcopy(_srcbuf,_dstbuf,readsize);
    }else{
    memmove((u8*)_dstbuf,_srcbuf,readsize);
  }
  
  ofs+=readsize;
}

