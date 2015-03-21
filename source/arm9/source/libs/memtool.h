
#ifndef memtool_h
#define memtool_h

extern void DCache_FlushRangeOverrun(const void *v,u32 size);
extern void DCache_CleanRangeOverrun(const void *v,u32 size);

extern void MemCopy8CPU(const void *src,void *dst,u32 len);
extern void MemCopy16CPU(const void *src,void *dst,u32 len);
extern void MemCopy32CPU(const void *src,void *dst,u32 len);
extern void MemSet8CPU(u8 v,void *dst,u32 len);
extern void MemSet16CPU(u16 v,void *dst,u32 len);
extern void MemSet32CPU(u32 v,void *dst,u32 len);

// ---------------------------------------------------------------------

typedef struct {
  u32 adr,size;
  const char *filename;
  int linenum;
  const char *funcname;
  bool locked;
} TMM_List;
  
typedef struct {
  const char *pName;
  u32 ListCount;
  TMM_List *pLists;
} TMM;

extern TMM MM_Temp,MM_System,MM_SystemAfter,MM_Skin,MM_DLLImage,MM_DLLSound,MM_DLLDPG,MM_PlayList,MM_Process;

extern void MM_Init(void);
extern void MM_ShowAllocated(TMM *pMM);
extern void MM_Compact(void);
extern void MM_CheckMemoryLeak(TMM *pMM);
extern void MM_CheckOverRange(void);
extern void MM_MemoryLock(TMM *pMM,void *ptr);
extern void MM_MemoryUnlock(TMM *pMM,void *ptr);

// 確保されたメモリを強制的に全て解放する。
// 推奨されないけど、mikmodがメモリリークするので仕方なく。
extern void MM_ExecuteForceAllFree(TMM *pMM);

extern bool (*safemalloc_CallBack_RequestFreeMemory_PlugSound)(void);
extern bool (*safemalloc_CallBack_RequestFreeMemory_PlugImage)(void);

extern void *__safemalloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,int size);
extern void *__safemalloc_chkmem__(TMM *pMM,const char *filename,int linenum,const char *funcname,int size);
extern void __safefree__(TMM *pMM,const char *filename,int linenum,const char *funcname,void *ptr);
extern void *__safecalloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,int nmemb, int size);
extern void *__saferealloc__(TMM *pMM,const char *filename,int linenum,const char *funcname,void *ptr, int size);

#define safemalloc(pMM,size) __safemalloc__(pMM,__FILE__,__LINE__,__FUNCTION__,size)
#define safemalloc_chkmem(pMM,size) __safemalloc_chkmem__(pMM,__FILE__,__LINE__,__FUNCTION__,size)
#define safefree(pMM,ptr) __safefree__(pMM,__FILE__,__LINE__,__FUNCTION__,ptr)
#define safecalloc(pMM,nmemb,size) __safecalloc__(pMM,__FILE__,__LINE__,__FUNCTION__,nmemb,size)
#define saferealloc(pMM,ptr,size) __saferealloc__(pMM,__FILE__,__LINE__,__FUNCTION__,ptr,size)

// ---------------------------------------------------------------------

extern void PrintFreeMem_Simple(void);
extern void PrintFreeMem_Accuracy(void);
extern void PrintFreeMem(void);

extern u32 GetMaxMemoryBlockSize(void);
extern void MemClearAllFreeBlocks(void);

#endif

