
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "memtool.h"

#include "arm9tcm.h"

#include "fat2.h"
#include "gba_nds_fat.h"
#include "disc_io.h"
#include "shell.h"

// Link from gba_nds_fat.

extern int filesysRootDir;
extern int filesysRootDirClus;
extern int filesysFAT;
extern int filesysSecPerFAT;
extern int filesysNumSec;
extern int filesysData;
extern int filesysBytePerSec;
extern int filesysSecPerClus;
extern int filesysBytePerClus;

extern FS_TYPE filesysType;

#define FAT_ClustToSect(m) (((m-2) * filesysSecPerClus) + filesysData)

extern u8 globalBuffer[BYTE_PER_READ];

extern FAT_FILE openFiles[MAX_FILES_OPEN];

extern u32 wrkDirCluster;
extern int wrkDirSector;
extern int wrkDirOffset;

extern u32 curWorkDirCluster;

extern UnicodeChar lfnNameUnicode[MAX_FILENAME_LENGTH];

extern u32 FAT_LinkFreeCluster(u32 cluster);
extern u32 FAT_NextCluster(u32 cluster);
extern DIR_ENT FAT_DirEntFromPath (const char* path);

// -----------------------

bool FAT2_InitFiles(void)
{
  extern bool FAT_InitFiles(void);
  if(FAT_InitFiles()==false) return(false);
  
  extern u32 FAT_FirstFreeCluster(void);
  if(FAT_FirstFreeCluster()==CLUSTER_EOF) return(false);
  
  return(true);
}

DATA_IN_IWRAM_MainPass void FAT2_Disabled64kClusterMode(void)
{
  if((32*1024/512)<filesysSecPerClus) StopFatalError(12001,"The cluster size that exceeds 32kbyte is not supported.\n");
  
  extern void FAT_Disabled64kClusterMode(void);
  FAT_Disabled64kClusterMode();
}

bool FAT2_FreeFiles(void)
{
  extern bool FAT_FreeFiles(void);
  return(FAT_FreeFiles());
}

static u16 SystemDateTime_Date=0,SystemDateTime_Time=0;

void FAT2_SetSystemDateTime(TFAT2_TIME time)
{
  u16 Date=0,Time=0;
  
  Date|=(time.Year-1980)<<9;
  Date|=time.Month<<5;
  Date|=time.Day;
  
  Time|=time.Hour<<11;
  Time|=time.Minuts<<5;
  Time|=time.Second/2;
  
  SystemDateTime_Date=Date;
  SystemDateTime_Time=Time;
  
  FAT_SetSystemDateTime(Date,Time);
}

u32 FAT2_GetOpenedFileHandlesCount(void)
{
  extern u32 FAT_GetOpenedFileHandlesCount(void);
  return(FAT_GetOpenedFileHandlesCount());
}

// -----------------------

u32 FAT2_GetFATType(void)
{
  switch (filesysType) 
  {
    case FS_UNKNOWN: return 0;
    case FS_FAT12: return 12;
    case FS_FAT16: return 16;
    case FS_FAT32: return 32;
    default: return 0;
  }
  
 return 0;
}

u32 FAT2_ClustToSect(u32 cluster)
{
  return(FAT_ClustToSect(cluster));
}

u32 FAT2_NextCluster(u32 cluster)
{
  return(FAT_NextCluster(cluster));
}

u32 FAT2_GetSecPerClus(void)
{
  return(filesysSecPerClus);
}

// -----------------------

FILE_TYPE FAT2_FindFirstFile (const char **pFilenameAlias)
{
  return(FAT_FindFirstFile(pFilenameAlias));
}

FILE_TYPE FAT2_FindNextFile (const char **pFilenameAlias)
{
  return(FAT_FindNextFile(pFilenameAlias));
}

// -----------------------

static TFAT2_TIME FAT2_FileTimeToTime (u16 fileTime, u16 fileDate)
{
  TFAT2_TIME ft;
  
  ft.Year = (fileDate >> 9) + 1980;    // years since midnight January 1970
  ft.Month = ((fileDate >> 5) & 0xf);  // Months since january
  ft.Day = fileDate & 0x1f;        // Day of the month

  ft.Hour = fileTime >> 11;        // hours past midnight
  ft.Minuts = (fileTime >> 5) & 0x3f;    // minutes past the hour
  ft.Second = (fileTime & 0x1f) * 2;    // seconds past the minute

  return(ft);
}

TFAT2_TIME FAT2_GetFileCreationTime(void)
{
  DIR_ENT *pDirEntry = &((DIR_ENT*) globalBuffer)[wrkDirOffset];
  
  TFAT2_TIME ft=FAT2_FileTimeToTime(pDirEntry->cTime,pDirEntry->cDate);
  
  return(ft);
}

TFAT2_TIME FAT2_GetFileLastWriteTime(void)
{
  DIR_ENT *pDirEntry = &((DIR_ENT*) globalBuffer)[wrkDirOffset];
  
  TFAT2_TIME ft=FAT2_FileTimeToTime(pDirEntry->mTime,pDirEntry->mDate);
  
  return(ft);
}

u32 FAT2_CurEntry_GetFileSize(void)
{
  DIR_ENT *pDirEntry = &((DIR_ENT*) globalBuffer)[wrkDirOffset];
  return(pDirEntry->fileSize);
}

u32 FAT2_GetFirstCluster(void)
{
  DIR_ENT *pDirEntry = &((DIR_ENT*) globalBuffer)[wrkDirOffset];
  u32 firstCluster = pDirEntry->startCluster | (pDirEntry->startClusterHigh << 16);
  return(firstCluster);
}

u8 FAT2_GetAttrib (void)
{
  DIR_ENT *pDirEntry = &((DIR_ENT*) globalBuffer)[wrkDirOffset];
  return(pDirEntry->attrib);
}

const UnicodeChar* FAT2_GetLongFilenameUnicode(void)
{
  if(lfnNameUnicode[0]==0) return(NULL);
  return(lfnNameUnicode);
}

// -----------------------

bool FAT2_chdir_Alias(const char *pPathAlias)
{
  return(FAT_chdir(pPathAlias));
}

bool FAT2_remove(const char *pFilenameAlias)
{
  return(FAT_remove(pFilenameAlias));
}

// -----------------------

FAT_FILE* FAT2_fopen_AliasForRead(const char *pFilenameAlias)
{
  FAT_FILE *pf=FAT_fopen(pFilenameAlias,"r");
  return(pf);
}

FAT_FILE* FAT2_fopen_AliasForWrite(const char *pFilenameAlias)
{
  FAT_FILE *pf=FAT_fopen(pFilenameAlias,"w");
  return(pf);
}

FAT_FILE* FAT2_fopen_AliasForReadWrite(const char *pFilenameAlias)
{
  FAT_FILE *pf=FAT_fopen(pFilenameAlias,"r+");
  return(pf);
}

FAT_FILE* FAT2_fopen_AliasForModify(const char *pFilenameAlias)
{
  FAT_FILE *pf=FAT_fopen(pFilenameAlias,"a");
  return(pf);
}

#include "FAT2_fopen_CreateForWrite_on_CurrentFolder.h"
#include "FAT2_fopen_CurrentForRead.h"

bool FAT2_fclose (FAT_FILE* file)
{
  return(FAT_fclose(file));
}

// -----------------------

u32 FAT2_ftell (FAT_FILE* file)
{
  return(FAT_ftell(file));
}

int FAT2_fseek(FAT_FILE* file, s32 offset, int origin)
{
  return(FAT_fseek(file,offset,origin));
}

// -----------------------

u32 FAT2_fread (void *pBuf, u32 size, u32 count, FAT_FILE* file)
{
  return(FAT_fread(pBuf,size,count,file));
}

u32 FAT2_fread_fast (void *pBuf, u32 size, u32 count, FAT_FILE* file)
{
  return(FAT_fread_fast(pBuf,size,count,file));
}

u32 FAT2_fskip (u32 size, u32 count, FAT_FILE* file)
{
  return(FAT_fskip(size,count,file));
}

u32 FAT2_fwrite (const void *pBuf, u32 size, u32 count, FAT_FILE* file)
{
  return(FAT_fwrite(pBuf,size,count,file));
}

char *FAT2_fgets(char *tgtBuffer, int num, FAT_FILE* file)
{
    return(FAT_fgets(tgtBuffer,num,file));
}

u32 FAT2_fprintf (FAT_FILE* file, const char * format , ...)
{
    static char strbuf[126+1];
          
    va_list args;
          
    va_start( args, format );
    vsnprintf( strbuf, 126, format, args );
    va_end( args );
    return(FAT_fputs(strbuf,file));
}

// -----------------------

u32 FAT2_GetFileSize(FAT_FILE *file)
{
  return(file->length);
}

// -----------------------------

void FAT2_SetSize(FAT_FILE *file, const u32 size, const u8 FillChar)
{
	if(file->length!=0) StopFatalError(12002,"FAT2_SetSize already file size is not zero.\n");
  
	if(file==NULL) StopFatalError(12003,"FAT2_SetSize file handle is NULL.\n");
	if(file->inUse==false) StopFatalError(12004,"FAT2_SetSize file handle is not opened.\n");
	if(file->write==false) StopFatalError(12005,"FAT2_SetSize file handle is not write mode.\n");
	
	disc_SystemWriteSector_SetWriteCache(true);
  
	u32 SecPerClus=FAT2_GetSecPerClus();
	u32 blocksize=SecPerClus*512;
  
	u8 *pblockbuf=(u8*)safemalloc(&MM_Temp,blocksize);
	if(pblockbuf==NULL) StopFatalError(12006,"FAT2_SetSize block memory overflow.\n");
	  
	MemSet32CPU((FillChar<<0)|(FillChar<<8)|(FillChar<<16)|(FillChar<<24),pblockbuf,blocksize);
	  
	u32 ClusListMax=(size+blocksize-1)/blocksize;
	u32 *pClusList=(u32*)safemalloc(&MM_Temp,ClusListMax*4);
	if(pClusList==NULL) StopFatalError(12007,"FAT2_SetSize Cluster list memory overflow.\n");
	  
	MemSet32CPU(0,pClusList,ClusListMax*4);
	  
	u32 ClusListCount=0;
	  
	{
		if(VerboseDebugLog==true) _consolePrint("FAT2_SetSize: Reserve cluster.\n");
    
		u32 curClus=file->curClus;
		u32 reqsize=size;
    
		if(reqsize!=0){
			pClusList[ClusListCount++]=curClus;
			if(blocksize<=reqsize){
				file->length+=blocksize;
				reqsize-=blocksize;
			}else{
				file->length+=reqsize;
				reqsize=0;
			}
		}
    
		while(reqsize!=0){
			curClus=FAT_LinkFreeCluster(curClus);
			if((curClus==0)||(curClus==CLUSTER_FREE)){
				FAT_fclose(file);
				disc_SystemWriteSector_SetWriteCache(false);
				StopFatalError(12008,"FAT2_SetSize disk full.\n");
			}
			pClusList[ClusListCount++]=curClus;
			if(blocksize<=reqsize){
				file->length+=blocksize;
				reqsize-=blocksize;
			}else{
				file->length+=reqsize;
				reqsize=0;
			}
		}
	}
	
	if(FillChar != 0xAB) {
		FAT_freopen(file);
	
		{
			if(VerboseDebugLog==true) _consolePrint("FAT2_SetSize: Initial data area.\n");
	    
			for(u32 idx=0;idx<ClusListCount;idx++){
				u32 curClus=pClusList[idx];
				if((curClus==0)||(curClus==CLUSTER_FREE)){
					FAT_fclose(file);
					StopFatalError(12009,"FAT2_SetSize Initial error.\n");
				}
				u32 curSec=FAT_ClustToSect(curClus);
				if(curSec==0){
					FAT_fclose(file);
					StopFatalError(12010,"FAT2_SetSize Initial error.\n");
				}
				disc_WriteSectors(curSec,blocksize/512,pblockbuf);
			}	
		}
	}
	
	safefree(&MM_Temp,pblockbuf); pblockbuf=NULL;
	  
	safefree(&MM_Temp,pClusList); pClusList=NULL;
	  
	disc_SystemWriteSector_SetWriteCache(false);
    
	if(VerboseDebugLog==true) _consolePrint("FAT2_SetSize: Proceeded.\n");
}

// -----------------------

bool FAT2_Move(const char *pSrcPath,const char *pDstPath)
{
  extern bool FAT_Move(const char *pSrcPath,const char *pDstPath);
  return(FAT_Move(pSrcPath,pDstPath));
}

bool FAT2_DeleteFile(const char *pSrcPath)
{
  extern bool FAT_DeleteFile(const char *pSrcPath);
  return(FAT_DeleteFile(pSrcPath));
}
  
bool FAT2_mkdir(const char *ppath)
{
  extern int FAT_mkdir (const char* path);
  if(FAT_mkdir(ppath)!=0) return(false);
  return(true);
}

void FAT2_ShowDirectoryEntryList(const char *pPath)
{
  extern void FAT_ShowDirectoryEntryList(const char *pPath);
  FAT_ShowDirectoryEntryList(pPath);
}
