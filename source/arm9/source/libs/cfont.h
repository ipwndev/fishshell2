
#ifndef cfont_h
#define cfont_h

#include "unicode.h"

#include "glib.h"

#define DefaultImplantCharsCount (15)
#define DefaultWeekStringsCount (7)
#define DefaultImplantChars "0123456789:/APM"

enum EBitmapMode {EBM_B15,EBM_TGF};

class CFont
{
  EBitmapMode BitmapMode;
  
  CglB15 *pB15;
  CglTGF *pTGF;
  
  u32 ImplantCharsCount;
  const char *pImplantChars;
  
  u32 CharHeight,CharWidth;
  
  u32 CharWidths[DefaultImplantCharsCount];
  s32 CharPads[DefaultImplantCharsCount];
  
  u32 SpaceWidth;
  
  CFont(const CFont&);
  CFont& operator=(const CFont&);
protected:
public:
  CFont(EBitmapMode _BitmapMode,CglB15 *_pB15,CglTGF *_pTGF,bool isWeek);
  ~CFont(void);
  u32 DrawChar(CglCanvas *pCanvas,u32 x,u32 y,char c);
  u32 DrawCharToBlack(CglCanvas *pCanvas,u32 x,u32 y,char c);
  u32 DrawText(CglCanvas *pCanvas,u32 x,u32 y,const char *pstr);
  u32 GetTextWidth(const char *pstr);
  u32 GetTextHeight(void);
};

#endif

