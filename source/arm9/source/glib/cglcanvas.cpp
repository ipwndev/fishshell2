
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglcanvas.h"

static CglFont *pCglFontDefault=NULL;

#define _GetScanLine(y) (&VRAMBuf[y*Width])

CglCanvas::CglCanvas(TMM *_pMM,u16 *_VRAMBuf,const int _Width,const int _Height,const EPixelFormat _PixelFormat)
{
//  glDebugPrintf("new Canvas %x,%d,%d,%d\n",_VRAMBuf,_Width,_Height,_PixelFormat);
  
  pMM=_pMM;
  
  VRAMBuf=NULL;
  VRAMBufInsideAllocatedFlag=false;
  Width=0;
  Height=0;
  Color=0;
  LastX=0;
  LastY=0;
  
  pCglFont=pCglFontDefault;
  
  SetVRAMBuf(_VRAMBuf,_Width,_Height,_PixelFormat);
}

CglCanvas::~CglCanvas(void)
{
  if(VRAMBufInsideAllocatedFlag==true){
    VRAMBufInsideAllocatedFlag=false;
    safefree(pMM,VRAMBuf); VRAMBuf=NULL;
  }
}

CODE_IN_ITCM_GLIB u16* CglCanvas::GetVRAMBuf(void) const
{
  return(VRAMBuf);
}

CODE_IN_ITCM_GLIB void CglCanvas::SetVRAMBuf(u16 *_VRAMBuf,const int _Width,const int _Height,const EPixelFormat _PixelFormat)
{
  if(VRAMBufInsideAllocatedFlag==true){
    VRAMBufInsideAllocatedFlag=false;
    safefree(pMM,VRAMBuf); VRAMBuf=NULL;
  }
  
  Width=_Width;
  Height=_Height;
  
  PixelFormat=_PixelFormat;
  
  if(_VRAMBuf==NULL){
    VRAMBufInsideAllocatedFlag=true;
    VRAMBuf=(u16*)safemalloc_chkmem(pMM,Width*Height*sizeof(u16));
    }else{
    VRAMBufInsideAllocatedFlag=false;
    VRAMBuf=_VRAMBuf;
  }
}

int CglCanvas::GetWidth(void) const
{
  return(Width);
}

int CglCanvas::GetHeight(void) const
{
  return(Height);
}

void CglCanvas::FillFull(const u16 _Color)
{
  u16 *pbuf=VRAMBuf;
  u32 size=Width*Height;
  u32 col=(u32)(_Color | (_Color<<16));
  
  if((size&1)!=0){
    pbuf[size-1]=col;
    size--;
  }
  
  MemSet32CPU(col,(u32*)pbuf,size*2);
}

void CglCanvas::FillFast(const int x,const int y,const int w,const int h)
{
  // 条件：キャンバス横サイズが2pixel単位、x,wが2pixel単位
  
  u32 col=(u32)(Color | (Color<<16));
  
  for(int py=y;py<(y+h);py++){
    u16 *pbuf=_GetScanLine(py);
    pbuf+=x;
    
    MemSet16CPU(col,pbuf,w*2);
  }
}

u16* CglCanvas::GetScanLine(const int y) const
{
  return(_GetScanLine(y));
}

bool CglCanvas::isInsidePosition(const int x,const int y) const
{
  if((x<0)||(Width<=x)) return(false);
  if((y<0)||(Height<=y)) return(false);
  
  return(true);
}

void CglCanvas::SetPixel(const int x,const int y,const u16 rgb)
{
  if(isInsidePosition(x,y)==false) return;
  
  _GetScanLine(y)[x]=rgb;
}

void CglCanvas::SetPixelAlpha(const int x,const int y,const u16 rgb,const int Alpha)
{
  if(isInsidePosition(x,y)==false) return;
  
  _GetScanLine(y)[x]=ColorMargeAlpha(_GetScanLine(y)[x],rgb,Alpha);
}

void CglCanvas::SetPixelAlphaAdd(const int x,const int y,const u16 rgb,const int Alpha)
{
  if(isInsidePosition(x,y)==false) return;
  
  _GetScanLine(y)[x]=ColorMargeAlphaAdd(_GetScanLine(y)[x],rgb,Alpha);
}

u16 CglCanvas::GetPixel(const int x,const int y) const
{
  if(isInsidePosition(x,y)==false) return(0);
  
  return(_GetScanLine(y)[x]);
}

void CglCanvas::SetColor(const u16 _Color)
{
  Color=_Color;
}

u16 CglCanvas::GetColor(void)
{
  return(Color);
}
void CglCanvas::DrawLine(const int x1,const int y1,const int x2,const int y2)
{
  if((x1==x2)&&(y1==y2)) return;
  
  if(x1==x2){
    int ys,ye;
    if(y1<y2){
      ys=y1;
      ye=y2-1;
      }else{
      ys=y2+1;
      ye=y1;
    }
    for(int py=ys;py<=ye;py++){
      SetPixel(x1,py,Color);
    }
    return;
  }
  
  if(y1==y2){
    int xs,xe;
    if(x1<x2){
      xs=x1;
      xe=x2-1;
      }else{
      xs=x2+1;
      xe=x1;
    }
    for(int px=xs;px<=xe;px++){
      SetPixel(px,y1,Color);
    }
    return;
  }
  
//  StopFatalError(12902,"Blocked free line function.\n");
  
#if 1
  if(abs(x2-x1)>abs(y2-y1)){
    int px=0;
    float py=0;
    int xe=x2-x1;
    float ye=y2-y1;
    int xv;
    float yv;
    
    if(0<xe){
      xv=1;
      }else{
      xv=-1;
    }
    yv=ye/abs(xe);
    
    while(px!=xe){
      SetPixel(x1+px,y1+(int)py,Color);
      px+=xv;
      py+=yv;
    }
    return;
    
    }else{
    float px=0;
    int py=0;
    float xe=x2-x1;
    int ye=y2-y1;
    float xv;
    int yv;
    
    xv=xe/abs(ye);
    if(0<ye){
      yv=1;
      }else{
      yv=-1;
    }
    
    while(py!=ye){
      SetPixel(x1+(int)px,y1+py,Color);
      px+=xv;
      py+=yv;
    }
    return;
  }
#endif
}

void CglCanvas::MoveTo(const int x,const int y)
{
  LastX=x;
  LastY=y;
}

void CglCanvas::LineTo(const int x,const int y)
{
  DrawLine(LastX,LastY,x,y);
  
  LastX=x;
  LastY=y;
}


void CglCanvas::FillBox(int x,int y,int w,int h) const
{
  if(x<0){
    w-=-x;
    x=0;
  }
  
  if(y<0){
    h-=-y;
    y=0;
  }
  
  if(w<0) return;
  if(h<0) return;
  
  if(Width<=x) return;
  if(Height<=y) return;
  
  if(Width<(x+w)) w=Width-x;
  if(Height<(y+h)) h=Height-y;
  
  u16 *pdata=&VRAMBuf[x+(y*Width)];
  u32 col=Color;
  
  for(int y=0;y<h;y++){
    for(int x=0;x<w;x++){
      pdata[x]=col;
    }
    pdata+=Width;
  }
}

void CglCanvas::DrawBox(const int x,const int y,const int w,const int h)
{
  if((w==0)||(h==0)) return;
  
  if((w==1)&&(h==1)){
    SetPixel(x,y,Color);
    return;
  }
  
  if(w==1){
    DrawLine(x,y,x,y+h);
    return;
  }
  
  if(h==1){
    DrawLine(x,y,x+w,y);
    return;
  }
  
  const int x1=x,y1=y,x2=x+w-1,y2=y+h-1;
  
  DrawLine(x1,y1,x2,y1);
  DrawLine(x2,y1,x2,y2);
  DrawLine(x2,y2,x1,y2);
  DrawLine(x1,y2,x1,y1);
}

void CglCanvas::SetFontTextColor(const u16 Color)
{
  CglFont *pFont=(CglFont*)pCglFont;
  
  pFont->SetTextColor(Color);
}

const char *CglCanvas::TextOutA(const int x,const int y,const char *str) const
{
  if((str==NULL)||(*str==0)) return(str);
  
  int dx=x;
  int dy=y;
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  while(*str!=0){
    TglUnicode uidx=(TglUnicode)*str;
    u32 w=pFont->GetFontWidth(uidx);
    if(Width<(dx+w)) return(str);
    pFont->DrawFont((CglCanvas*)this,dx,dy,uidx);
    dx+=w;
    str++;
  }
  
  return(str);
}

const TglUnicode *CglCanvas::TextOutW(const int x,const int y,const TglUnicode *str) const
{
  if((str==NULL)||(*str==0)) return(str);
  
  int dx=x;
  int dy=y;
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  while(*str!=0){
    u32 w=pFont->GetFontWidth(*str);
    if(Width<(dx+w)) return(str);
    pFont->DrawFont((CglCanvas*)this,dx,dy,*str);
    dx+=w;
    str++;
  }
  
  return(str);
}

void CglCanvas::TextOutUTF8(const int x,const int y,const char *str) const
{
  if((str==NULL)||(*str==0)) return;
  
  int dx=x;
  int dy=y;
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  while(*str!=0){
    u32 b0=(byte)str[0],b1=(byte)str[1],b2=(byte)str[2];
    TglUnicode uc;
    
    if(b0<0x80){
      uc=b0;
      str++;
      }else{
      if((b0&0xe0)==0xc0){ // 0b 110. ....
        uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
        str+=2;
        }else{
        if((b0&0xf0)==0xe0){ // 0b 1110 ....
          uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
          str+=3;
          }else{
          uc=(u16)'?';
          str+=4;
        }
      }
    }
    
    u32 w=pFont->GetFontWidth(uc);
    if(Width<(dx+w)) return;
    pFont->DrawFont((CglCanvas*)this,dx,dy,uc);
    dx+=w;
  }
}

int CglCanvas::GetTextWidthA(const char *str) const
{
  int w=0;
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  while(*str!=0){
    TglUnicode uidx=(TglUnicode)*str++;
    w+=pFont->GetFontWidth(uidx);
  }
  
  return(w);
}

int CglCanvas::GetTextWidthW(const TglUnicode *str) const
{
  int w=0;
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  while(*str!=0){
    w+=pFont->GetFontWidth(*str);
    str++;
  }
  
  return(w);
}

int CglCanvas::GetTextWidthUTF8(const char *str) const
{
  if(str==NULL) return(0);
  if(*str==0) return(0);
  
  CglFont *pFont=(CglFont*)pCglFont;
  
  int w=0;
  
  while(*str!=0){
    u16 b0=(byte)str[0],b1=(byte)str[1],b2=(byte)str[2];
    TglUnicode uc;
    
    if(b0<0x80){
      uc=b0;
      str++;
      }else{
      if((b0&0xe0)==0xc0){ // 0b 110. ....
        uc=((b0&~0xe0)<<6)+((b1&~0xc0)<<0);
        str+=2;
        }else{
        if((b0&0xf0)==0xe0){ // 0b 1110 ....
          uc=((b0&~0xf0)<<12)+((b1&~0xc0)<<6)+((b2&~0xc0)<<0);
          str+=3;
          }else{
          uc=(u16)'?';
          str+=4;
        }
      }
    }
    
    w+=pFont->GetFontWidth(uc);
  }
  
  return(w);
}

int CglCanvas::GetCharWidthW(const TglUnicode wchar) const
{
  CglFont *pFont=(CglFont*)pCglFont;
  return(pFont->GetFontWidth(wchar));
}
void CglCanvas::SetCglFont(void *_pCglFont)
{
  if(_pCglFont==NULL){
    pCglFont=pCglFontDefault;
    }else{
    pCglFont=_pCglFont;
  }
}

void CglCanvas::BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop,const bool TransFlag) const
{
  if(TransFlag==false){
    BitBltBeta(pDestCanvas,nDestLeft,nDestTop,nWidth,nHeight,nSrcLeft,nSrcTop);
    }else{
    BitBltTrans(pDestCanvas,nDestLeft,nDestTop,nWidth,nHeight,nSrcLeft,nSrcTop);
  }
}

void CglCanvas::BitBltBeta(CglCanvas *pDestCanvas,int nDestLeft,int nDestTop,int nWidth,int nHeight,int nSrcLeft,int nSrcTop) const
{
  int dstw=pDestCanvas->GetWidth();
  int dsth=pDestCanvas->GetHeight();
  
  if(nDestLeft<0){
    nWidth-=-nDestLeft;
    nSrcLeft+=-nDestLeft;
    nDestLeft=0;
  }
  
  if(nDestTop<0){
    nHeight-=-nDestTop;
    nSrcTop+=-nDestTop;
    nDestTop=0;
  }
  
  if(nWidth<=0) return;
  if(nHeight<=0) return;
  
  if(Width<=nSrcLeft) return;
  if(Height<=nSrcTop) return;
  
  if(Width<(nSrcLeft+nWidth)) nWidth=Width-nSrcLeft;
  if(Height<(nSrcTop+nHeight)) nHeight=Height-nSrcTop;
  
  if(dstw<=nDestLeft) return;
  if(dsth<=nDestTop) return;
  
  if(dstw<(nDestLeft+nWidth)) nWidth=dstw-nDestLeft;
  if(dsth<(nDestTop+nHeight)) nHeight=dsth-nDestTop;
  
  u16 *psrcbuf=&VRAMBuf[nSrcLeft+(nSrcTop*Width)];
  u32 srcw=Width;
  u16 *pdstbuf=&pDestCanvas->GetScanLine(nDestTop)[nDestLeft];
  
  u32 len=nWidth*2;
  
  if( (((u32)psrcbuf&3)==0) && (((u32)pdstbuf&3)==0) && ((len&3)==0) ){
    for(u32 y=0;y<(u32)nHeight;y++){
      MemCopy32CPU(psrcbuf,pdstbuf,len);
      psrcbuf+=srcw;
      pdstbuf+=dstw;
    }
    }else{
    for(u32 y=0;y<(u32)nHeight;y++){
      MemCopy16CPU(psrcbuf,pdstbuf,len);
      psrcbuf+=srcw;
      pdstbuf+=dstw;
    }
  }
}

void CglCanvas::BitBltTrans(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nWidth,const int nHeight,const int nSrcLeft,const int nSrcTop) const
{
  int w=nWidth;
  int h=nHeight;
  
  if(Width<=nSrcLeft) return;
  if(Height<=nSrcTop) return;
  
  if(Width<(nSrcLeft+w)) w=Width-nSrcLeft;
  if(Height<(nSrcTop+h)) h=Height-nSrcTop;
  
  u16 *pdata=&VRAMBuf[nSrcLeft+(nSrcTop*Width)];
  
  for(int y=0;y<h;y++){
    for(int x=0;x<w;x++){
      u16 data=pdata[x];
      if(data!=0){
        pDestCanvas->SetPixel(nDestLeft+x,nDestTop+y,data);
      }
    }
    pdata+=Width;
  }
}

void CglCanvas::BitBltFullBeta(CglCanvas *pDestCanvas) const
{
  //MemCopy32CPU(VRAMBuf,pDestCanvas->GetVRAMBuf(),Width*Height*2);
    MemCopy32CPU(VRAMBuf,pDestCanvas->GetVRAMBuf(),pDestCanvas->GetWidth()*pDestCanvas->GetHeight()*2);
}

u8* CglCanvas::CreateBMPImage(u32 *size) const
{
  u32 linelen;
  
  linelen=Width*3;
  linelen=(linelen+3)&(~3);
  
  u32 bufsize=14+40+(Height*linelen);
  u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
  u32 ofs=0;
  
#define add8(d) { pbuf[ofs]=d; ofs++; }
#define add16(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); ofs+=2; }
#define add32(d) { pbuf[ofs+0]=(u8)((d>>0)&0xff); pbuf[ofs+1]=(u8)((d>>8)&0xff); pbuf[ofs+2]=(u8)((d>>16)&0xff); pbuf[ofs+3]=(u8)((d>>24)&0xff); ofs+=4; }

  // BITMAPFILEHEADER
  
  // bfType 2 byte ファイルタイプ 'BM' - OS/2, Windows Bitmap
  add8((u8)'B');
  add8((u8)'M');
  // bfSize 4 byte ファイルサイズ (byte)
  add32(bufsize);
  // bfReserved1 2 byte 予約領域 常に 0
  add16(0);
  // bfReserved2 2 byte 予約領域 常に 0
  add16(0);
  // bfOffBits 4 byte ファイル先頭から画像データまでのオフセット (byte)
  add32(14+40);
  
  // BITMAPINFOHEADER
  
  // biSize 4 byte 情報ヘッダのサイズ (byte) 40
  add32(40);
  // biWidth 4 byte 画像の幅 (ピクセル)
  add32(Width);
  // biHeight 4 byte 画像の高さ (ピクセル) biHeight の値が正数なら，画像データは下から上へ
  add32(Height);
  // biPlanes 2 byte プレーン数 常に 1
  add16(1);
  // biBitCount 2 byte 1 画素あたりのデータサイズ (bit)
  add16(24);
  // biCopmression 4 byte 圧縮形式 0 - BI_RGB (無圧縮)
  add32(0);
  // biSizeImage 4 byte 画像データ部のサイズ (byte) 96dpi ならば3780
  add32(0);
  // biXPixPerMeter 4 byte 横方向解像度 (1mあたりの画素数) 96dpi ならば3780
  add32(0);
  // biYPixPerMeter 4 byte 縦方向解像度 (1mあたりの画素数) 96dpi ならば3780
  add32(0);
  // biClrUsed 4 byte 格納されているパレット数 (使用色数) 0 の場合もある
  add32(0);
  // biCirImportant 4 byte 重要なパレットのインデックス 0 の場合もある
  add32(0);
  
  for(int y=Height-1;0<=y;y--){
    u16 *psrcbm=&VRAMBuf[y*Width];
    for(int x=0;x<Width;x++){
      u16 col=*psrcbm++;
      u8 b=(col>>10)&0x1f;
      u8 g=(col>>5)&0x1f;
      u8 r=(col>>0)&0x1f;
      add8(b<<3);
      add8(g<<3);
      add8(r<<3);
    }
    for(u32 x=0;x<(linelen-(Width*3));x++){
      add8(0);
    }
  }
  
#undef add8
#undef add16
#undef add32
  
  *size=bufsize;
  return(pbuf);
}

#include "shell.h"
void CglCanvas::TextOutLang(const int x,const int y,const char *str) const
{
  static char buf[128];
  
  if(Shell_isEUCmode()==false){
    u32 idx=0;
    while(str[idx]!='|'){
      buf[idx]=str[idx];
      idx++;
    }
    buf[idx]=0;
    }else{
    while(*str++!='|'){
    }
    u32 idx=0;
    while(str[idx]!=0){
      buf[idx]=str[idx];
      idx++;
    }
    buf[idx]=0;
  }
  
  TextOutUTF8(x,y,buf);
}
int CglCanvas::GetTextWidthLang(const char *str) const
{
  static char buf[128];
  
  if(Shell_isEUCmode()==false){
    u32 idx=0;
    while(str[idx]!='|'){
      buf[idx]=str[idx];
      idx++;
    }
    buf[idx]=0;
    }else{
    while(*str++!='|'){
    }
    u32 idx=0;
    while(str[idx]!=0){
      buf[idx]=str[idx];
      idx++;
    }
    buf[idx]=0;
  }
  
  return(GetTextWidthUTF8(buf));
}

