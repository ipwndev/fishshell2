
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_const.h"
#include "_console.h"
#include "shell.h"
#include "strtool.h"
#include "procstate.h"
#include "unicode.h"

#include "memtool.h"
#include "arm9tcm.h"

#include "lang.h"

extern char Shell_FAT_fopen_fullfn[MaxFilenameLength+1];

// ----------------------------------------

FAT_FILE* Shell_FAT_fopen_FullPath(const UnicodeChar *pFullPathUnicode)
{
  const char *pfullalias=ConvertFullPath_Unicode2Alias(pFullPathUnicode);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_FullPath=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_Split(const UnicodeChar *pFilePathUnicode,const UnicodeChar *pFileNameUnicode)
{
  const char *pfullalias=ConvertFull_Unicode2Alias(pFilePathUnicode,pFileNameUnicode);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13619,"Not found base file for split read.\n");
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_Split=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

// ------------------------------------------------

DATA_IN_AfterSystem static char CodePageStr[4]={0,0,0,0};
DATA_IN_AfterSystem static bool isCHNmode;
DATA_IN_AfterSystem static bool isJPNmode;
DATA_IN_AfterSystem static bool isKORmode;

void Shell_FAT_fopen_LanguageInit(void)
{
  FAT_FILE *pf=FAT2_fopen_AliasForRead(DefaultLanguageSetFullPathFilename);
  
  if(pf==NULL){
    StrCopy("000",CodePageStr);
    }else{
    FAT2_fread(CodePageStr,1,3,pf);
    CodePageStr[3]=0;
    FAT2_fclose(pf);
  }
  
  _consolePrintf("Setup default code page is '%s'.\n",CodePageStr);
  
  isCHNmode=false;
  if((strcmp("936",CodePageStr)==0)||(strcmp("950",CodePageStr)==0)) isCHNmode=true;
  isJPNmode=false;
  if((strcmp("932",CodePageStr)==0)||(strcmp("933",CodePageStr)==0)) isJPNmode=true;
  isKORmode=false;
  if(strcmp("949",CodePageStr)==0) isKORmode=true;
}

bool Shell_isEUCmode(void)
{
  return(isCHNmode || isJPNmode || isKORmode);
}

const char *Shell_GetCodePageStr(void)
{
  return(CodePageStr);
}

FAT_FILE* Shell_FAT_fopen_Language_chrglyph(void)
{
  if(CodePageStr[0]=='0'){
    snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultLanguageDataPath "/chrglyph.000");
    }else{
    snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultLanguageDataPath "/chrglyph.%s",CodePageStr);
  }
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_Language_chrglyph=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_Language_messages(void)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultLanguageDataPath "/messages.%s",CodePageStr);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_Language_messages=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_TextFont(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultDataPath "/%s/%s",Lang_GetUTF8("TextFontFolderName"),fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_TextFont=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_CodepageToUnicodeTable(void)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultDataPath "/unicode/cp%d.tbl",ProcState.Text.DefaultCodePage);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_CodepageToUnicodeTable=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_VariableCodepageToUnicodeTable(u32 cp)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultDataPath "/unicode/cp%d.tbl",cp);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_VariableCodepageToUnicodeTable=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

FAT_FILE* Shell_FAT_fopen_EUCToUnicodeTable(void)
{
    snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultDataPath "/unicode/cp%s.tbl",CodePageStr);
    const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
    if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_EUCToUnicodeTable=%s\n",pfullalias);
    return(FAT2_fopen_AliasForRead(pfullalias));
}

// ----------------------------------------

#include "shell_SwapFile.h"

#include "shell_CreateNewFile.h"

// ----------------------------------------

DATA_IN_AfterSystem bool isSwapFilenameUnicode_isEqual;

bool isSwapFilenameUnicode(const UnicodeChar *puc0,const UnicodeChar *puc1)
{
  isSwapFilenameUnicode_isEqual=false;
  
  if(puc0==puc1){
    isSwapFilenameUnicode_isEqual=true;
    return(false);
  }
  
  while(1){
    u32 uc0=*puc0;
    u32 uc1=*puc1;
    if(((u32)'A'<=uc0)&&(uc0<=(u32)'Z')) uc0+=0x20;
    if(((u32)'A'<=uc1)&&(uc1<=(u32)'Z')) uc1+=0x20;
    
    if(uc0==uc1){
      if(uc0==0){
        isSwapFilenameUnicode_isEqual=true;
        return(false);
      }
      }else{
      // ファイル名長さチェック
      if(uc0==0) return(false);
      if(uc1==0) return(true);
      if(uc0==(u32)'.') return(false);
      if(uc1==(u32)'.') return(true);
      // 文字比較
      if(uc0<uc1){
        return(false);
        }else{
        return(true);
      }
    }
    
    puc0++; puc1++;
  }
}


// ------------------------------------------------------
