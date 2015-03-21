
#ifndef shell_h
#define shell_h

#include "unicode.h"
#include "fat2.h"

#define MaxFilenameLength (256)

#define DefaultDataPath "/FISHELL2"
#define DefaultRootPath DefaultDataPath
#define DefaultInternalPath DefaultDataPath "/INTERNAL"
#define DefaultSkinPath DefaultDataPath "/SKINS"
#define ResetMSEPath DefaultDataPath "/RESETMSE"

#define DefaultLanguageSelectNDSFilename DefaultDataPath "/LANGUAGE.DAT"
#define DefaultLanguageSetFullPathFilename DefaultDataPath "/LANGUAGE.SET"
#define DefaultLanguageDataPath DefaultDataPath "/LANGUAGE"

#define INIFilename "FISHELL2.INI"
#define DataFilename "FISHELL2.DAT"
#define LogFilename "LOGBUF.TXT"
#define LaunchFilename "LAUNCH.DAT"
#define BGBMPFilename "BGBMP.DAT"
#define ResumeFilename "RESUME.DAT"
#define PlayListFolderFilename "PLAYLIST.ALL"
#define ResumePlayListFilename "RESUME.PLS"
#define ResumeVideoFilename "RESUME.VID"
#define LanguageFilename "LANGUAGE.INI"

#define DiskCheck_Read16bitBinFilename "CHK16BIT.BIN"
#define HiddenPathsFilename "HIDEPATH.INI"

#define FolderVersionFilename "VERSION.DAT"
#define OverlayDLLFilename "OVERLAY.DLL"
#define FontWidthFilenameFormat "FONT%d.GLW"
#define FontGlyphFilenameFormat "FONT%d.GLF"
#define FontClearTypeFilenameFormat "FONT%d.CTF"

#define SwapFilename "SWAPFILE.$$$"
#define PrgJpegSwapFilename "PRGJPEG.$$$"

#define ExtLinkPath DefaultDataPath "/EXTLINK"
#define ExtLinkDATFilename "EXTLINK.DAT"

#define SNDEFFDATFilename "SNDEFF.DAT"

#define DefaultNFilename "/defaultn.nds"
#define DefaultXFilename "/defaultx.nds"
#define DefaultYFilename "/defaulty.nds"

#define MemoPath "/FISHMEMO"

#define MSPPackageFilename "plugins.pak"

// ------------------------------------------------------

bool Shell_FAT_fopen_isExists_Root(const char *fn);
FAT_FILE* Shell_FAT_fopen_Root(const char *fn);
FAT_FILE* Shell_FAT_fopen_Root_WithCheckExists(const char *fn);
FAT_FILE* Shell_FAT_fopenwrite_Root(const char *fn);
FAT_FILE* Shell_FAT_fopenreadwrite_Root(const char *fn);
FAT_FILE* Shell_FAT_fopencreate_Root(const char *fn);
FAT_FILE* Shell_FAT_fopenmodify_Root(const char *fn);
bool Shell_FAT_fopen_isExists_Internal(const char *fn);
FAT_FILE* Shell_FAT_fopen_Internal(const char *fn);
FAT_FILE* Shell_FAT_fopen_Internal_WithCheckExists(const char *fn);
FAT_FILE* Shell_FAT_fopenwrite_Internal(const char *fn);
FAT_FILE* Shell_FAT_fopenreadwrite_Internal(const char *fn);
FAT_FILE* Shell_FAT_fopencreate_Internal(const char *fn);
FAT_FILE* Shell_FAT_fopenmodify_Internal(const char *fn);

FAT_FILE* Shell_FAT_fopen_Cheat(void);

FAT_FILE* Shell_FAT_fopen_FullPath(const UnicodeChar *pFullPathUnicode);
FAT_FILE* Shell_FAT_fopen_Split(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode);

FAT_FILE* Shell_FAT_fopen_TextFont(const char *fn);
FAT_FILE* Shell_FAT_fopen_CodepageToUnicodeTable(void);
FAT_FILE* Shell_FAT_fopen_VariableCodepageToUnicodeTable(u32 cp);
FAT_FILE* Shell_FAT_fopen_EUCToUnicodeTable(void);

FAT_FILE* Shell_FAT_fopen_MIDIData(const char *fn);

// ------------------------------------------------------

void Shell_FAT_fopen_LanguageInit(void);
bool Shell_isEUCmode(void);
const char *Shell_GetCodePageStr(void);
FAT_FILE* Shell_FAT_fopen_Language_chrglyph(void);
FAT_FILE* Shell_FAT_fopen_Language_messages(void);

// ------------------------------------------------------

bool Shell_FAT_ReadAlloc(TMM *pMM,const char *fn,void **pbuf,s32 *psize);
bool Shell_FAT_ReadSEAlloc(TMM *pMM,const char *fn,void **pbuf,s32 *psize);

void Shell_FAT_ReadMSP_Open(void);
void Shell_FAT_ReadMSP_Close(void);
void Shell_FAT_ReadMSP_ReadHeader(const char *fn,void *buf,s32 size);
void Shell_FAT_ReadMSP_ReadBody(TMM *pMM,const char *fn,void **pbuf,s32 *psize);

// ------------------------------------------------------

void Shell_Init_SwapFile(void);
FAT_FILE* Shell_FAT_fopen_SwapFile(u32 RequestSizeByte);
void Shell_FAT_fclose_SwapFile(void);

void Shell_Init_SwapFile_PrgJpeg(void);
FAT_FILE* Shell_FAT_fopen_SwapFile_PrgJpeg(u32 RequestSizeByte);
void Shell_FAT_fclose_SwapFile_PrgJpeg(void);

// ------------------------------------------------------

extern const char* ConvertFull_Unicode2Alias(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode);
extern const char* ConvertPath_Unicode2Alias_CheckExists(const UnicodeChar *pBasePathUnicode);
extern const char* ConvertFullPath_Unicode2Alias(const UnicodeChar *pFullPathUnicode);
extern const char* ConvertFullPath_Ansi2Alias(const char *pFullPathAnsi);
extern bool forPLS_ConvertFull_Unicode2Alias(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode,char *pPathAlias,char *pFilenameAlias);

// ------------------------------------------------------

extern bool FileExistsUnicode(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode);
extern bool PathExistsUnicode(const UnicodeChar *pBasePathUnicode);

extern bool FullPath_FileExistsUnicode(const UnicodeChar *pFullPathUnicode);
extern bool FullPath_FileExistsAnsi(const char *pFullPathAnsi);

extern void SplitItemFromFullPathAlias(const char *pFullPathAlias,char *pPathAlias,char *pFilenameAlias);
extern void SplitItemFromFullPathUnicode(const UnicodeChar *pFullPathUnicode,UnicodeChar *pPathUnicode,UnicodeChar *pFilenameUnicode);

extern const UnicodeChar* FullPathUnicodeFromSplitItem(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pFilenameUnicode);

// ------------------------------------------------------

const char* Shell_CreateNewFileUnicode(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode);

// ------------------------------------------------------





extern bool isSwapFilenameUnicode_isEqual;
extern bool isSwapFilenameUnicode(const UnicodeChar *puc0,const UnicodeChar *puc1);

// ------------------------------------------------------

const UnicodeChar* Shell_GetMemoPathUnicode(void);

// ------------------------------------------------------

static inline const UnicodeChar* Shell_GetDustBoxPathUnicode(void)
{
  static const UnicodeChar fnw[9]={(UnicodeChar)'/',(UnicodeChar)'d',(UnicodeChar)'u',(UnicodeChar)'s',(UnicodeChar)'t',(UnicodeChar)'b',(UnicodeChar)'o',(UnicodeChar)'x',0};
  return(fnw);
}

static inline const UnicodeChar* Shell_GetSystemPathUnicode(void)
{
  static const UnicodeChar fnw[10]={(UnicodeChar)'/',(UnicodeChar)'m',(UnicodeChar)'o',(UnicodeChar)'o',(UnicodeChar)'n',(UnicodeChar)'s',(UnicodeChar)'h',(UnicodeChar)'l',(UnicodeChar)'2',0};
  return(fnw);
}

// ------------------------------------------------------

#endif

