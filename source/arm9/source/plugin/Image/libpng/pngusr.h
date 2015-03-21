
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"

#include "memtool.h"

#include "fat2.h"

#define PNG_NO_ASSEMBLER_CODE

#define PNG_SETJMP_NOT_SUPPORTED
#define PNG_NO_SETJMP_SUPPORTED

#define PNG_NO_WRITE_SUPPORTED

#define PNG_NO_STDIO
#define PNG_NO_MNG_FEATURES
#define PNG_NO_FLOATING_POINT_SUPPORTED
#define PNG_NO_FIXED_POINT_SUPPORTED

#define PNG_NO_READ_UNKNOWN_CHUNKS
#define PNG_NO_WRITE_UNKNOWN_CHUNKS

#define PNG_USE_LOCAL_ARRAYS

#define PNG_NO_ERROR_NUMBERS

#define PNG_NO_READ_tIME
