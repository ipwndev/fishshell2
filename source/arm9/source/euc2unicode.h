
#ifndef euc2unicode_h
#define euc2unicode_h

#include <nds.h>
#include "unicode.h"

typedef struct {
  void *pBinary;
  s32 BinarySize;
  const u8 *panktbl;
  const u16 *ps2utbl;
  u16 *pu2stbl;
} TEUC2Unicode;

extern TEUC2Unicode EUC2Unicode;

extern void EUC2Unicode_Init(void);
extern void EUC2Unicode_Free(void);
extern void EUC2Unicode_Load(void);
extern void EUC2Unicode_Convert(const char *pStrL,UnicodeChar *pStrW,u32 dstlen);
extern void Unicode2EUC_Convert(const UnicodeChar *pStrW,char *pStrL,u32 dstlen);

#endif

