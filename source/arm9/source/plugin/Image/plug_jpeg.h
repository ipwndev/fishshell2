
#ifndef _plug_jpeg_h
#define _plug_jpeg_h

#include "fat2.h"

extern bool PlugJpeg_Start(FAT_FILE *_FileHandle,bool EnabledAutoFitting);
extern void PlugJpeg_Free(void);
extern void PlugJpeg_GetBitmap24(u32 LineY,u8 *pBM);
extern s32 PlugJpeg_GetWidth(void);
extern s32 PlugJpeg_GetHeight(void);
extern int PlugJpeg_GetInfoIndexCount(void);
extern bool PlugJpeg_GetInfoStrL(int idx,char *str,int len);
extern bool PlugJpeg_GetInfoStrW(int idx,UnicodeChar *str,int len);
extern bool PlugJpeg_GetInfoStrUTF8(int idx,char *str,int len);

#endif
