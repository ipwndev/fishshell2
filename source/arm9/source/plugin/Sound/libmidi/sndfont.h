
#ifndef sndfont_h
#define sndfont_h

extern void SndFont_Open(void);
extern void SndFont_Close(void);
extern void SndFont_SetOffset(u32 ofs);
extern u32 SndFont_Read16bit(void *_pbuf,u32 size);
extern u32 SndFont_Get32bit(void);

#endif

