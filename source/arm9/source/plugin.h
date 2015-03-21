
#ifndef plugin_h
#define plugin_h

#include <stdio.h>

#include <nds.h>

#include "memtool.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  void (*_consolePrint)(const char *str);
  void (*_consolePrintf)(const char* format, ...);
  void (*_consolePrintSet)(int x, int y);
  void (*ShowLogHalt)(void);
  void (*MWin_ProgressShow)(const char *TitleStr,s32 Max);
  void (*MWin_ProgressSetPos)(s32 Position);
  void (*MWin_ProgressHide)(void);
  
  void (*MemCopy8CPU)(const void *src,void *dst,u32 len);
  void (*MemCopy16CPU)(const void *src,void *dst,u32 len);
  void (*MemCopy32CPU)(const void *src,void *dst,u32 len);
  void (*MemSet16CPU)(vu16 v,void *dst,u32 len);
  void (*MemSet32CPU)(u32 v,void *dst,u32 len);
  
  void *(*safemalloc)(TMM *pMM,int size);
  void (*safefree)(TMM *pMM,void *ptr);
  void *(*safecalloc)(TMM *pMM,int nmemb, int size);
  void *(*saferealloc)(TMM *pMM,void *ptr, int size);
  
  int (*rand)(void);
  
  u32 (*fread)(void *buf, u32 size, u32 n, int fp);
  u32 (*fskip)(u32 size, u32 n, int fp);
  int (*fseek)(int fp, u32 offset, int origin);
  long (*ftell)(int fp);
  
  int (*sprintf)(char *str, const char *format, ...);
  int (*snprintf)(char *str, size_t size, const char *format, ...);
  
  void *(*memchr)(const void *buf, int ch, size_t n);
  int (*memcmp)(const void *buf1, const void *buf2,size_t n);
  void *(*memcpy)(void *buf1, const void *buf2, size_t n);
  void *(*memmove)(void *buf1, const void *buf2, size_t n);
  void *(*memset)(void *buf, int ch, size_t n);
  
  int (*abs)(int j);
  long int (*labs)(long int j);
  long long int (*llabs)(long long int j);
  double (*fabs)(double x);
  float (*fabsf)(float x);
  
  double (*atof)(const char *nptr);
  int (*atoi)(const char *nptr);
  long (*atol)(const char *nptr);
  long long (*atoll)(const char *nptr);
  
  char *(*strcat)(char *dest, const char *src);
  char *(*strchr)(const char *s, int c);
  int (*strcmp)(const char *s1, const char *s2);
  int (*strcoll)(const char *s1, const char *s2);
  char *(*strcpy)(char *dest, const char *src);
  size_t (*strcspn)(const char *s, const char *reject);
  size_t (*strlen)(const char *s);
  char *(*strncat)(char *dest, const char *src, size_t n);
  int (*strncmp)(const char *s1, const char *s2, size_t n);
  char *(*strncpy)(char *dest, const char *src, size_t n);
  char *(*strpbrk)(const char *s, const char *accept);
  char *(*strrchr)(const char *s, int c);
  char *(*strsep)(char **stringp, const char *delim);
  size_t (*strspn)(const char *s, const char *accept);
  char *(*strstr)(const char *haystack, const char *needle);
  char *(*strtok)(char *s, const char *delim);
  size_t (*strxfrm)(char *dest, const char *src, size_t n);
  
  u16 d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12,d13,d14,d15;
} TPlugin_StdLib;

typedef u16 UnicodeChar;

#ifdef PluginMode_Image

typedef struct {
  bool (*Start)(int FileHandle);
  void (*Free)(void);
  
  void (*GetBitmap24)(u32 LineY,u8 *pBM);
  
  s32 (*GetWidth)(void);
  s32 (*GetHeight)(void);
  
  int (*GetInfoIndexCount)(void);
  bool (*GetInfoStrL)(int idx,char *str,int len);
  bool (*GetInfoStrW)(int idx,UnicodeChar *str,int len);
  bool (*GetInfoStrUTF8)(int idx,char *str,int len);
} TPlugin_ImageLib;

#endif

#ifdef PluginMode_Sound

typedef struct {
  bool (*Start)(int FileHandle);
  void (*Free)(void);
  bool (*isComplexDecoder)(void);
  
  u32 (*Update)(u32 *plrbuf);
  
  s32 (*GetPosMax)(void);
  s32 (*GetPosOffset)(void);
  void (*SetPosOffset)(s32 ofs);
  
  u32 (*GetChannelCount)(void);
  u32 (*GetSampleRate)(void);
  u32 (*GetSamplePerFrame)(void);
  
  int (*GetInfoIndexCount)(void);
  bool (*GetInfoStrL)(int idx,char *str,int len);
  bool (*GetInfoStrW)(int idx,UnicodeChar *str,int len);
  bool (*GetInfoStrUTF8)(int idx,char *str,int len);
} TPlugin_SoundLib;

#endif

extern const TPlugin_StdLib *pStdLib;

extern bool LoadLibrary(const TPlugin_StdLib *_pStdLib,TMM *pMM);
extern void FreeLibrary(void);
extern int QueryInterfaceLibrary(void);

enum EPluginType {EPT_None=0,EPT_Image=1,EPT_Sound=2};

#ifdef __cplusplus
}
#endif

#endif

