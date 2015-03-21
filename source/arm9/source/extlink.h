
#ifndef extlink_h
#define extlink_h

extern void ExtLink_Init(void);
extern void ExtLink_Free(void);
extern u32 ExtLink_GetTargetIndex(u32 Ext32);
extern const UnicodeChar* ExtLink_GetNDSFullPathFilenameUnicode(u32 idx);

#endif

