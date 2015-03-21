
#ifndef CglB15_h
#define CglB15_h

#include <stdlib.h>
#include <nds.h>

#include "glmemtool.h"
#include "cglcanvas.h"

class CglB15
{
  TMM *pMM;
  bool TransFlag;
  int Width,Height;
  u16 *data;
  CglB15(const CglB15&);
  CglB15& operator=(const CglB15&);
public:
  CglCanvas *pCanvas;
  CglB15(TMM *_pMM,const u8 *_buf,const int _size);
  ~CglB15(void);
  int GetWidth(void) const;
  int GetHeight(void) const;
  void BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop) const;
};

#endif

