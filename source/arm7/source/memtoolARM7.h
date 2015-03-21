
#ifndef memtoolARM7_h
#define memtoolARM7_h

#include <nds.h>
	
#ifdef __cplusplus
extern "C" {
#endif
	
#define ALIGN32 __attribute__ ((align(4)))

void SetMemoryMode(bool _MappedVRAM);
void SetMemoryMode_End(void);

void* safemalloc(int size);
void safefree(void *ptr);

//void MemSet16CPU(vu16 v,void *dst,u32 len);
//void MemSet32CPU(u32 v,void *dst,u32 len);
void MemCopy16DMA3(void *src,void *dst,u32 len);
//void MemCopy32DMA3(void *src,void *dst,u32 len);
void MemSet16DMA3(u16 v,void *dst,u32 len);
//void MemSet32DMA3(u32 v,void *dst,u32 len);

#define MemCopy32swi256bit(src,dst,len) swiFastCopy(src,dst,COPY_MODE_COPY | ((len)>>2) );

u32 PrintFreeMem(void);

#ifdef __cplusplus
}
#endif

#endif

