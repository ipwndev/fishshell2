
#ifndef dllsound_h
#define dllsound_h

#include <nds.h>

const u32 ID3Tag_LinesMaxCount=4;

typedef struct {
  u32 LinesCount;
  UnicodeChar *ppLines[ID3Tag_LinesMaxCount];
} TID3Tag;

extern TID3Tag ID3Tag;

extern void DLLSound_Open(const char *pFilename);
extern void DLLSound_Close(bool UseGapless);
extern bool DLLSound_isOpened(void);
extern bool DLLSound_isComplexDecoder(void);
//extern bool DLLSound_Update(void);
extern void DLLSound_UpdateLoop(bool OneShot);
extern void DLLSound_WaitForStreamPCM(void);
extern void DLLSound_SetVolume64(u32 v);
extern u32 DLLSound_GetSampleRate(void);

extern void DLLSound_SetFirstDecode(void);
extern bool DLLSound_GetFirstDecode(void);

extern s32 DLLSound_GetPosMax(void);
extern s32 DLLSound_GetPosOffset(void);
extern void DLLSound_SetPosOffset(s32 pos);

extern bool DLLSound_GetTimeSec(u32 *pCurTime,u32 *pPlayTime);

extern bool DLLSound_SeekPer(s32 per);

#endif

