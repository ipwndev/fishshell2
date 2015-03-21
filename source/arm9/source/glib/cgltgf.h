
#ifndef CglTGF_h
#define CglTGF_h

#include <stdlib.h>
#include <nds.h>

#include "glmemtool.h"
#include "cglcanvas.h"

class CglTGF
{
  TMM *pMM;
  int Width,Height;
  u16 *pdata;
  u32 datasize;
  u16 **ppLineOffsets;
  CglTGF(const CglTGF&);
  CglTGF& operator=(const CglTGF&);
public:
  CglTGF(TMM *_pMM,const u8 *_buf,const int _size);
  ~CglTGF(void);
  u32 GetDataSize(void) const;
  u16* GetData(void) const;
  int GetWidth(void) const;
  int GetHeight(void) const;
  void BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop) const;
  void BitBltLimitY(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nHeight,const int nSrcTop) const;
};

#endif

