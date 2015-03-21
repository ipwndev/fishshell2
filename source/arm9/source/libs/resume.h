
#ifndef resume_h
#define resume_h

#include <nds.h>

#include "unicode.h"

enum EResumeMode {ERM_None,ERM_Video,ERM_Image,ERM_Text};

extern void Resume_Load(void);
extern void Resume_Clear(void);
extern void Resume_Save(void);

extern void Resume_SetResumeMode(EResumeMode rm);
extern void Resume_SetFilename(const UnicodeChar *pFullFilenameUnicode);
extern void Resume_SetPos(u32 pos);

extern EResumeMode Resume_GetResumeMode(void);
extern const UnicodeChar* Resume_GetFilename(void);
extern u32 Resume_GetPos(void);

#endif

