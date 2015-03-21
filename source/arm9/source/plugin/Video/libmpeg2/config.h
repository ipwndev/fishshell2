
#ifndef CONFIG_H
#define CONFIG_H

#include <nds.h>

#define CODE_IN_ITCM_DPG __attribute__ ((section (".ITCM_libglobal_dpg")))
#undef DATA_IN_DTCM
#define DATA_IN_MTCM_SET __attribute__ ((section (".mtcmset"))) 
#define DATA_IN_MTCM_SET_CONST __attribute__ ((section (".mtcmset.const"))) 
#define DATA_IN_MTCM_VAR __attribute__ ((section (".mtcmvar"))) 

#include "_console.h"
#include "_consoleWriteLog.h"
#include "_const.h"


#endif /* CONFIG_H */
