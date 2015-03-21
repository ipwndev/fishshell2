
#ifndef _const_h
#define _const_h

#include "arm9tcm.h"
#include "_console.h"

static inline unsigned int __current_sp(void)
{
	unsigned int csp;
	asm volatile (
		"mov %[csp], sp \n\t"
		:[csp]"=r"(csp)
		::"memory"
	);
	return csp;
}
static inline unsigned int __current_pc(void)
{
	unsigned int csp;
	asm volatile (
		"mov %[csp], pc \n\t"
		:[csp]"=r"(csp)
		::"memory"
	);
	return csp;
}

extern void __StopFatalError__(const char *filename,int linenum,const char *funcname,s32 errnum,const char* format, ...);
#define StopFatalError(errnum, ...) __StopFatalError__(__FILE__,__LINE__,__FUNCTION__,errnum,__VA_ARGS__);

#define TCM_StackStart (0x0b003f00)
extern u32 *pDTCMEND,*pMTCMEND;

static inline void DTCM_StackCheck(s32 ID) // 動画以外モード用スタックオーバーフローチェック
{
  u32 *p=pDTCMEND;
  if(*p!=(u32)p) StopFatalError(11701,"DTCM Stack overflow!!\n");
  if(ID==-1) return;
  for(;p<(u32*)__current_sp();p++){
    if(*p!=(u32)p) break;
  }
  _consolePrintf("DStack:%d Cur:%d, (%d/%d).\n",ID,TCM_StackStart-(u32)__current_sp(),TCM_StackStart-(u32)p,TCM_StackStart-(u32)pDTCMEND);
}

static inline void MTCM_StackCheck(s32 ID) // 動画モード用スタックオーバーフローチェック
{
  u32 *p=pMTCMEND;
  if(*p!=(u32)p) StopFatalError(11702,"MTCM Stack overflow!!\n");
  if(ID==-1) return;
  for(;p<(u32*)__current_sp();p++){
    if(*p!=(u32)p) break;
  }
  _consolePrintf("MStack:%d Cur:%d, (%d/%d).\n",ID,TCM_StackStart-(u32)__current_sp(),TCM_StackStart-(u32)p,TCM_StackStart-(u32)pMTCMEND);
}

#define BIT0 (1<<0)
#define BIT1 (1<<1)
#define BIT2 (1<<2)
#define BIT3 (1<<3)
#define BIT4 (1<<4)
#define BIT5 (1<<5)
#define BIT6 (1<<6)
#define BIT7 (1<<7)
#define BIT8 (1<<8)
#define BIT9 (1<<9)
#define BIT10 (1<<10)
#define BIT11 (1<<11)
#define BIT12 (1<<12)
#define BIT13 (1<<13)
#define BIT14 (1<<14)
#define BIT15 (1<<15)
#define BIT16 (1<<16)
#define BIT17 (1<<17)
#define BIT18 (1<<18)
#define BIT19 (1<<19)
#define BIT20 (1<<20)
#define BIT21 (1<<21)
#define BIT22 (1<<22)
#define BIT23 (1<<23)
#define BIT24 (1<<24)
#define BIT25 (1<<25)
#define BIT26 (1<<26)
#define BIT27 (1<<27)
#define BIT28 (1<<28)
#define BIT29 (1<<29)
#define BIT30 (1<<30)
#define BIT31 (1<<31)

#define ScreenWidth (256)
#define ScreenHeight (192)

//#define attrinline __attribute__ ((always_inline, const)) static inline 
#define attrinline __attribute__ ((always_inline)) static inline 
//#define attrinline 

/*
static void ExeclusiveWaitForPressButton(void)
{
  u16 KEYS_CUR;
  
  KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  while(KEYS_CUR!=0){
    KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
  while(KEYS_CUR==0){
    KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
  while(KEYS_CUR!=0){
    KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
}
*/

#endif

