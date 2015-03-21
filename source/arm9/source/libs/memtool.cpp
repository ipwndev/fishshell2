
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_const.h"

#include "memtool.h"
#include "arm9tcm.h"

//#define MEMTOOL32_C

#define MEMCHK_COPY(align,p1,p2,p3) { \
  if((p1==NULL)||(p2==NULL)||(p3==0)||(((u32)p1&align)!=0)||(((u32)p2&align)!=0)){ \
    StopFatalError(13406,"COPY. Hooked memory address error. %s%d (%d) p1=0x%08x,p2=0x%08x,p3=0x%08x\n",__FILE__,__LINE__,align,p1,p2,p3); \
  } \
}

#define MEMCHK_SET(align,p1,p2,p3) { \
  if((p2==NULL)||(p3==0)||(((u32)p2&align)!=0)){ \
    StopFatalError(13407,"SET. Hooked memory address error. %s%d (%d) p1=0x%08x,p2=0x%08x,p3=0x%08x\n",__FILE__,__LINE__,align,p1,p2,p3); \
  } \
}

#undef MEMCHK_COPY
#define MEMCHK_COPY(align,p1,p2,p3)
#undef MEMCHK_SET
#define MEMCHK_SET(align,p1,p2,p3)

#define CACHE_LINE_SIZE (32)

void DCache_FlushRangeOverrun(const void *v,u32 size)
{
//  Flush up. (ダーティーデータをライトバックせずにキャッシュを無効化する）

  u32 va=(u32)v;
  if((va<0x02000000)||(0x02400000<=va)) return;
  
  va&=~(CACHE_LINE_SIZE-1);
  size+=CACHE_LINE_SIZE;
  
  size+=CACHE_LINE_SIZE-1;
  size&=~(CACHE_LINE_SIZE-1);
  
  if(size==0) return;

  while(size!=0){  
	  asm volatile (
	    "mcr p15, 0, %[va], c7, c6, 1 \n\t"
		::[va]"r"(va):"memory"
		);
    va+=CACHE_LINE_SIZE;
    size-=CACHE_LINE_SIZE;
	}
}

void DCache_CleanRangeOverrun(const void *v,u32 size)
{
// Clean up. (ダーティーデータをライトバッファに送ってクリアする。キャッシュは有効のまま）

  u32 va=(u32)v;
  if((va<0x02000000)||(0x02400000<=va)) return;

  va&=~(CACHE_LINE_SIZE-1);
  size+=CACHE_LINE_SIZE;
  
  size+=CACHE_LINE_SIZE-1;
  size&=~(CACHE_LINE_SIZE-1);
  
  if(size==0) return;

  while(size!=0){  
	  asm volatile (
		 "mcr p15, 0, %[va], c7, c10, 1 \n\t"
		::[va]"r"(va):"memory"
		);
    va+=CACHE_LINE_SIZE;
    size-=CACHE_LINE_SIZE;
	}
}

CODE_IN_ITCM_MemTool void MemCopy8CPU(const void *src,void *dst,u32 len)
{MEMCHK_COPY(0,src,dst,len);
  if((len&1)==0){
    if( (((u32)src&1)==0) && (((u32)dst&1)==0) ){
      MemCopy16CPU(src,dst,len);
      return;
    }
  }
  
  len>>=0;
  if(len==0) return;
  
  u8 *_src=(u8*)src;
  u8 *_dst=(u8*)dst;
  
  for(u32 idx=0;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}

CODE_IN_ITCM_MemTool void MemCopy16CPU(const void *src,void *dst,u32 len)
{MEMCHK_COPY(1,src,dst,len);
  if((len&3)==0){
    if( (((u32)src&3)==0) && (((u32)dst&3)==0) ){
      MemCopy32CPU(src,dst,len);
      return;
    }
  }
  
  len>>=1;
  if(len==0) return;
  
  u16 *_src=(u16*)src;
  u16 *_dst=(u16*)dst;
  
  for(u32 idx=0;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}

CODE_IN_ITCM_MemTool void MemCopy32CPU(const void *src,void *dst,u32 len)
#ifdef MEMTOOL32_C
{MEMCHK_COPY(3,src,dst,len);
  
  len>>=2;
  if(len==0) return;
  
  u32 *_src=(u32*)src;
  u32 *_dst=(u32*)dst;
  
  for(u32 idx=0;idx<len;idx++){
    _dst[idx]=_src[idx];
  }
}
#else
{/* MEMCHK_COPY(3,src,dst,len); */
	asm volatile (
	"c32psrc .req r0 \n\t"
	"c32pdst .req r1 \n\t"
	"c32size .req r2 \n\t"
	" \n\t"
	"cmp c32size,#0 \n\t"
	"bxeq lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6,r7,r8,r9,r10} \n\t"
	" \n\t"
	"cmp c32size,#4*8 \n\t"
	"blo c32set32x1 \n\t"
	" \n\t"
	"c32set32x8: \n\t"
	"ldmia c32psrc!,{r3,r4,r5,r6,r7,r8,r9,r10} \n\t"
	"stmia c32pdst!,{r3,r4,r5,r6,r7,r8,r9,r10} \n\t"
	"subs c32size,c32size,#4*8 \n\t"
	"cmp c32size,#4*8 \n\t"
	"bhs c32set32x8 \n\t"
	" \n\t"
	"cmp c32size,#0 \n\t"
	"beq c32setend \n\t"
	" \n\t"
	"c32set32x1: \n\t"
	"ldr r3,[c32psrc],#4 \n\t"
	"subs c32size,c32size,#4 \n\t"
	"str r3,[c32pdst],#4 \n\t"
	"bne c32set32x1 \n\t"
	" \n\t"
	"c32setend: \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9,r10} \n\t"
	"bx lr       \n\t"
	:::"memory"
	);
}
#endif

CODE_IN_ITCM_MemTool void MemSet8CPU(u8 v,void *dst,u32 len)
{MEMCHK_SET(0,v,dst,len);
  len>>=0;
  if(len==0) return;
  
  u8 *_dst=(u8*)dst;
  
  for(u32 cnt=0;cnt<len;cnt++){
    _dst[cnt]=v;
  }
}

CODE_IN_ITCM_MemTool void MemSet16CPU(u16 v,void *dst,u32 len)
{MEMCHK_SET(1,v,dst,len);
  len>>=1;
  if(len==0) return;

  u16 *_dst=(u16*)dst;
  
  for(u32 idx=0;idx<len;idx++){
    _dst[idx]=v;
  }
}

CODE_IN_ITCM_MemTool void MemSet32CPU(u32 v,void *dst,u32 len)
#ifdef MEMTOOL32_C
{MEMCHK_SET(3,v,dst,len);
  len>>=2;
  if(len==0) return;

  u32 *_dst=(u32*)dst;
  
  for(u32 idx=0;idx<len;idx++){
    _dst[idx]=v;
  }
}
#else
{/* MEMCHK_SET(3,v,dst,len); */
	asm volatile (
	"s32data .req r0 \n\t"
	"s32pbuf .req r1 \n\t"
	"s32size .req r2 \n\t"
	" \n\t"
	"cmp s32size,#0 \n\t"
	"bxeq lr \n\t"
	" \n\t"
	"stmfd sp!,{r4,r5,r6,r7,r8,r9} \n\t"
	" \n\t"
	"mov r3,s32data \n\t"
	"mov r4,s32data \n\t"
	"mov r5,s32data \n\t"
	"mov r6,s32data \n\t"
	"mov r7,s32data \n\t"
	"mov r8,s32data \n\t"
	"mov r9,s32data \n\t"
	" \n\t"
	"cmp s32size,#4*8 \n\t"
	"blo s32set32x1 \n\t"
	" \n\t"
	"s32set32x8: \n\t"
	"stmia s32pbuf!,{s32data,r3,r4,r5,r6,r7,r8,r9} \n\t"
	"subs s32size,s32size,#4*8 \n\t"
	"cmp s32size,#4*8 \n\t"
	"bhs s32set32x8 \n\t"
	" \n\t"
	"cmp s32size,#0 \n\t"
	"beq s32setend \n\t"
	" \n\t"
	"s32set32x1: \n\t"
	"str s32data,[s32pbuf],#4 \n\t"
	"subs s32size,s32size,#4 \n\t"
	"bne s32set32x1 \n\t"
	" \n\t"
	"s32setend: \n\t"
	"ldmfd sp!,{r4,r5,r6,r7,r8,r9} \n\t"
	"bx lr       \n\t"
	:::"memory"
	);
}
#endif

// ----------------------------------------------

bool (*safemalloc_CallBack_RequestFreeMemory_PlugSound)(void)=NULL;
bool (*safemalloc_CallBack_RequestFreeMemory_PlugImage)(void)=NULL;

static bool CallRequestFree(void)
{
  bool retry=false;
  
  static bool rn=false;
  
  if(rn==false){
    rn=true;
    if((retry==false)&&(safemalloc_CallBack_RequestFreeMemory_PlugSound!=NULL)){
      if(safemalloc_CallBack_RequestFreeMemory_PlugSound()==true) retry=true;
    }
    if((retry==false)&&(safemalloc_CallBack_RequestFreeMemory_PlugImage!=NULL)){
      if(safemalloc_CallBack_RequestFreeMemory_PlugImage()==true) retry=true;
    }
    }else{
    rn=false;
    if((retry==false)&&(safemalloc_CallBack_RequestFreeMemory_PlugImage!=NULL)){
      if(safemalloc_CallBack_RequestFreeMemory_PlugImage()==true) retry=true;
    }
    if((retry==false)&&(safemalloc_CallBack_RequestFreeMemory_PlugSound!=NULL)){
      if(safemalloc_CallBack_RequestFreeMemory_PlugSound()==true) retry=true;
    }
  }
  
  return(retry);
}

// ----------------------------------------------

#include "memtool_MM.h"

// ----------------------------------------------

void *__safemalloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,int size)
{
// _consolePrintf("safemalloc(%d);\n",size);

  if(pMM==NULL) __StopFatalError__(filename,linenum,funcname,106,"MemMgr is NULL.");
  
  if(size<=0) return(NULL);
  
  void *ptr=NULL;
  
  while(ptr==NULL){
    ptr=malloc(size+(8*2)); // 先頭直前と終端直後に検査コードを入れる
    if(ptr!=NULL) break;
    
    bool retry=CallRequestFree();
    if(retry==false){
      _consolePrintf("safemalloc(%d) failed allocate error. not use auto allocator.\n",size);
      return(NULL);
    }
  }
  
  ptr=(void*)((u32)ptr+8);
  
  //if(VerboseDebugLog==true) _consolePrintf("safemalloc(%d)=0x%08x [%s] %s:%d %s\n",size,ptr,pMM->pName,filename,linenum,funcname);

  MM_Set(pMM,filename,linenum,funcname,(u32)ptr,size);
  
  u32 *pbuf32=(u32*)ptr;
  pbuf32[-2]=0xa5a6a7a8;
  pbuf32[-1]=0xa1a2a3a4;
  
  u8 *pbuf8=(u8*)ptr;
  pbuf8+=size;
  pbuf8[0]=0xb0;
  pbuf8[1]=0xb1;
  pbuf8[2]=0xb2;
  pbuf8[3]=0xb3;
  pbuf8[4]=0xb4;
  pbuf8[5]=0xb5;
  pbuf8[6]=0xb6;
  pbuf8[7]=0xb7;
  
  return(ptr);
}

void *__safemalloc_chkmem__(TMM *pMM,const char *filename,int linenum,const char *funcname,int size)
{
  if(pMM==NULL) __StopFatalError__(filename,linenum,funcname,107,"MemMgr is NULL.");
  
  void *p=__safemalloc__(pMM,filename,linenum,funcname,size);
  
  if(p==NULL){
    MM_ShowAllocated(&MM_DLLSound);
    MM_ShowAllocated(&MM_DLLImage);
    PrintFreeMem_Accuracy();
    __StopFatalError__(filename,linenum,funcname,101,"Insufficient memory. malloc(%d)\n",size);
  }
  
  return(p);
}

void __safefree__(TMM *pMM,const char *filename,int linenum,const char *funcname,void *ptr)
{
  if(pMM==NULL) __StopFatalError__(filename,linenum,funcname,108,"MemMgr is NULL.");
  
  if(ptr==NULL) __StopFatalError__(filename,linenum,funcname,102,"Request free null pointer in safefree.\n");
  
//  _consolePrintf("safefree(0x%08x)=0d\n",ptr,MM_GetSize((u32)ptr));
  //if(VerboseDebugLog==true) _consolePrintf("safefree(0x%08x)=%d %s:%d %s\n",ptr,MM_GetSize(pMM,(u32)ptr),filename,linenum,funcname);
  
  {
    TMM_List *pList=NULL;
    for(u32 idx=0;idx<pMM->ListCount;idx++){
      TMM_List *_pList=&pMM->pLists[idx];
      if(_pList->adr==(u32)ptr) pList=_pList;
    }
    if(pList==NULL) __StopFatalError__(filename,linenum,funcname,104,"Not found target ptr in MM->pLists. [0x%08x]\n",ptr);
    
    u32 size=pList->size;
    u8 *pbuf=(u8*)ptr;
    
    if((pbuf[-8]!=0xa8)||(pbuf[-7]!=0xa7)||(pbuf[-6]!=0xa6)||(pbuf[-5]!=0xa5)||(pbuf[-4]!=0xa4)||(pbuf[-3]!=0xa3)||(pbuf[-2]!=0xa2)||(pbuf[-1]!=0xa1)||
       (pbuf[size+0]!=0xb0)||(pbuf[size+1]!=0xb1)||(pbuf[size+2]!=0xb2)||(pbuf[size+3]!=0xb3)||(pbuf[size+4]!=0xb4)||(pbuf[size+5]!=0xb5)||(pbuf[size+6]!=0xb6)||(pbuf[size+7]!=0xb7)){
      _consolePrint("\n");
      _consolePrintf("adr=0x%08x size=%d %s:%d %s\n",pbuf,size,pList->filename,pList->linenum,pList->funcname);
      for(u32 idx=8;idx>0;idx--){
        _consolePrintf("%02x,",pbuf[-idx]);
      }
      _consolePrint("\n");
      for(u32 idx=0;idx<24;idx++){
        _consolePrintf("%02x,",pbuf[size+idx-8]);
      }
      _consolePrint("\n");
      __StopFatalError__(filename,linenum,funcname,105,"Memory check error. Ignore writing code?\n");
    }
    
  }
  
  MM_Clear(pMM,(u32)ptr);
  
  ptr=(void*)((u32)ptr-8);
  free(ptr);
}

void *__safecalloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,int nmemb, int size)
{
  if(pMM==NULL) __StopFatalError__(filename,linenum,funcname,109,"MemMgr is NULL.");
  
  size*=nmemb;
  
// _consolePrintf("safecalloc(%d);\n",size);

  if(size<=0) return(NULL);
  
  void *ptr=NULL;
  
  while(ptr==NULL){
    ptr=malloc(size+(8*2)); // 先頭直前と終端直後に検査コードを入れる
    if(ptr!=NULL) break;
    
    bool retry=CallRequestFree();
    if(retry==false){
      _consolePrintf("safemalloc(%d) failed allocate error. not use auto allocator.\n",size);
      return(NULL);
    }
  }
  
  ptr=(void*)((u32)ptr+8);
  
//  _consolePrintf("safecalloc(%d)=0x%08x\n",size,ptr);

  //if(VerboseDebugLog==true) _consolePrintf("safecalloc(%d)=0x%08x [%s] %s:%d %s\n",size,ptr,pMM->pName,filename,linenum,funcname);
  
  MM_Set(pMM,filename,linenum,funcname,(u32)ptr,size);
  
  u32 *pbuf32=(u32*)ptr;
  pbuf32[-2]=0xa5a6a7a8;
  pbuf32[-1]=0xa1a2a3a4;
  
  u8 *pbuf8=(u8*)ptr;
  pbuf8+=size;
  pbuf8[0]=0xb0;
  pbuf8[1]=0xb1;
  pbuf8[2]=0xb2;
  pbuf8[3]=0xb3;
  pbuf8[4]=0xb4;
  pbuf8[5]=0xb5;
  pbuf8[6]=0xb6;
  pbuf8[7]=0xb7;
  
  MemSet8CPU(0,ptr,size);
  
  return(ptr);
}

void *__saferealloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,void *ptr, int size)
{
  if(pMM==NULL) __StopFatalError__(filename,linenum,funcname,110,"MemMgr is NULL.");
  
  if(ptr==NULL) ptr=safemalloc(pMM,size);
  if(ptr==NULL) __StopFatalError__(filename,linenum,funcname,103,"Insufficient memory. realloc(%d)\n",size);
  
  MM_Clear(pMM,(u32)ptr);
  
  ptr=(void*)((u32)ptr-8);
  ptr=realloc(ptr,size+(8*2));
  ptr=(void*)((u32)ptr+8);
  
  //if(VerboseDebugLog==true) _consolePrintf("saferealloc(%d)=0x%08x [%s] %s:%d %s\n",size,ptr,pMM->pName,filename,linenum,funcname);
  
  MM_Set(pMM,filename,linenum,funcname,(u32)ptr,size);
  
  u32 *pbuf32=(u32*)ptr;
  pbuf32[-2]=0xa5a6a7a8;
  pbuf32[-1]=0xa1a2a3a4;
  
  u8 *pbuf8=(u8*)ptr;
  pbuf8+=size;
  pbuf8[0]=0xb0;
  pbuf8[1]=0xb1;
  pbuf8[2]=0xb2;
  pbuf8[3]=0xb3;
  pbuf8[4]=0xb4;
  pbuf8[5]=0xb5;
  pbuf8[6]=0xb6;
  pbuf8[7]=0xb7;
  
  return(ptr);
}

// ---------------------------------------------------------------------------------

void PrintFreeMem_Simple(void)
{
  const u32 FreeMemSeg=2*1024;
  
  for(u32 i=1*FreeMemSeg;i<4*1024*1024;i+=FreeMemSeg){
    void *ptr=malloc(i);
    if(ptr==NULL){
      _consolePrintf("FreeMem=%dbyte\n",i-FreeMemSeg);
      break;
    }
    free(ptr); ptr=NULL;
  }
}

void PrintFreeMem_Accuracy_Already(void)
{
  const u32 maxsize=4*1024*1024;
  const u32 segsize=1*1024;
  const u32 count=maxsize/segsize;
  u32 *pptrs=(u32*)malloc(count*4);
  
  if(pptrs==NULL){
    _consolePrintf("PrintFreeMem_Accuracy: Investigation was interrupted. Very low free area.\n");
    return;
  }
  
  u32 FreeMemSize=0;
  u32 MaxBlockSize=0;
  
  for(u32 idx=0;idx<count;idx++){
    u32 size=maxsize-(segsize*idx);
    pptrs[idx]=(u32)malloc(size);
    if(pptrs[idx]!=0){
      FreeMemSize+=size;
      if(MaxBlockSize<size) MaxBlockSize=size;
    }
  }
  
  _consolePrintf("AccuracyFreeMem=%dbyte (MaxBlockSize=%dbyte)\n",FreeMemSize,MaxBlockSize);
  
  for(u32 idx=0;idx<count;idx++){
    if(pptrs[idx]!=0){
      free((void*)pptrs[idx]); pptrs[idx]=0;
    }
  }
  
  free(pptrs); pptrs=NULL;
}

void PrintFreeMem_Accuracy(void)
{
  //if(VerboseDebugLog==false) return;
  PrintFreeMem_Accuracy_Already();
}

void PrintFreeMem(void)
{
  //if(VerboseDebugLog==false) return;
  
  PrintFreeMem_Simple();
  
  //PrintFreeMem_Accuracy();
}

u32 GetMaxMemoryBlockSize(void)
{
  const u32 FreeMemSeg=64*1024;
  for(u32 i=4*1024*1024;i!=FreeMemSeg;i-=FreeMemSeg){
    void *ptr=malloc(i+(8*2));
    if(ptr!=NULL){
      free(ptr); ptr=NULL;
      return(i);
    }
  }
  
  void *ptr=safemalloc(&MM_Temp,FreeMemSeg);
  if(ptr!=NULL){
    safefree(&MM_Temp,ptr); ptr=NULL;
    return(FreeMemSeg);
  }
  
  return(0);
}

void MemClearAllFreeBlocks(void)
{
  const u32 maxsize=4*1024*1024;
  const u32 segsize=512;
  const u32 count=maxsize/segsize;
  u32 *pptrs=(u32*)malloc(count*4);
  
  u32 FreeMemSize=0;
  u32 MaxBlockSize=0;
  
  for(u32 idx=0;idx<count;idx++){
    u32 size=maxsize-(segsize*idx);
    pptrs[idx]=(u32)malloc(size);
    if(pptrs[idx]!=0){
      FreeMemSize+=size;
      if(MaxBlockSize<size) MaxBlockSize=size;
      _consolePrintf("Clear block 0x%08x %dbyte.\n",pptrs[idx],size);
      MemSet32CPU(0,(void*)pptrs[idx],size);
    }
  }
  
  _consolePrintf("AccuracyFreeMem=%dbyte (MaxBlockSize=%dbyte)\n",FreeMemSize,MaxBlockSize);
  
  for(u32 idx=0;idx<count;idx++){
    if(pptrs[idx]!=0){
      free((void*)pptrs[idx]); pptrs[idx]=0;
    }
  }
  
  free(pptrs); pptrs=NULL;
}
