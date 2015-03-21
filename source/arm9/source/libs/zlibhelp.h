
#ifndef zlibhelp_h
#define zlibhelp_h

#include <nds.h>

#define ZLIBTitle "ZLIB (C) 1995-2004 Jean-loup Gailly and Mark Adler"

typedef struct {
  u32 SrcSize;
  u8 *pSrcBuf;
  u32 DstSize;
  u8 *pDstBuf;
} TZLIBData;

extern bool zlibcompress(TZLIBData *pZLIBData,u32 LimitSize);
extern bool zlibdecompress(TZLIBData *pZLIBData);

#endif

