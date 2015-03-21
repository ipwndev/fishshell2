
#ifndef cglscreen_h
#define cglscreen_h

#include <stdlib.h>
#include <nds.h>

#include "cglcanvas.h"

enum EScrMainMode {ESMM_Normal,ESMM_ForARM7};

class CglScreenMain
{
  u16 *VRAMBufArray[2];
  u32 BackVRAMPage;
  EScrMainMode mode;
  CglScreenMain(const CglScreenMain&);
  CglScreenMain& operator=(const CglScreenMain&);
public:
  CglCanvas *pViewCanvas,*pBackCanvas;
  CglScreenMain(void);
  ~CglScreenMain(void);
  void Flip(const bool ShowFlag);
  void Flip_FlickerFree(void);
  void Flip_FlickerFree_FromInterrupt(void);
  void FlipForVSyncAuto(void);
  void SetBlendLevel(const int BlendLevel);
  void SetBlendLevelManual(const int BlendLevelBack,const int BlendLevelView);
  void SetMode(EScrMainMode _mode);
  EScrMainMode GetMode(void);
};

class CglScreenMainOverlay
{
  CglScreenMainOverlay(const CglScreenMainOverlay&);
  CglScreenMainOverlay& operator=(const CglScreenMainOverlay&);
public:
  CglCanvas *pCanvas;
  CglScreenMainOverlay(void);
  ~CglScreenMainOverlay(void);
  u16* GetVRAMBuf(void) const;
  void SetPosition_for_Right64x192(s32 x,s32 y) const;
  void SetVisible_for_LeftTop128x64(bool Visible) const;
};

class CglScreenSub
{
  CglScreenSub(const CglScreenSub&);
  CglScreenSub& operator=(const CglScreenSub&);
public:
  CglCanvas *pCanvas;
  CglScreenSub(void);
  ~CglScreenSub(void);
  u16* GetVRAMBuf(void) const;
  void SetBlackOutLevel16(const int Level16);
};

#endif

