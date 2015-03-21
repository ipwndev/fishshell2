
// ------------------------------------------------------------------------------------

bool Shell_FAT_fopen_isExists_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  return(FullPath_FileExistsAnsi(Shell_FAT_fopen_fullfn));
}

FAT_FILE* Shell_FAT_fopen_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_Root=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopen_Root_WithCheckExists(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  if(FullPath_FileExistsAnsi(Shell_FAT_fopen_fullfn)==false) return(NULL);
  return(Shell_FAT_fopen_Root(fn));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenwrite_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenwrite_Root=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13616,"Not found base file for write.\n");
  return(FAT2_fopen_AliasForWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenreadwrite_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenreadwrite_Root=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13616,"Not found base file for write.\n");
  return(FAT2_fopen_AliasForReadWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopencreate_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopencreate_Root=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13617,"Not found base file for create.\n");
  return(FAT2_fopen_AliasForWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenmodify_Root(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultRootPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenmodify_Root=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13618,"Not found base file for modify.\n");
  return(FAT2_fopen_AliasForModify(pfullalias));
}
// ------------------------------------------------------------------------------------


CODE_IN_AfterSystem bool Shell_FAT_fopen_isExists_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  //_consolePrintf("Shell_FAT_fopen_isExists_Internal=%s\n",Shell_FAT_fopen_fullfn);
  return(FullPath_FileExistsAnsi(Shell_FAT_fopen_fullfn));
}

FAT_FILE* Shell_FAT_fopen_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_Internal=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopen_Internal_WithCheckExists(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  if(FullPath_FileExistsAnsi(Shell_FAT_fopen_fullfn)==false) return(NULL);
  return(Shell_FAT_fopen_Internal(fn));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenwrite_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenwrite_Internal=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13616,"Not found base file for write.\n");
  return(FAT2_fopen_AliasForWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenreadwrite_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenreadwrite_Internal=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13616,"Not found base file for write.\n");
  return(FAT2_fopen_AliasForReadWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopencreate_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopencreate_Internal=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13617,"Not found base file for create.\n");
  return(FAT2_fopen_AliasForWrite(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopenmodify_Internal(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopenmodify_Internal=%s\n",pfullalias);
  if(str_isEmpty(pfullalias)==true) StopFatalError(13618,"Not found base file for write.\n");
  return(FAT2_fopen_AliasForModify(pfullalias));
}

CODE_IN_AfterSystem FAT_FILE* Shell_FAT_fopen_MIDIData(const char *fn)
{
  snprintf(Shell_FAT_fopen_fullfn,MaxFilenameLength,DefaultInternalPath "/%s",fn);
  const char *pfullalias=ConvertFullPath_Ansi2Alias(Shell_FAT_fopen_fullfn);
  if(VerboseDebugLog==true) _consolePrintf("Shell_FAT_fopen_MIDIData=%s\n",pfullalias);
  return(FAT2_fopen_AliasForRead(pfullalias));
}
// ------------------------------------------------------------------------------------

