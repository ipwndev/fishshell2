
bool FAT_DeleteFile(const char *pSrcPath)
{
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Get source directory entry.\n");
  DIR_ENT SrcDirEntry=FAT_DirEntFromPath(pSrcPath);
  
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Check source name.\n");
  if(SrcDirEntry.name[0]==FILE_FREE) StopFatalError(0,"FAT_FileDelete: Source is removed.\n");
  
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Check source file attribute.\n");
  // Only delete directories if the directory is entry
  if((SrcDirEntry.attrib&ATTRIB_DIR)!=0) StopFatalError(0,"FAT_FileDelete: Can not move source folder.\n");
  
  u32 StartClusterIndex=SrcDirEntry.startCluster | (SrcDirEntry.startClusterHigh<<16);
  _consolePrintf("FAT_FileDelete: Clear links. StartClusterIndex=0x%08x.\n",StartClusterIndex);
  FAT_ClearLinks(StartClusterIndex);
    if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Write to dist directory entry.\n");
  disc_SystemReadSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);
  ((DIR_ENT*)globalBuffer)[wrkDirOffset]=SrcDirEntry;
  disc_SystemWriteSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);
    
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Flush dist directory entry sectors in disc cache\n");
  disc_CacheFlush();
  
  // ---------------------------------
  
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Get source LFN directory entry.\n");
  SrcDirEntry=FAT_DirEntFromPath_LFNTopDirEntry(pSrcPath);
  if(SrcDirEntry.name[0]==FILE_FREE){
      if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Get source alias directory entry.\n");
    SrcDirEntry=FAT_DirEntFromPath(pSrcPath);
    if(SrcDirEntry.name[0]==FILE_FREE) StopFatalError(0,"FAT_FileDelete: Not found source directory entry.\n");
  }

  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Remove source LFN and alias directory entry.\n");
  while(SrcDirEntry.name[0]!=FILE_FREE){
    disc_SystemReadSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);
    ((DIR_ENT*)globalBuffer)[wrkDirOffset].name[0] = FILE_FREE;
    disc_SystemWriteSector ( (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector , globalBuffer);
    _consolePrintf("Removed 0x%02x, ",SrcDirEntry.attrib);
    if((SrcDirEntry.attrib & ATTRIB_LFN)!=ATTRIB_LFN){
      _consolePrintf("Alias %.8s.%.3s\n",SrcDirEntry.name,SrcDirEntry.ext);
      }else{
      _consolePrint("LFN entry.\n");
    }
    if((SrcDirEntry.attrib & ATTRIB_LFN)!=ATTRIB_LFN) break;
    DIR_ENT *pDirEntry=FAT_GetAllDirEntry((u32)-1, 1, SEEK_CUR);
    SrcDirEntry=*pDirEntry;
  }
    
  if(VerboseDebugLog==true) _consolePrint("FAT_FileDelete: Flush source directory entry sectors in disc cache\n");
  disc_CacheFlush();
  
  return(true);
}
