
#ifndef extmem_h
#define extmem_h

typedef struct {
  const char *pID;
  u32 MaxSizeByte;
  u32 TopAddr,CurAddr,TermAddr;
  u32 AllocateCount;
} Textmem;

extern Textmem extmem;

extern void extmem_Init(void);
extern void extmem_Start(void);
extern void* extmem_malloc(u32 size);
extern void extmem_ShowMemoryInfo(void);
extern void extmem_ShowMallocInfo(void);
extern const char* extmem_GetID(void);
extern u32 extmem_GetPureMaxSizeByte(void);
extern u32 extmem_GetMaxSizeByte(void);
extern u32 extmem_GetTopAddr(void);

#endif
