
// ------------------------------------------------

void Shell_Init_SwapFile(void)
{
  if(Shell_FAT_fopen_isExists_Internal(SwapFilename)==false) return;
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Internal(SwapFilename);
    u32 fsize=FAT2_GetFileSize(pf);
    FAT2_fclose(pf);
    if(fsize==1) return;
  }
  
  FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(SwapFilename);
  u8 tmp=0;
  FAT2_fwrite(&tmp,1,1,pf);
  FAT2_fclose(pf);
}

FAT_FILE* Shell_FAT_fopen_SwapFile(u32 RequestSizeByte)
{
  if(Shell_FAT_fopen_isExists_Internal(SwapFilename)==false) return(NULL);
  
  FAT_FILE *pf=Shell_FAT_fopen_Internal(SwapFilename);
  if(pf==NULL) StopFatalError(13602,"Can not found swap file. [%s]\n",SwapFilename);
  FAT2_fclose(pf);
  
  if(VerboseDebugLog==true) _consolePrintf("Create swap file. %dbytes.\n",RequestSizeByte);
  
  pf=Shell_FAT_fopenwrite_Internal(SwapFilename);
  FAT2_SetSize(pf,RequestSizeByte,0xAB);
  
  return(pf);
}

void Shell_FAT_fclose_SwapFile(void)
{
  FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(SwapFilename);
  u8 tmp=0;
  FAT2_fwrite(&tmp,1,1,pf);
  FAT2_fclose(pf);
}

// ------------------------------------------------

void Shell_Init_SwapFile_PrgJpeg(void)
{
  if(Shell_FAT_fopen_isExists_Internal(PrgJpegSwapFilename)==false) return;
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Internal(PrgJpegSwapFilename);
    u32 fsize=FAT2_GetFileSize(pf);
    FAT2_fclose(pf);
    if(fsize==1) return;
  }
  
  FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(PrgJpegSwapFilename);
  u8 tmp=0;
  FAT2_fwrite(&tmp,1,1,pf);
  FAT2_fclose(pf);
}

FAT_FILE* Shell_FAT_fopen_SwapFile_PrgJpeg(u32 RequestSizeByte)
{
  if(Shell_FAT_fopen_isExists_Internal(PrgJpegSwapFilename)==false) return(NULL);
  
  FAT_FILE *pf=Shell_FAT_fopen_Internal(PrgJpegSwapFilename);
  if(pf==NULL) StopFatalError(13603,"Can not found prgjpeg swap file. [%s]\n",PrgJpegSwapFilename);
  FAT2_fclose(pf);
  
  if(VerboseDebugLog==true) _consolePrintf("Create PrgJpeg swap file. %dbytes.\n",RequestSizeByte);
  
  pf=Shell_FAT_fopenwrite_Internal(PrgJpegSwapFilename);
  FAT2_SetSize(pf,RequestSizeByte,0xAB);
  
  return(pf);
}

void Shell_FAT_fclose_SwapFile_PrgJpeg(void)
{
  FAT_FILE *pf=Shell_FAT_fopenwrite_Internal(PrgJpegSwapFilename);
  u8 tmp=0;
  FAT2_fwrite(&tmp,1,1,pf);
  FAT2_fclose(pf);
}


