#pragma once


#include "fat2.h"

extern void SndEff_DFS_Init(FAT_FILE *FileHandle);
extern void SndEff_DFS_Free(void);
extern void SndEff_DFS_SetAttribute(u32 _DataTopOffset,u32 _Size);
extern u32 SndEff_DFS_GetSize(void);
extern void SndEff_DFS_SetOffset(u32 ofs);
extern u32 SndEff_DFS_Read32bit(void *_pbuf,u32 size);

