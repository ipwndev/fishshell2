
#ifndef playlist_h
#define playlist_h

#include <nds.h>
#include "plug_lyric.h" 

extern bool RequestRefreshPlayCursorIndex;

extern bool PlayList_isOpened(void);

extern void PlayList_Init(void);
extern void PlayList_Free(void);

extern void PlayList_UpdateResume(u32 VsyncCount);

extern void PlayList_Stop(bool WaitForEmpty);

extern void PlayList_Repeat(void);
extern void PlayList_Prev(void);
extern void PlayList_PrevShuffle(void);
extern void PlayList_Next(void);
extern void PlayList_NextShuffle(void);

extern u32 PlayList_GetFilesCount(void);
extern u32 PlayList_GetCurrentIndex(void);
extern const UnicodeChar* PlayList_GetCurrentFilename(void);
extern const UnicodeChar* PlayList_GetCurrentPath(void);

extern bool PlayList_GetListEndFlag(void);

extern void PlayList_ConvertM3U(const UnicodeChar *pBasePathUnicode,const UnicodeChar *pPlayListFilenameUnicode);
extern void PlayList_MakeFolder(bool FindDeep,const UnicodeChar *pBasePathUnicode);

extern void PlayList_MakeBlank(void);

extern bool PlayList_DeleteListItem(const UnicodeChar *pFileNameUnicode);

extern bool PlayList_Start(bool ShowPrg,const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode);

extern bool PlayList_GetPause(void);
extern void PlayList_SetPause(bool f);
extern void PlayList_TogglePause(void);

extern void PlayList_SetError(bool f);

#endif
