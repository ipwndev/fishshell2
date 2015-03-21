
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"

#include "sndfont.h"

#include "memtool.h"
#include "fat2.h"
#include "shell.h"

#include "sndfont_dfs.h"

void SndFont_Open(void)
{
  _consolePrint("SndFont: Create.\n");
  
  const char *pfn="sc88proe.bin";
  FAT_FILE *pf=Shell_FAT_fopen_MIDIData(pfn);
  if(pf==NULL) StopFatalError(15601,"SndFont_Open: File not found. [%s]\n",pfn);
  
  SndFontDFS_Init(pf);
  FAT2_fclose(pf);
  
  _consolePrint("SndFont: Initialized.\n");
}

void SndFont_Close(void)
{
  SndFontDFS_Free();
}

void SndFont_SetOffset(u32 ofs)
{
  SndFontDFS_SetOffset(ofs);
}

u32 SndFont_Read16bit(void *_pbuf,u32 size)
{
//  _consolePrintf("r32:%d, %d\n",SndFont_DFS_GetOffset(),size);
  return(SndFontDFS_Read16bit(_pbuf,size));
}

u32 SndFont_Get32bit(void)
{
  u32 tmp;
  SndFontDFS_Read16bit(&tmp,4);
  return(tmp);
}

