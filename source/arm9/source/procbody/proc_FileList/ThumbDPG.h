
#ifndef thumbdpg_h
#define thumbdpg_h

extern void InitDPGThumb(void);
extern void FreeDPGThumb(void);
extern void ReloadDPGThumb(const UnicodeChar *pCurrentPathUnicode,s32 fileidx);
extern bool DrawDPGThumb(CglCanvas *pcan);

#endif
