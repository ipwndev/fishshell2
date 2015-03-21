
#include <stdlib.h>
#include <nds.h>

#include "glib.h"
#include "glmemtool.h"
#include "cgltgf.h"

#include "cglstream.h"

CglTGF::CglTGF(TMM *_pMM,const u8 *_buf,const int _size)
{
  pMM=_pMM;
  
  CglStream stream(_buf,_size);
  
  Width=stream.Readu16();
  Height=stream.Readu16();
  
  datasize=_size;
  
  //if(VerboseDebugLog==true) _consolePrintf("new CglTGF: w,h=%d,%d size=%d\n",Width,Height,datasize);
  
  int size=stream.GetSize()-stream.GetOffset();
  pdata=(u16*)safemalloc_chkmem(pMM,size);
  
  stream.ReadBuffer(pdata,size);
  
  ppLineOffsets=(u16**)safemalloc_chkmem(pMM,Height*4);
  
  u16 *_pdata=pdata;
  
  for(int y=0;y<Height;y++){
    ppLineOffsets[y]=_pdata;
    int x=0;
    while(x<Width){
      u16 curdata=*_pdata++;
      int alpha=curdata & 0xff;
      int len=curdata >> 8;
      
      x+=len;
      if(alpha!=31) _pdata+=len;
    }
  }
}

CglTGF::~CglTGF(void)
{
  safefree(pMM,pdata); pdata=NULL;
  safefree(pMM,ppLineOffsets); ppLineOffsets=NULL;
}
u32 CglTGF::GetDataSize(void) const
{
  return(datasize);
}
u16* CglTGF::GetData(void) const
{
  return(pdata);
}

int CglTGF::GetWidth(void) const
{
  return(Width);
}

int CglTGF::GetHeight(void) const
{
  return(Height);
}

u32 CglTGF_BitBlt_Body(const u16 *pdata,const u16 *pBuf,const u32 Width)
{
	asm volatile (
	"REG_pdata .req r0 \n\t"
	"REG_pBuf .req r1 \n\t"
	"REG_Width .req r2 \n\t"
	" \n\t"
	"REG_pdataMaster .req r4 \n\t"
	"REG_alpha .req r5 \n\t"
	"REG_len .req r6 \n\t"
	"REG_src .req r7 \n\t"
	"REG_add .req r8 \n\t"
	"REG_tmp1 .req r9 \n\t"
	"REG_tmp2 .req r10 \n\t"
	"REG_tmp3 .req r11 \n\t"
	"REG_jumptable .req lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,lr} \n\t"
	" \n\t"
	"mov REG_pdataMaster,REG_pdata \n\t"
	"ldr REG_jumptable,=CglTGF_BitBlt_jumptable \n\t"
	" \n\t"
	"CglTGF_BitBlt_Start: \n\t"
	"ldrh REG_alpha,[REG_pdata],#2 \n\t"
	"lsr REG_len,REG_alpha,#8 \n\t"
	"and REG_alpha,#0xff \n\t"
	" \n\t"
	"sub REG_Width,REG_len @ Width-=len for interlock \n\t"
	"ldr pc,[REG_jumptable,REG_alpha,lsl #2] \n\t"
	" \n\t"
	".macro  CglTGF_BitBlt_TransEnd \n\t"
	"sub r0,REG_pdata,REG_pdataMaster \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10,r11,pc} \n\t"
	".endm \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha0: \n\t"
	"CglTGF_BitBlt_Alpha0_Loop: \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	"subs REG_len,#1 \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	"bne CglTGF_BitBlt_Alpha0_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha2: \n\t"
	"ldr REG_tmp1,=((1<<4)<<0) | ((1<<4)<<5) | ((1<<4)<<10) \n\t"
	"CglTGF_BitBlt_Alpha2_Loop: \n\t"
	"ldrh REG_src,[REG_pBuf] \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	"subs REG_len,#1 \n\t"
	"and REG_src,REG_src,REG_tmp1 \n\t"
	"add REG_add,REG_add,REG_src,lsr #4 \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	"bne CglTGF_BitBlt_Alpha2_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha4: \n\t"
	"ldr REG_tmp1,=((3<<3)<<0) | ((3<<3)<<5) | ((3<<3)<<10) \n\t"
	"CglTGF_BitBlt_Alpha4_Loop: \n\t"
	"ldrh REG_src,[REG_pBuf] \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	"subs REG_len,#1 \n\t"
	"and REG_src,REG_src,REG_tmp1 \n\t"
	"add REG_add,REG_add,REG_src,lsr #3 \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	"bne CglTGF_BitBlt_Alpha4_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha8: \n\t"
	"ldr REG_tmp1,=((7<<2)<<0) | ((7<<2)<<5) | ((7<<2)<<10) \n\t"
	"CglTGF_BitBlt_Alpha8_Loop: \n\t"
	"ldrh REG_src,[REG_pBuf] \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	"subs REG_len,#1 \n\t"
	"and REG_src,REG_src,REG_tmp1 \n\t"
	"add REG_add,REG_add,REG_src,lsr #2 \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	"bne CglTGF_BitBlt_Alpha8_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha16: \n\t"
	"ldr REG_tmp1,=((15<<1)<<0) | ((15<<1)<<5) | ((15<<1)<<10) \n\t"
	"CglTGF_BitBlt_Alpha16_Loop: \n\t"
	"ldrh REG_src,[REG_pBuf] \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	"subs REG_len,#1 \n\t"
	"and REG_src,REG_src,REG_tmp1 \n\t"
	"add REG_add,REG_add,REG_src,lsr #1 \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	"bne CglTGF_BitBlt_Alpha16_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_AlphaAny: \n\t"
	"lsl REG_alpha,#11 @ alpha 5bit to 16bit \n\t"
	" \n\t"
	"CglTGF_BitBlt_AlphaAny_Loop: \n\t"
	"ldrh REG_src,[REG_pBuf] \n\t"
	"ldrh REG_add,[REG_pdata],#2 \n\t"
	" \n\t"
	"and REG_tmp1,REG_src,#(0x1f<<0) \n\t"
	"smulwb REG_tmp1,REG_alpha,REG_tmp1 \n\t"
	"and REG_tmp2,REG_src,#(0x1f<<5) \n\t"
	"add REG_add,REG_tmp1 \n\t"
	"smulwb REG_tmp2,REG_alpha,REG_tmp2 \n\t"
	"and REG_tmp3,REG_src,#(0x1f<<10) \n\t"
	"smulwb REG_tmp3,REG_alpha,REG_tmp3 \n\t"
	"and REG_tmp2,REG_tmp2,#(0x1f<<5) \n\t"
	"add REG_add,REG_tmp2 \n\t"
	"and REG_tmp3,REG_tmp3,#(0x1f<<10) \n\t"
	"add REG_add,REG_tmp3 \n\t"
	" \n\t"
	"strh REG_add,[REG_pBuf],#2 \n\t"
	" \n\t"
	"subs REG_len,#1 \n\t"
	"bne CglTGF_BitBlt_AlphaAny_Loop \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_Alpha31: \n\t"
	"add REG_pBuf,REG_len,lsl #1 \n\t"
	" \n\t"
	"cmp REG_Width,#0 \n\t"
	"bne CglTGF_BitBlt_Start \n\t"
	"CglTGF_BitBlt_TransEnd \n\t"
	" \n\t"
	"CglTGF_BitBlt_jumptable: \n\t"
	".word CglTGF_BitBlt_Alpha0,CglTGF_BitBlt_Alpha0,CglTGF_BitBlt_Alpha2,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_Alpha4,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_Alpha8,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_Alpha16,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny \n\t"
	".word CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_AlphaAny,CglTGF_BitBlt_Alpha31 \n\t"
	:::"memory"
	);
}

void CglTGF::BitBlt(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop) const
{
  u16 *_pdata=pdata; 
  u16 *pBuf=pDestCanvas->GetVRAMBuf();
  pBuf=&pBuf[(nDestTop*pDestCanvas->GetWidth())+nDestLeft];
  
  u32 DestWidth=pDestCanvas->GetWidth();
  u32 DestHeight=pDestCanvas->GetHeight();
  
  u32 drawHeight=Height;
  if((nDestTop+drawHeight)>DestHeight) drawHeight=DestHeight-nDestTop;
  
  for(int y=0;y<drawHeight;y++){
    u32 srclen=CglTGF_BitBlt_Body(_pdata,pBuf,Width);
//    _consolePrintf("f%d/%d: %x,%x %d %d\n",y,Height,_pdata,pBuf,Width,srclen);
    _pdata+=srclen/2;
    pBuf+=DestWidth;
  }
}

void CglTGF::BitBltLimitY(CglCanvas *pDestCanvas,const int nDestLeft,const int nDestTop,const int nHeight,const int nSrcTop) const
{
  u16 *_pdata=ppLineOffsets[nSrcTop];
  u16 *pBuf=pDestCanvas->GetVRAMBuf();
  pBuf=&pBuf[(nDestTop*pDestCanvas->GetWidth())+nDestLeft];
  
  u32 DestWidth=pDestCanvas->GetWidth();
  u32 DestHeight=pDestCanvas->GetHeight();
  
  if(nSrcTop<0) return;
  if(Height<=nSrcTop) return;
  if(nHeight<=0) return;
  if(Height<nHeight) return;

  u32 drawHeight=nHeight;
  if((nDestTop+drawHeight)>DestHeight) drawHeight=DestHeight-nDestTop;
  
  for(int y=0;y<drawHeight;y++){
    u32 srclen=CglTGF_BitBlt_Body(_pdata,pBuf,Width);
//    _consolePrintf("y%d/%d: %x,%x %d %d\n",y,nHeight,_pdata,pBuf,Width,srclen);
    _pdata+=srclen/2;
    pBuf+=DestWidth;
  }
}

