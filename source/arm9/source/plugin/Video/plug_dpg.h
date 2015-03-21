
#ifndef _plug_dpg_h
#define _plug_dpg_h

#include "clibdpg.h"
#include "clibmpg.h"

#include "fat2.h"

#define DPGTitle0 libdpgTitle
#define DPGTitle1 libmpeg2Title
#define DPGTitle2 "libmad - MPEG audio decoder library"

extern bool DPG_RequestSyncStart;

extern bool StartDPG(FAT_FILE *_FileHandleVideo,FAT_FILE *_FileHandleAudio);
extern void UpdateDPG_Audio(void);
extern bool UpdateDPG_Video();
extern void FreeDPG(void);

extern u32 DPG_GetCurrentFrameCount(void);
extern u32 DPG_GetTotalFrameCount(void);
extern u32 DPG_GetFPS(void);
extern u32 DPG_GetSampleRate(void);
extern u32 DPG_GetChannelCount(void);
extern void DPG_SetFrameCount(u32 Frame);

extern u32 DPG_GetWidth(void);
extern u32 DPG_GetHeight(void);

extern void DPG_DrawInfo(CglCanvas *pCanvas);
extern u32 DPG_DrawInfoDiffHeight(void);
extern void DPG_DrawInfoDiff(CglCanvas *pCanvas);

extern EDPGAudioFormat DPG_GetDPGAudioFormat(void);

extern void DPG_SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2);

#endif
