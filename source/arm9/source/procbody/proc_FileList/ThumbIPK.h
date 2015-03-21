
#ifndef ipk_h
#define ipk_h

extern void InitIPK(void);
extern void FreeIPK(void);
extern void ReloadIPK(const UnicodeChar *pCurrentPathUnicode,s32 fileidx);
extern bool DrawIPK(CglCanvas *pcan);

#endif
