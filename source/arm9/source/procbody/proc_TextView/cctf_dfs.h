
#ifndef _cctf_dfs_h
#define _cctf_dfs_h

#include "fat2.h"

extern void CTF_DFS_Init(FAT_FILE *FileHandle);
extern void CTF_DFS_Free(void);
extern void CTF_DFS_SetAttribute(u32 _DataTopOffset,u32 _Size);
extern u32 CTF_DFS_GetSize(void);
extern void CTF_DFS_SetOffset(u32 ofs);
extern u32 CTF_DFS_Read32bit(void *_pbuf,u32 size);

#endif
