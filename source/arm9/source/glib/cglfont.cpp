
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cglfont.h"

#include "cglstream.h"

#include "lang.h"

static u32 FontWidthPadding;

CglFont::CglFont(TMM *_pMM,const u8 *_buf,const int _size)
{
  pMM=_pMM;
  
  CglStream stream(_buf,_size);
  
  u32 FontCount;
  
  if(_size<2048){
    FontCount=0x80;
    }else{
    FontCount=0x10000;
  }
  
  DataTable=(u16**)safemalloc_chkmem(pMM,FontCount*4);
  
  u32 DataSize=stream.GetSize()-(FontCount*1);
  Data=(u16*)safemalloc_chkmem(pMM,DataSize);
  
  u8 *pDiffTbl=(u8*)safemalloc_chkmem(&MM_Temp,FontCount*1);
  
  stream.ReadBuffer(pDiffTbl,FontCount*1);
  FontWidthPadding=pDiffTbl[0];
  pDiffTbl[0]=0;
  
  stream.ReadBuffer(Data,DataSize);
  
  u32 Height=Data[0];
  if(glFontHeight!=Height) StopFatalError(13004,"Not support font height. (%d!=%d)\n",glFontHeight,Height);
  
  u32 LastOffset=0;
  
  for(int idx=0;idx<FontCount;idx++){
    u32 diff=pDiffTbl[idx];
    if(diff==0){
      static u16 WidthZero=0;
      DataTable[idx]=&WidthZero;
      }else{
      LastOffset+=diff;
      DataTable[idx]=&Data[LastOffset/2];
    }
  }
  
  if(pDiffTbl!=NULL){
    safefree(&MM_Temp,pDiffTbl); pDiffTbl=NULL;
  }
  
  TextColor=RGB15(0,0,0)|BIT15;
}

CglFont::~CglFont(void)
{
  safefree(pMM,DataTable); DataTable=NULL;
  safefree(pMM,Data); Data=NULL;
}

u16* CglFont::GetBulkData(const TglUnicode uidx) const
{
  u16 *pBulkData=DataTable[uidx];
  if(*pBulkData!=0) return(pBulkData);
  return(DataTable[(u32)'?']);
}

void DrawFont1bppUnder8pix(const u16 *pBulkData,u16 *pbuf,u32 bufwidth,u32 TextColor)
{
	asm volatile (
	"df1bu8_pBulkData .req r0 \n\t"
	"df1bu8_pbuf .req r1 \n\t"
	"df1bu8_bufwidth .req r2 \n\t"
	"df1bu8_TextColor .req r3 \n\t"
	"df1bu8_Height .req r4 \n\t"
	"df1bu8_BitImage .req lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,lr} \n\t"
	" \n\t"
	"mov df1bu8_Height,#12/2 \n\t"
	" \n\t"
	"df1bu8_DrawFont1bpp_LoopYStart: \n\t"
	" \n\t"
	"ldrh df1bu8_BitImage,[df1bu8_pBulkData],#2 \n\t"
	" \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+0) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*0] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+1) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*1] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+2) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*2] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+3) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*3] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+4) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*4] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+5) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*5] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+6) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*6] \n\t"
	"tst df1bu8_BitImage,#1<<((8*0)+7) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*7] \n\t"
	" \n\t"
	"add df1bu8_pbuf,df1bu8_bufwidth \n\t"
	" \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+0) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*0] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+1) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*1] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+2) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*2] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+3) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*3] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+4) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*4] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+5) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*5] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+6) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*6] \n\t"
	"tst df1bu8_BitImage,#1<<((8*1)+7) \n\t"
	"strneh df1bu8_TextColor,[df1bu8_pbuf,#2*7] \n\t"
	" \n\t"
	"add df1bu8_pbuf,df1bu8_bufwidth \n\t"
	" \n\t"
	"subs df1bu8_Height,#1 \n\t"
	"bne df1bu8_DrawFont1bpp_LoopYStart \n\t"
	" \n\t"
	"ldmfd sp!,{r4,pc} \n\t"
	:::"memory"
	);
}

void DrawFont1bppUnder12pix(const u16 *pBulkData,u16 *pbuf,u32 bufwidth,u32 TextColor)
{
	asm volatile (
	"df1bu12_pBulkData .req r0 \n\t"
	"df1bu12_pbuf .req r1 \n\t"
	"df1bu12_bufwidth .req r2 \n\t"
	"df1bu12_TextColor .req r3 \n\t"
	"df1bu12_Height .req r4 \n\t"
	"df1bu12_BitImage .req lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,lr} \n\t"
	" \n\t"
	"mov df1bu12_Height,#12 \n\t"
	" \n\t"
	"df1bu12_DrawFont1bpp_LoopYStart: \n\t"
	" \n\t"
	"ldrh df1bu12_BitImage,[df1bu12_pBulkData],#2 \n\t"
	" \n\t"
	"tst df1bu12_BitImage,#1<<0 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*0] \n\t"
	"tst df1bu12_BitImage,#1<<1 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*1] \n\t"
	"tst df1bu12_BitImage,#1<<2 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*2] \n\t"
	"tst df1bu12_BitImage,#1<<3 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*3] \n\t"
	"tst df1bu12_BitImage,#1<<4 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*4] \n\t"
	"tst df1bu12_BitImage,#1<<5 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*5] \n\t"
	"tst df1bu12_BitImage,#1<<6 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*6] \n\t"
	"tst df1bu12_BitImage,#1<<7 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*7] \n\t"
	"tst df1bu12_BitImage,#1<<8 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*8] \n\t"
	"tst df1bu12_BitImage,#1<<9 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*9] \n\t"
	"tst df1bu12_BitImage,#1<<10 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*10] \n\t"
	"tst df1bu12_BitImage,#1<<11 \n\t"
	"strneh df1bu12_TextColor,[df1bu12_pbuf,#2*11] \n\t"
	" \n\t"
	"add df1bu12_pbuf,df1bu12_bufwidth \n\t"
	" \n\t"
	"subs df1bu12_Height,#1 \n\t"
	"bne df1bu12_DrawFont1bpp_LoopYStart \n\t"
	" \n\t"
	"ldmfd sp!,{r4,pc} \n\t"
	:::"memory"
	);
}

void DrawFont1bppUnder16pix(const u16 *pBulkData,u16 *pbuf,u32 bufwidth,u32 TextColor)
{
	asm volatile (
	"df1bu16_pBulkData .req r0 \n\t"
	"df1bu16_pbuf .req r1 \n\t"
	"df1bu16_bufwidth .req r2 \n\t"
	"df1bu16_TextColor .req r3 \n\t"
	"df1bu16_Height .req r4 \n\t"
	"df1bu16_BitImage .req lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,lr} \n\t"
	" \n\t"
	"mov df1bu16_Height,#12 \n\t"
	" \n\t"
	"df1bu16_DrawFont1bpp_LoopYStart: \n\t"
	" \n\t"
	"ldrh df1bu16_BitImage,[df1bu16_pBulkData],#2 \n\t"
	" \n\t"
	"tst df1bu16_BitImage,#1<<0 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*0] \n\t"
	"tst df1bu16_BitImage,#1<<1 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*1] \n\t"
	"tst df1bu16_BitImage,#1<<2 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*2] \n\t"
	"tst df1bu16_BitImage,#1<<3 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*3] \n\t"
	"tst df1bu16_BitImage,#1<<4 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*4] \n\t"
	"tst df1bu16_BitImage,#1<<5 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*5] \n\t"
	"tst df1bu16_BitImage,#1<<6 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*6] \n\t"
	"tst df1bu16_BitImage,#1<<7 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*7] \n\t"
	"tst df1bu16_BitImage,#1<<8 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*8] \n\t"
	"tst df1bu16_BitImage,#1<<9 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*9] \n\t"
	"tst df1bu16_BitImage,#1<<10 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*10] \n\t"
	"tst df1bu16_BitImage,#1<<11 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*11] \n\t"
	"tst df1bu16_BitImage,#1<<12 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*12] \n\t"
	"tst df1bu16_BitImage,#1<<13 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*13] \n\t"
	"tst df1bu16_BitImage,#1<<14 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*14] \n\t"
	"tst df1bu16_BitImage,#1<<15 \n\t"
	"strneh df1bu16_TextColor,[df1bu16_pbuf,#2*15] \n\t"
	" \n\t"
	"add df1bu16_pbuf,df1bu16_bufwidth \n\t"
	" \n\t"
	"subs df1bu16_Height,#1 \n\t"
	"bne df1bu16_DrawFont1bpp_LoopYStart \n\t"
	" \n\t"
	"ldmfd sp!,{r4,pc} \n\t"
	:::"memory"
	);
}

void CglFont::DrawFont(CglCanvas *pCanvas,const int x,const int y,const TglUnicode uidx) const
{
  const u16 *pBulkData=GetBulkData(uidx);

  int Width=*pBulkData++;
  if(Width==0) return;
  
  u16 *pbuf=pCanvas->GetVRAMBuf();
  u32 bufwidth=pCanvas->GetWidth();
  
  pbuf=&pbuf[(y*bufwidth)+x];

  if(Width<=8){
    DrawFont1bppUnder8pix(pBulkData,pbuf,bufwidth*2,TextColor);
    }else{
    if(Width<=12){
      DrawFont1bppUnder12pix(pBulkData,pbuf,bufwidth*2,TextColor);
      }else{
      DrawFont1bppUnder16pix(pBulkData,pbuf,bufwidth*2,TextColor);
    }
  }
}

static bool isUnicodeArabic(u32 uc)
{
  if(uc<0x0600) return(false);
  
  if((0x0600<=uc)&&(uc<0x0700)) return(true); // UNICODE Arabic
  if((0xfb50<=uc)&&(uc<0xfe00)) return(true); // UNICODE Arabic Presentation Forms-A
  if((0xfe70<=uc)&&(uc<0xff00)) return(true); // UNICODE Arabic Presentation Forms-B
  return(false);
}
int CglFont::GetFontWidth(const TglUnicode uidx) const
{
  u16 *BulkData=GetBulkData(uidx);
  
  int Width=*BulkData;
  
  if(isUnicodeArabic(uidx)==true) Width--;
  
  return(Width+FontWidthPadding);
}

void CglFont::SetTextColor(const u16 Color)
{
  TextColor=Color;
}

bool CglFont::isExists(const TglUnicode uidx) const
{
  if(*DataTable[uidx]!=0) return(true);
  return(false);
}

