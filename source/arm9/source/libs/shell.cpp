
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "shell.h"
#include "strtool.h"
#include "procstate.h"

#include "memtool.h"

#include "splash.h"
#include "lang.h"


extern char Shell_FAT_fopen_fullfn[MaxFilenameLength+1];
char Shell_FAT_fopen_fullfn[MaxFilenameLength+1];

#include "shell_FAT_fopen.h"
#include "shell_FAT_Read.h"

// -----------------------------------------------------

#include "shell_ConvertFull.h"

#include "shell_Helpers.h"



CODE_IN_AfterSystem const UnicodeChar* Shell_GetMemoPathUnicode(void)
{
	DATA_IN_AfterSystem static UnicodeChar pathw[16];
  if(FAT2_chdir_Alias(MemoPath)==false){
      StrConvert_Ank2Unicode("/",pathw);
  }else{
      StrConvert_Ank2Unicode(MemoPath,pathw);
  }
  return(pathw);




}

// ----------------------------------------

