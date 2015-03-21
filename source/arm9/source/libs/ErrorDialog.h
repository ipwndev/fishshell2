
#ifndef ErrorDialog_h
#define ErrorDialog_h

#include <nds.h>
#include "glib.h"

enum EErrorCode {
EEC_MemoryOverflow_CanRecovery,
EEC_NotSupportFileFormat,
EEC_ProgressiveJpeg,
EEC_Text0byte,
EEC_OverflowLargeImage,
EEC_NotFoundMusicFile,

EEC_UnknownError,
};

extern void ErrorDialog_Init(void);
extern void ErrorDialog_Clear(void);
extern bool ErrorDialog_isExists(void);
extern void ErrorDialog_Set(EErrorCode _ErrorCode);
extern void ErrorDialog_Draw(CglCanvas *pcan);

#endif

