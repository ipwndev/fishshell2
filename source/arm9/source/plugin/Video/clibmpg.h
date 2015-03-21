
#ifndef clibmpg_h
#define clibmpg_h

#define libmpeg2Title "libmpeg2 is a free library for decoding mpeg-2 and mpeg-1 video streams."

#include <nds.h>
#include "cstream.h"

#include "libmpeg2/inttypes.h"
#include "libmpeg2/mpeg2.h"
#include "libmpeg2/mpeg2convert.h"

typedef struct {
  int width,height;
  int Ydiv2;
} TYUV420toBGR15_DATA;

enum EMPGPixelFormat {PF_RGB15=0,PF_RGB18=1,PF_RGB21=2,PF_RGB24=3,PF_ENUMCOUNT=4};

class Clibmpg
{
  CStream *pCStream;
  int DataTopPosition;
  const u32 TotalFrameCount,FPS;
  const u32 SndFreq;
  const EMPGPixelFormat PixelFormat;
  u32 GlobalDelaySamples;
  u8 *ReadBuf;
  mpeg2dec_t *decoder;
  const mpeg2_info_t *info;
  int Width,Height;
  u32 framenum;
  TYUV420toBGR15_DATA YUV420toBGR15_DATA;
  Clibmpg(const Clibmpg&);
  Clibmpg& operator=(const Clibmpg&);
  bool ProcReadBuffer(void);
  bool ProcSequence(void);
  void YUV420toBGR15_Init(int Width,int Height);
  void YUV420toBGR15Fix_CopyFull0_asm(const void *fbuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyFull1_asm(const void *fbuf,u16 *_FrameBuf);
  void YUV420toBGR15Fix_CopyFull2_asm(const void *fbuf,u16 *_FrameBuf);
  void debug_tableout(void);
public:
  bool Initialized;
  Clibmpg(CStream *_pCStream,const u32 _TotalFrameCount,const u32 _FPS,const u32 _SndFreq,const EMPGPixelFormat _PixelFormat);
  ~Clibmpg(void);
  int GetWidth(void) const;
  int GetHeight(void) const;
  void Reopen(u32 StartFrame,u32 StartOffset);
  bool ProcMoveFrame(u32 TargetFrame,u64 TargetSamplesCount);
  bool ProcMoveFrameGOP(u32 TargetFrame,u64 TargetSamplesCount,u32 TargetGOPFrame,u32 TargetGOPOffset);
  bool ProcDecode(void);
  void SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2);
  int GetFrameNum(void);
};

extern bool FrameCache_isReadEmpty(void);
extern u32 FrameCache_GetReadFramesCount(void);
extern u32 FrameCache_GetReadFrameLastCount(void);

#endif

