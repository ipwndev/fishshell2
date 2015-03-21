
#ifndef clibdpg_h
#define clibdpg_h

#define libdpgTitle "libdpg Mpeg1 and MP2 or OGG mux"

#include <nds.h>
#include "cstream.h"
#include "glib.h"

#include "clibmpg.h"

typedef struct {
  int TotalFrame;
  int FPS; // fix8.8
  int SndFreq,SndCh;
  int AudioPos,AudioSize;
  int MoviePos,MovieSize;
  int GOPListPos,GOPListSize;
  EMPGPixelFormat PixelFormat;
  bool isExistsThumbnailImage;
  int ThumbnailImageOffset;
} TDPGINFO;

typedef struct {
  u32 FrameIndex;
  u32 Offset;
} TGOPList;

enum EDPGAudioFormat {DPGAF_MP2};

class Clibdpg
{
  CStream *pCStreamMovie,*pCStreamAudio;
  Clibmpg *pClibmpg;
  Clibdpg(const Clibdpg&);
  Clibdpg& operator=(const Clibdpg&);
  bool LoadDPGINFO(CStream *pCStream);
  u32 GOPListCount;
  TGOPList *pGOPList;
public:
  Clibdpg(CStream *_pCStreamMovie,CStream *_pCStreamAudio);
  ~Clibdpg(void);
  bool Initialized;
  TDPGINFO DPGINFO;
  bool MovieProcDecode(void);
  void SliceOneFrame(u16 *pVRAMBuf1,u16 *pVRAMBuf2);
  void SetFrame(u32 Frame);
  int GetFrameNum(void);
  int GetWidth(void);
  int GetHeight(void);
  EDPGAudioFormat GetDPGAudioFormat(void);
};

#endif

