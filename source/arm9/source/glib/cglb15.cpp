
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglb15.h"

#include "cglstream.h"

CglB15::CglB15(TMM *_pMM,const u8 *_buf,const int _size)
{
  pMM=_pMM;
  
  if(_buf==NULL){
//    StopFatalError(12801,"CglB15::CglB15: _buf is NULL.\n");
    u32 size=_size;
    Width=(size>>0)&0xffff;
    Height=(size>>16)&0xffff;
    data=(u16*)safemalloc_chkmem(pMM,Height*Width*sizeof(u16));
    pCanvas=new CglCanvas(pMM,&data[0],Width,Height,pf15bit);
    return;
  }
  
  CglStream stream(_buf,_size);
  
  Width=stream.Readu16();
  Height=stream.Readu16();
  
  if(stream.Readu16()==0){
    TransFlag=false;
    }else{
    TransFlag=true;
  }
  
  stream.Readu16();
  data=(u16*)safemalloc_chkmem(pMM,Height*Width*sizeof(u16));
  
  stream.ReadBuffer(data,Height*Width*sizeof(u16));
  
  pCanvas=new CglCanvas(pMM,&data[0],Width,Height,pf15bit);
}

CglB15::~CglB15(void)
{
  delete pCanvas; pCanvas=NULL;
  safefree(pMM,data); data=NULL;
}

int CglB15::GetWidth(void) const
{
  return(Width);
}

int CglB15::GetHeight(void) const
{
  return(Height);
}

void CglB15::BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop) const
{
  pCanvas->BitBlt(pDestCanvas,nDestLeft,nDestTop,nWidth,nHeight,nSrcLeft,nSrcTop,TransFlag);
}

