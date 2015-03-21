
#ifndef cglcanvas_h
#define cglcanvas_h

#include <stdlib.h>
#include <nds.h>

#include "glmemtool.h"
#include "tglunicode.h"

enum EPixelFormat {pf15bit};

#define glCanvasTextHeight (12)

class CglCanvas
{
  TMM *pMM;
  u16 *VRAMBuf;
  bool VRAMBufInsideAllocatedFlag;
  int Width;
  int Height;
  EPixelFormat PixelFormat;
  u16 Color;
  int LastX,LastY;
  void *pCglFont;
  CglCanvas(const CglCanvas&);
  CglCanvas& operator=(const CglCanvas&);
  bool isInsidePosition(const int x,const int y) const;
  void BitBltBeta(CglCanvas *pDestCanvas,int nDestLeft,int nDestTop,int nWidth,int nHeight,int nSrcLeft,int nSrcTop) const;
  void BitBltTrans(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop) const;
public:
  CglCanvas(TMM *_pMM,u16 *_VRAMBuf,const int _Width,const int _Height,const EPixelFormat _PixelFormat);
  ~CglCanvas(void);
  u16* GetVRAMBuf(void) const;
  void SetVRAMBuf(u16 *_VRAMBuf,const int _Width,const int _Height,const EPixelFormat _PixelFormat);
  int GetWidth(void) const;
  int GetHeight(void) const;
  void FillFull(const u16 _Color);
  void FillFast(const int x,const int y,const int w,const int h);
  u16* GetScanLine(const int y) const;
  void SetPixel(const int x,const int y,const u16 rgb);
  void SetPixelAlpha(const int x,const int y,const u16 rgb,const int Alpha);
  void SetPixelAlphaAdd(const int x,const int y,const u16 rgb,const int Alpha);
  u16 GetPixel(const int x,const int y) const;
  void SetColor(const u16 _Color);
  u16 GetColor(void);
  void DrawLine(const int x1,const int y1,const int x2,const int y2);
  void MoveTo(const int x,const int y);
  void LineTo(const int x,const int y);
  void FillBox(int x,int y,int w,int h) const;
  void DrawBox(const int x,const int y,const int w,const int h);
  void SetFontTextColor(const u16 Color);
  const char *TextOutA(const int x,const int y,const char *str) const;
  void TextOutLang(const int x,const int y,const char *str) const;
  const TglUnicode *TextOutW(const int x,const int y,const TglUnicode *str) const;
  void TextOutUTF8(const int x,const int y,const char *str) const;
  int GetTextWidthA(const char *str) const;
  int GetTextWidthLang(const char *str) const;
  int GetTextWidthW(const TglUnicode *str) const;
  int GetTextWidthUTF8(const char *str) const;
  int GetCharWidthW(const TglUnicode wchar) const;
  void SetCglFont(void *_pCglFont);
  void BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop,const bool TransFlag) const;
  void BitBltFullBeta(CglCanvas *pDestCanvas) const;
  u8* CreateBMPImage(u32 *size) const;
};

#endif

