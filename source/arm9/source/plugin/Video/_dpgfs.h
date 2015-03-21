
#ifndef _DPGFS_H
#define _DPGFS_H

#include "fat2.h"

extern void DPGFS_Init(FAT_FILE *pFileHandle);
extern void DPGFS_Free(void);
extern void DPGFS_Movie_SetAttribute(u32 _DataTopOffset,u32 _Size);
extern void DPGFS_Audio_SetAttribute(u32 _DataTopOffset,u32 _Size);
extern u32 DPGFS_Movie_GetSize(void);
extern u32 DPGFS_Audio_GetSize(void);
extern void DPGFS_Movie_SetOffset(u32 ofs);
extern void DPGFS_Audio_SetOffset(u32 ofs);
extern u32 DPGFS_Audio_GetOffset(void);
extern u32 DPGFS_Movie_Read32bit(void *_pbuf,u32 size);
extern u32 DPGFS_Audio_Read32bit(void *_pbuf,u32 size);

#endif
