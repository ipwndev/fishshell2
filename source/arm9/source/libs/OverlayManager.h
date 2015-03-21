
#ifndef OverlayManager_h
#define OverlayManager_h

#include "maindef.h"

extern void OVM_Init(void);
extern void OVM_Free(void);

extern u32 OVM_GetRomeo2NCD_Size(void);
extern void OVM_GetRomeo2NCD_Data(void *pdstbuf,u32 dstbufsize);

extern void OVM_proc_MainPass(void);

extern void OVM_proc_Start(ENextProc NextProc);

extern void OVM_libsnd_mp3(void);
extern void OVM_libsnd_midi(void);
extern void OVM_libsnd_gme(void);
extern void OVM_libsnd_ogg(void);
extern void OVM_libsnd_wave(void);

extern void OVM_libimg_jpeg(void);
extern void OVM_libimg_png(void);
extern void OVM_libimg_bmp(void);
extern void OVM_libimg_gif(void);
extern void OVM_libimg_psd(void);

extern void OVM_libglobal_dpg(void);
extern void OVM_libglobal_ndsrom(void);

extern void OVM_LoadAfterSystem(void);
extern void OVM_LoadAfterSystem2(void);

#endif

