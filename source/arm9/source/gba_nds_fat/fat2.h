
#ifndef FAT2_H
#define FAT2_H
    
// File Constants
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

// Directory Constants
typedef enum {FT_NONE, FT_FILE, FT_DIR} FILE_TYPE;

// Open file information structure
typedef struct
{
  u32 firstCluster;
  u32 length;
  u32 curPos;
  u32 curClus;      // Current cluster to read from
  int curSect;      // Current sector within cluster
  int curByte;      // Current byte within sector
  char readBuffer[512];  // Buffer used for unaligned reads
  u32 appClus;      // Cluster to append to
  int appSect;      // Sector within cluster for appending
  int appByte;      // Byte within sector for appending
  bool read;  // Can read from file
  bool write;  // Can write to file
  bool append;// Can append to file
  bool inUse;  // This file is open
  u32 dirEntSector;  // The sector where the directory entry is stored
  int dirEntOffset;  // The offset within the directory sector
  char tmpfna[16];
} FAT_FILE;

typedef struct {
  u32 Year,Month,Day;
  u32 Hour,Minuts,Second;
} TFAT2_TIME;

typedef u16 UnicodeChar;

extern u32 GetSecPerCls(void);
extern u32 GetFisrtCls(void);

extern bool FAT2_InitFiles(void);
extern void FAT2_Disabled64kClusterMode(void);
extern bool FAT2_FreeFiles(void);
extern void FAT2_SetSystemDateTime(TFAT2_TIME time);
extern u32 FAT2_GetOpenedFileHandlesCount(void);

extern u32 FAT2_GetFATType(void);
extern u32 FAT2_ClustToSect(u32 cluster);
extern u32 FAT2_NextCluster(u32 cluster);
extern u32 FAT2_GetSecPerClus(void);

extern FILE_TYPE FAT2_FindFirstFile (const char **pFilenameAlias);
extern FILE_TYPE FAT2_FindNextFile (const char **pFilenameAlias);

extern TFAT2_TIME FAT2_GetFileCreationTime (void);
extern TFAT2_TIME FAT2_GetFileLastWriteTime (void);
extern u32 FAT2_CurEntry_GetFileSize(void);
extern u32 FAT2_GetFirstCluster(void);
extern u8 FAT2_GetAttrib(void);
extern const UnicodeChar* FAT2_GetLongFilenameUnicode(void);

extern bool FAT2_chdir_Alias(const char *pPathAlias);
extern bool FAT2_remove(const char *pFilenameAlias);

extern FAT_FILE* FAT2_fopen_AliasForRead(const char *pFilenameAlias);
extern FAT_FILE* FAT2_fopen_AliasForWrite(const char *pFilenameAlias);
extern FAT_FILE* FAT2_fopen_AliasForReadWrite(const char *pFilenameAlias);
extern FAT_FILE* FAT2_fopen_AliasForModify(const char *pFilenameAlias);
extern FAT_FILE* FAT2_fopen_CurrentForRead(void);
extern FAT_FILE* FAT2_fopen_CreateForWrite_on_CurrentFolder(const char *pFilenameAlias,const u16 *pFilenameUnicode);
extern bool FAT2_fclose (FAT_FILE* file);

extern u32 FAT2_ftell (FAT_FILE* file);
extern int FAT2_fseek(FAT_FILE* file, s32 offset, int origin);

extern u32 FAT2_fread (void *pBuf, u32 size, u32 count, FAT_FILE* file);
extern u32 FAT2_fread_fast (void *pBuf, u32 size, u32 count, FAT_FILE* file);
extern u32 FAT2_fskip (u32 size, u32 count, FAT_FILE* file);
extern u32 FAT2_fwrite (const void *pBuf, u32 size, u32 count, FAT_FILE* file);
extern char *FAT2_fgets(char *tgtBuffer, int num, FAT_FILE* file);
extern u32 FAT2_fprintf (FAT_FILE* file, const char * format , ...);


extern u32 FAT2_GetFileSize(FAT_FILE *file);

extern void FAT2_SetSize(FAT_FILE *file, const u32 size, const u8 FillChar);

extern bool FAT2_Move(const char *pSrcPath,const char *pDstPath);
extern bool FAT2_DeleteFile(const char *pSrcPath);
extern bool FAT2_mkdir(const char *ppath); // Upper case ALIAS only. no check.

extern void FAT2_ShowDirectoryEntryList(const char *pPath);

extern FAT_FILE *CheckFile_in_Folder(char *CheckName,char*extname,char*Folder,uint32 createSize);

#endif
