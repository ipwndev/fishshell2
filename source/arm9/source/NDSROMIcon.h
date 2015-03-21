
#ifndef NDSROMIcon_h
#define NDSROMIcon_h

#include <nds.h>

#include "glib.h"

#define NDSROMIcon32Width (32)
#define NDSROMIcon32Height (32)
#define NDSROMIcon16Width (16)
#define NDSROMIcon16Height (16)

#define NDSROMIconXMargin (2)

typedef struct {
  u16 Size32BM[NDSROMIcon32Width*NDSROMIcon32Height];
  u16 Size16BM[NDSROMIcon16Width*NDSROMIcon16Height];
  u8 Size16Alpha[NDSROMIcon16Width*NDSROMIcon16Height];
} TNDSROMIcon;

extern bool NDSROMIcon_Get(const char *pFilenameAlias,TNDSROMIcon *pNDSROMIcon,u16 *pJpnTitle ,u16 *pEngTitle,bool *isCommercialROM);
extern void NDSROMIcon_DrawIcon16(TNDSROMIcon *pNDSROMIcon,CglCanvas *ptagbm,u32 dx,u32 dy);
extern void NDSROMIcon_DrawIcon32(TNDSROMIcon *pNDSROMIcon,CglCanvas *ptagbm,u32 dx,u32 dy);
extern bool NDSROMIcon_isLoaded(TNDSROMIcon *pNDSROMIcon);

#endif

