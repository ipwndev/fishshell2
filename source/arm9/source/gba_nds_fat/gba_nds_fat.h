/*
  gba_nds_fat.h
  By chishm (Michael Chisholm)

  Routines for reading a compact flash card
  using the GBA Movie Player or M3.

  Some FAT routines are based on those in fat.c, which
  is part of avrlib by Pascal Stang.

  This software is completely free. No warranty is provided.
  If you use it, please give me credit and email me about your
  project at chishm@hotmail.com

  See gba_nds_fat.txt for help and license details.
*/

//---------------------------------------------------------------

#ifndef _GBA_NDS_FAT_INCLUDED
#define _GBA_NDS_FAT_INCLUDED

//---------------------------------------------------------------
// Customisable features

// Maximum number of files open at once
// Increase this to open more files, decrease to save memory
#define MAX_FILES_OPEN  8

// Allow file writing
// Disable this to remove file writing support
#define CAN_WRITE_TO_DISC

//---------------------------------------------------------------
// Platform specific includes

// When compiling for NDS, make sure NDS is defined
// If using this on the ARM7, you will need to explicitly define NDS
#ifdef ARM9
 #ifndef NDS
  #define NDS 1
 #endif
#endif

#ifdef NDS
 #include <nds/jtypes.h>
#else
 #include "gba_types.h"
#endif

//---------------------------------------------------------------
//---------------------------------------------------------------

// Take care of packing for GCC - it doesn't obey pragma pack()
// properly for ARM targets.
#ifdef __GNUC__
 #define __PACKED __attribute__ ((__packed__))
#else
 #define __PACKED 
 #pragma pack(1)
#endif

//-----------------------------------------------------------------
// long file name constants
#define LFN_END 0x40
#define LFN_DEL 0x80

//-----------------------------------------------------------------
// FAT constants
#define CLUSTER_EOF_16 0xFFFF
#define CLUSTER_EOF 0x0FFFFFFF
#define CLUSTER_FREE 0x0000
#define CLUSTER_FIRST 0x0002

#define FILE_LAST 0x00
#define FILE_FREE 0xE5

#define ATTRIB_ARCH  0x20
#define ATTRIB_DIR  0x10
#define ATTRIB_LFN  0x0F
#define ATTRIB_VOL  0x08
#define ATTRIB_HID  0x02
#define ATTRIB_SYS  0x04
#define ATTRIB_RO  0x01

#define FAT16_ROOT_DIR_CLUSTER 0x00

//---------------------------------------------------------------
// Important constants

// Directory entry - must be packed
typedef struct
{
  __PACKED  u8  name[8];
  __PACKED  u8  ext[3];
  __PACKED  u8  attrib;
  __PACKED  u8  reserved;
  __PACKED  u8  cTime_ms;
  __PACKED  u16  cTime;
  __PACKED  u16  cDate;
  __PACKED  u16  aDate;
  __PACKED  u16  startClusterHigh;
  __PACKED  u16  mTime;
  __PACKED  u16  mDate;
  __PACKED  u16  startCluster;
  __PACKED  u32  fileSize;
}  DIR_ENT;

// Long file name directory entry - must be packed
typedef struct
{
  __PACKED  u8 ordinal;  // Position within LFN
  __PACKED  u16 char0;  
  __PACKED  u16 char1;
  __PACKED  u16 char2;
  __PACKED  u16 char3;
  __PACKED  u16 char4;
  __PACKED  u8 flag;  // Should be equal to ATTRIB_LFN
  __PACKED  u8 reserved1;  // Always 0x00
  __PACKED  u8 checkSum;  // Checksum of short file name (alias)
  __PACKED  u16 char5;
  __PACKED  u16 char6;
  __PACKED  u16 char7;
  __PACKED  u16 char8;
  __PACKED  u16 char9;
  __PACKED  u16 char10;
  __PACKED  u16 reserved2;  // Always 0x0000
  __PACKED  u16 char11;
  __PACKED  u16 char12;
}  DIR_ENT_LFN;

#define MAX_FILENAME_LENGTH 256  // Maximum LFN length. Don't change this one

// File Constants
#ifndef EOF
#define EOF -1
#endif

// Filesystem type
typedef enum {FS_UNKNOWN, FS_FAT12, FS_FAT16, FS_FAT32} FS_TYPE;

#include "gba_nds_fat_move_def.h"
#include "gba_nds_fat_deletefile_def.h"
//-----------------------------------------------------------------

void FAT_SetSystemDateTime(u16 Date,u16 Time);

u32 FAT_GetOpenedFileHandlesCount(void);

/*-----------------------------------------------------------------
FAT_InitFiles
Reads the FAT information from the CF card.
You need to call this before reading any files.
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_InitFiles (void);

/*-----------------------------------------------------------------
FAT_FreeFiles
Closes all open files then resets the CF card.
Call this before exiting back to the GBAMP
bool return OUT: true if successful.
-----------------------------------------------------------------*/
bool FAT_FreeFiles (void);

/*-----------------------------------------------------------------
FAT_GetAlias
Get the alias (short name) of the last file or directory entry read
  using GetDirEntry. Works for FindFirstFile and FindNextFile
char* alias OUT: will be filled with the alias (short filename),
  should be at least 13 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
bool FAT_GetAlias (char* alias);

/*-----------------------------------------------------------------
FAT_GetLongFilename
Get the long name of the last file or directory retrived with 
  GetDirEntry. Also works for FindFirstFile and FindNextFile
char* filename: OUT will be filled with the filename, should be at
  least 256 bytes long
bool return OUT: return true if successful
-----------------------------------------------------------------*/
//bool FAT_GetLongFilename (char* filename);

/*-----------------------------------------------------------------
FAT_GetFileSize
Get the file size of the last file found or openned.
This idea is based on a modification by MoonShine
u32 return OUT: the file size
-----------------------------------------------------------------*/
u32 FAT_GetFileSize (void);

/*-----------------------------------------------------------------
FAT_GetFileCluster
Get the first cluster of the last file found or openned.
u32 return OUT: the file start cluster
-----------------------------------------------------------------*/
u32 FAT_GetFileCluster (void);

/*-----------------------------------------------------------------
FAT_FindNextFile
Gets the name of the next directory entry
  (can be a file or subdirectory)
char* filename: OUT filename, must be at least 13 chars long
FILE_TYPE return: OUT returns FT_NONE if failed, 
  FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindNextFile (const char** filename);

/*-----------------------------------------------------------------
FAT_FindFirstFile
Gets the name of the first directory entry and resets the count
  (can be a file or subdirectory)
char* filename: OUT filename, must be at least 13 chars long
FILE_TYPE return: OUT returns FT_NONE if failed, 
  FT_FILE if it found a file and FT_DIR if it found a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FindFirstFile (const char** filename);

/*-----------------------------------------------------------------
FAT_FileExists
Returns the type of file 
char* filename: IN filename of the file to look for
FILE_TYPE return: OUT returns FT_NONE if there is now file with 
  that name, FT_FILE if it is a file and FT_DIR if it is a directory
-----------------------------------------------------------------*/
FILE_TYPE FAT_FileExists (const char* filename);

/*-----------------------------------------------------------------
FAT_GetFileSystemType
FS_TYPE return: OUT returns the current file system type
-----------------------------------------------------------------*/
FS_TYPE FAT_GetFileSystemType (void);

/*-----------------------------------------------------------------
FAT_GetFileSystemTotalSize
u32 return: OUT returns the total disk space (used + free)
-----------------------------------------------------------------*/
u32 FAT_GetFileSystemTotalSize (void);

/*
FAT_GetVolumeLabel
Get volume label
*/
char* FAT_GetVolumeLabel (void);

/*-----------------------------------------------------------------
FAT_chdir
Changes the current working directory
const char* path: IN null terminated string of directory separated by 
  forward slashes, / is root
bool return: OUT returns true if successful
-----------------------------------------------------------------*/
bool FAT_chdir (const char* path);

char* FAT_getdir(void);

//-----------------------------------------------------------------
// File functions

/*-----------------------------------------------------------------
FAT_fopen(filename, mode)
Opens a file
const char* path: IN null terminated string of filename and path 
  separated by forward slashes, / is root
const char* mode: IN mode to open file in
  Supported modes: "r", "r+", "w", "w+", "a", "a+", don't use
  "b" or "t" in any mode, as all files are openned in binary mode
FAT_FILE* return: OUT handle to open file, returns -1 if the file 
  couldn't be openned
-----------------------------------------------------------------*/
FAT_FILE* FAT_fopen(const char* path, const char* mode);

/*-----------------------------------------------------------------
FAT_fclose(file)
Closes a file
FAT_FILE* file: IN handle of the file to close
bool return OUT: true if successful, false if not
-----------------------------------------------------------------*/
bool FAT_fclose (FAT_FILE* file);

/*-----------------------------------------------------------------
FAT_ftell(file)
Returns the current position in a file
FAT_FILE* file: IN handle of an open file
u32 OUT: Current position
-----------------------------------------------------------------*/
u32 FAT_ftell (FAT_FILE* file);

/*-----------------------------------------------------------------
FAT_fseek(file, offset, origin)
Seeks to specified byte position in file
int file: IN handle of an open file
u32 offset IN: position to seek to, relative to origin
int origin IN: origin to seek from
int OUT: Returns 0 if successful, -1 if not
-----------------------------------------------------------------*/
int FAT_fseek(FAT_FILE* file, s32 offset, int origin);

/*-----------------------------------------------------------------
FAT_fread(buffer, size, count, file)
Reads in length number of bytes into buffer from file, starting
  from current position. It then sets the current position to the
  byte after the last byte read. If it reaches the end of file
  before filling the buffer then it stops reading.
void* buffer OUT: Pointer to buffer to fill. Should be at least as
  big as the number of bytes required
u32 size IN: size of each item to read
u32 count IN: number of items to read
FAT_FILE* file IN: Handle of an open file
u32 OUT: returns the actual number of bytes read
-----------------------------------------------------------------*/

// auto detect alignment
u32 FAT_fread (void* buffer, u32 size, u32 count, FAT_FILE* file);

// fast request 16bit aligned file position and write buffer
u32 FAT_fread_fast (void* buffer, u32 size, u32 count, FAT_FILE* file);

// auto detect alignment
u32 FAT_fskip (u32 size, u32 count, FAT_FILE* file);

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fwrite(buffer, size, count, file)
Writes size * count bytes into file from buffer, starting
  from current position. It then sets the current position to the
  byte after the last byte written. If the file was openned in 
  append mode it always writes to the end of the file.
const void* buffer IN: Pointer to buffer containing data. Should be
  at least as big as the number of bytes to be written.
u32 size IN: size of each item to write
u32 count IN: number of items to write
FAT_FILE* file IN: Handle of an open file
u32 OUT: returns the actual number of bytes written
-----------------------------------------------------------------*/
u32 FAT_fwrite (const void* buffer, u32 size, u32 count, FAT_FILE* file);
#endif

/*-----------------------------------------------------------------
FAT_feof(file)
Returns true if the end of file has been reached
FAT_FILE* file IN: Handle of an open file
bool return OUT: true if EOF, false if not
-----------------------------------------------------------------*/
bool FAT_feof(FAT_FILE* file);

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_remove (path)
Deletes the file or empty directory sepecified in path
const char* path IN: Path of item to delete
int return OUT: zero if successful, non-zero if not
-----------------------------------------------------------------*/
int FAT_remove (const char* path);
#endif

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_mkdir (path)
Makes a new directory, so long as no other directory or file has 
  the same name.
const char* path IN: Path and filename of directory to make
int return OUT: zero if successful, non-zero if not
-----------------------------------------------------------------*/
int FAT_mkdir (const char* path);
#endif

/*-----------------------------------------------------------------
FAT_fgetc (handle)
Gets the next character in the file
FAT_FILE* file IN: Handle of open file
bool return OUT: character if successful, EOF if not
-----------------------------------------------------------------*/
char FAT_fgetc (FAT_FILE* file);

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fputc (character, handle)
Writes the given character into the file
char c IN: Character to be written
FAT_FILE* handle IN: Handle of open file
bool return OUT: character if successful, EOF if not
-----------------------------------------------------------------*/
char FAT_fputc (char c, FAT_FILE* file);
#endif

/*-----------------------------------------------------------------
FAT_fgets (char *tgtBuffer, int num, FAT_FILE* file)
Gets a up to num bytes from file, stopping at the first
 newline.

CAUTION: does not do strictly streaming. I.e. it's 
 reading more then needed bytes and seeking back.
 shouldn't matter for random access 

char *tgtBuffer OUT: buffer to write to
int num IN: size of target buffer
FAT_FILE* file IN: Handle of open file
bool return OUT: character if successful, EOF if not

  Written by MightyMax
  Modified by Chishm - 2005-11-17
  * Added check for unix style text files
  * Removed seek when no newline is found, since it isn't necessary
-------------------------------------------------------------------*/
char *FAT_fgets(char *tgtBuffer, int num, FAT_FILE* file) ;

#ifdef CAN_WRITE_TO_DISC
/*-----------------------------------------------------------------
FAT_fputs (const char *string, FAT_FILE* file)
Writes string to file, excluding end of string character
const char *string IN: string to write
FAT_FILE* file IN: Handle of open file
bool return OUT: number of characters written if successful,
  EOF if not

  Written by MightyMax
  Modified by Chishm - 2005-11-17
  * Uses FAT_FILE instead of int
  * writtenBytes is now u32 instead of int
-------------------------------------------------------------------*/
int FAT_fputs (const char *string, FAT_FILE* file);
#endif

#ifdef CAN_WRITE_TO_DISC
int FAT_fprintf (FAT_FILE* file, const char * format , ...);
#endif

//------------------------------------------------------------------
//------------------------------------------------------------------

void FAT_freopen(FAT_FILE* file);

void FAT_ShowDirectoryEntryList(const char *pPath);

#endif  // ifndef _GBA_NDS_FAT

