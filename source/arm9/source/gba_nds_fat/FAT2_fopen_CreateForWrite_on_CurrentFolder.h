
static bool FAT2_fopen_CreateForWrite_ins_AddDirEntry (const char *pFilenameAlias,const u16 *pFilenameUnicode, DIR_ENT newDirEntry)
{
  // Store current working directory
  u32 oldWorkDirCluster=curWorkDirCluster;

  { // Check illigal chars.
    _consolePrint("fopenwrite: Check illigal chars.\n");
    u32 idx;
    idx=0;
    while(1){
      const char ch=pFilenameAlias[idx++];
      if(ch==0) break;
      if((ch=='\"')||(ch=='*')||(ch=='/')||(ch==':')||(ch=='<')||(ch=='>')||(ch=='?')||(ch=='\\')||(ch=='|')){
        StopFatalError(12101,"Included illigal chars in Alias and LFN. [%s]\n",pFilenameAlias);
      }
      if((ch=='+')||(ch==',')||(ch==';')||(ch=='=')||(ch=='[')||(ch==']')){
        StopFatalError(12102,"Included illigal chars in Alias. [%s]\n",pFilenameAlias);
      }
    }
    idx=0;
    while(1){
      const u16 ch=pFilenameUnicode[idx++];
      if(ch==0) break;
      if((ch=='\"')||(ch=='*')||(ch=='/')||(ch==':')||(ch=='<')||(ch=='>')||(ch=='?')||(ch=='\\')||(ch=='|')){
        StopFatalError(12103,"Included illigal chars in LFN. [%s]\n",pFilenameAlias);
      }
    }
  }
  
  {  // Store alias name and ext.
    u32 srcpos=0;
    {
      for(u32 idx=0;idx<8;idx++){
        newDirEntry.name[idx]=' ';
      }
      u32 dstpos=0;
      while(pFilenameAlias[srcpos]!=0){
        char ch=pFilenameAlias[srcpos++];
        if(ch=='.') break;
        if(8<=dstpos) StopFatalError(12104,"Alias name overflow. %d,%d [%s]\n",srcpos,dstpos,pFilenameAlias);
        newDirEntry.name[dstpos++]=ch;
      }
    }
    {
      for(u32 idx=0;idx<3;idx++){
        newDirEntry.ext[idx]=' ';
      }
      u32 dstpos=0;
      while(pFilenameAlias[srcpos]!=0){
        char ch=pFilenameAlias[srcpos++];
        if(3<=dstpos) StopFatalError(12105,"Alias ext overflow. %d,%d [%s]\n",srcpos,dstpos,pFilenameAlias);
        newDirEntry.ext[dstpos++]=ch;
      }
    }
    _consolePrintf("fopenwrite: Alias name=[%8.8s] ext=[%3.3s]\n",newDirEntry.name,newDirEntry.ext);
  }
  
  unsigned char chkSum = 0;
  { // Calculate checksum.
    for(u32 idx=0;idx<8;idx++){
      char ch=newDirEntry.name[idx];
      // NOTE: The operation is an unsigned char rotate right
      chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + ch;
    }
    for(u32 idx=0;idx<3;idx++){
      char ch=newDirEntry.ext[idx];
      // NOTE: The operation is an unsigned char rotate right
      chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + ch;
    }
    _consolePrintf("fopenwrite: Check sum is 0x%x.\n",chkSum);
  }
  
  {
    char lfn[256];
    char *plfn=lfn;
    const u16 *pufn=pFilenameUnicode;
    while(*pufn!=0){
      u16 uc=*pufn++;
      if(uc<0x20){
        *plfn++='_';
        }else{
        if(uc<0x80){
          *plfn++=(char)uc;
          }else{
          *plfn++='?';
        }
      }
    }
    *plfn=0;
    _consolePrintf("fopenwrite: LFN=[%s]\n",lfn);
  }
  
  { // Create long filename.
    const u32 maxlfnlen=256;
    static u16 lfn[maxlfnlen+1];
    for(u32 idx=0;idx<maxlfnlen+1;idx++){
      lfn[idx]=0xffff; // Set for LFN compatibility
    }
    u32 idx=0;
    while(pFilenameUnicode[idx]!=0){
      if(idx==maxlfnlen) StopFatalError(12106,"LFN is too long.\n");
      lfn[idx]=pFilenameUnicode[idx];
      idx++;
    }
    lfn[idx]=0;
    pFilenameUnicode=lfn;
  }
  
  int lfnPos;
  { // Calc long filename entry size
    int lfnlen = 0;
    while(pFilenameUnicode[lfnlen]!=0){
      lfnlen++;
    }
    lfnPos = (lfnlen-1) / 13; // Ignore NULL char.
    _consolePrintf("fopenwrite: lfnlen=%d lfnPos=%d\n",lfnlen,lfnPos);
  }
  
  // -------------------------------------
  
  int dirEntryLength = lfnPos + 2;
  
  DIR_ENT* dirEntries = (DIR_ENT*)globalBuffer;
  u32 dirCluster = curWorkDirCluster;
  int secOffset = 0;
  int entryOffset = 0;
  
  u32 firstSector = (dirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(dirCluster));
  
  // Modifying the last directory is a special case - have to erase following entries
  bool dirEndFlag = false;
  
  { // Search for a large enough space to fit in new directory entry
    disc_SystemReadSector (firstSector + secOffset, dirEntries);
    
    u32 tempDirCluster = dirCluster;
    int tempSecOffset = secOffset;
    int tempEntryOffset = entryOffset;
  
    int dirEntryFreeCount=0;
    
    _consolePrint("fopenwrite: seek free entry point.\n");
    while (1){
      if (dirEntries[entryOffset].name[0] == FILE_LAST){
        if(dirEntryFreeCount==0){
          tempDirCluster = dirCluster;
          tempSecOffset = secOffset;
          tempEntryOffset = entryOffset;
        }
        dirEndFlag = true;
        break;
      }
      if (dirEntries[entryOffset].name[0] == FILE_FREE){
        if(dirEntryFreeCount==0){
          tempDirCluster = dirCluster;
          tempSecOffset = secOffset;
          tempEntryOffset = entryOffset;
        }
        dirEntryFreeCount++;
        if(dirEntryLength==dirEntryFreeCount) break;
        }else{
        dirEntryFreeCount=0;
      }
      
      entryOffset++;
  
      if (entryOffset == BYTE_PER_READ / sizeof (DIR_ENT)){
//        _consolePrint("fopenwrite: Seek next sector.\n");
        secOffset++;
        if ((dirCluster == FAT16_ROOT_DIR_CLUSTER) && (secOffset == (filesysData - filesysRootDir))){
          return false;  // Got to end of root dir - can't fit in more files
        }
        if(secOffset == filesysSecPerClus){
          secOffset = 0;
          u32 NextClus=FAT_NextCluster(dirCluster);
          if (NextClus != CLUSTER_EOF){
            dirCluster = NextClus;
            firstSector = FAT_ClustToSect(dirCluster);    
            }else{
            dirCluster = FAT_LinkFreeCluster(dirCluster);
            firstSector = FAT_ClustToSect(dirCluster);    
            MemSet32CPU(FILE_LAST, dirEntries, BYTE_PER_READ); // FILE_LAST == 0
            for(u32 idx=0;idx<filesysSecPerClus;idx++){
              disc_SystemWriteSector (firstSector + idx, dirEntries);
            }
          }
        }
        disc_SystemReadSector (firstSector + secOffset, dirEntries);
        entryOffset = 0;
      }
  
    }

    // Recall last used entry
    dirCluster = tempDirCluster;
    secOffset = tempSecOffset;
    entryOffset = tempEntryOffset;
    
    _consolePrintf("fopenwrite: Finded free entry point. dirCluster=%d, secOffset=%d, entryOffset=%d.\n",dirCluster,secOffset,entryOffset);
  }

  // -------------------------------------
  
  firstSector = (dirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(dirCluster));
  
  // Re-read in first sector that will be written to
  disc_SystemReadSector (firstSector + secOffset, dirEntries);
  
  bool lfnFirstEntry=true;
  bool ReqStoreLastFlag=false;
  
  // Add new directory entry
  while (1){
    if (lfnPos >= 0){
      // Generate LFN entries
//      _consolePrint("fopenwrite: Store LFN entry.\n");
      DIR_ENT_LFN lfnEntry;
      lfnEntry.ordinal = (lfnPos + 1);
      if(lfnFirstEntry==true){
        lfnFirstEntry=false;
        lfnEntry.ordinal|=LFN_END;
      }
      const u16 *plfn=&pFilenameUnicode[lfnPos*13];
      lfnEntry.char0 = *plfn++;
      lfnEntry.char1 = *plfn++;
      lfnEntry.char2 = *plfn++;
      lfnEntry.char3 = *plfn++;
      lfnEntry.char4 = *plfn++;
      lfnEntry.char5 = *plfn++;
      lfnEntry.char6 = *plfn++;
      lfnEntry.char7 = *plfn++;
      lfnEntry.char8 = *plfn++;
      lfnEntry.char9 = *plfn++;
      lfnEntry.char10 = *plfn++;
      lfnEntry.char11 = *plfn++;
      lfnEntry.char12 = *plfn++;
      lfnEntry.checkSum = chkSum;
      lfnEntry.flag = ATTRIB_LFN;
      lfnEntry.reserved1 = 0;
      lfnEntry.reserved2 = 0;
      
      *((DIR_ENT_LFN*)&dirEntries[entryOffset]) = lfnEntry;
      lfnPos --;
      }else{
      if(ReqStoreLastFlag==false){
        // Generate Alias entry
        _consolePrint("fopenwrite: Store Alias entry.\n");
        dirEntries[entryOffset] = newDirEntry;
        if (dirEndFlag==false) break;
        ReqStoreLastFlag=true;
        }else{
        dirEntries[entryOffset].name[0] = FILE_LAST;
        break;
      }
    }
    
    entryOffset++;

    if (entryOffset == BYTE_PER_READ / sizeof (DIR_ENT)){
//      _consolePrint("fopenwrite: Seek next sector.\n");
      disc_SystemWriteSector (firstSector + secOffset, dirEntries);
      secOffset++;
      if ((dirCluster == FAT16_ROOT_DIR_CLUSTER) && (secOffset == (filesysData - filesysRootDir))){
        return false;  // Got to end of root dir - can't fit in more files
      }
      if(secOffset == filesysSecPerClus){
        secOffset = 0;
        u32 NextClus=FAT_NextCluster(dirCluster);
        if (NextClus != CLUSTER_EOF){
          dirCluster = NextClus;
          firstSector = FAT_ClustToSect(dirCluster);    
          }else{
          dirCluster = FAT_LinkFreeCluster(dirCluster);
          firstSector = FAT_ClustToSect(dirCluster);
          MemSet32CPU(FILE_LAST, dirEntries, BYTE_PER_READ); // FILE_LAST == 0
          for(u32 idx=0;idx<filesysSecPerClus;idx++){
            disc_SystemWriteSector (firstSector + idx, dirEntries);
          }
        }
      }
      disc_SystemReadSector (firstSector + secOffset, dirEntries);
      entryOffset = 0;
    }
  }
  
  // Write directory back to disk
  disc_SystemWriteSector (firstSector + secOffset, dirEntries);

  // Change back to Working DIR
  curWorkDirCluster = oldWorkDirCluster;

  return true;
}

FAT_FILE* FAT2_fopen_CreateForWrite_on_CurrentFolder(const char *pFilenameAlias,const u16 *pFilenameUnicode)
{
  FAT_FILE* file;
  
  { // Find a free file buffer
    int fileNum;
    for (fileNum = 0; (fileNum < MAX_FILES_OPEN) && (openFiles[fileNum].inUse == true); fileNum++);
    if (fileNum == MAX_FILES_OPEN){ // No free files
      StopFatalError(12107,"FAT_FILE buffer full.\n");
    }
    file = &openFiles[fileNum];
  }

  DIR_ENT dirEntry;
  
  { // Create file entry.
    dirEntry.name[0]=FILE_FREE;
  
    dirEntry.attrib = ATTRIB_ARCH;
    dirEntry.reserved = 0;
    
    // Time and date set to system time and date
    dirEntry.cTime_ms = 0;
    dirEntry.cTime = SystemDateTime_Time;
    dirEntry.cDate = SystemDateTime_Date;
    dirEntry.aDate = SystemDateTime_Date;
    dirEntry.mTime = SystemDateTime_Time;
    dirEntry.mDate = SystemDateTime_Date;
  }
  
    
  // Get a cluster to use
  u32 startCluster = FAT_LinkFreeCluster (CLUSTER_FREE);
  if (startCluster == CLUSTER_FREE){ // Couldn't get a free cluster
    StopFatalError(12108,"Create first cluster error. Disk full!!\n");
  }

  // Store cluster position into the directory entry
  dirEntry.startCluster = (startCluster & 0xFFFF);
  dirEntry.startClusterHigh = ((startCluster >> 16) & 0xFFFF);

  // The file has no data in it - its over written so should be empty
  dirEntry.fileSize = 0;

  // Have to create a new entry
  if(!FAT2_fopen_CreateForWrite_ins_AddDirEntry (pFilenameAlias, pFilenameUnicode, dirEntry)) StopFatalError(12109,"Can not add dir entry.\n");
  // Get the newly created dirEntry
  dirEntry = FAT_DirEntFromPath (pFilenameAlias);

  // Remember where directory entry was
  file->dirEntSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector;
  file->dirEntOffset = wrkDirOffset;

  // Now that file is created, open it
  file->read = false;
  file->write = true;
  file->append = false;
  
  // Store information about position within the file, for use
  // by FAT_fread, FAT_fseek, etc.
  file->firstCluster = startCluster;
  file->length = 0;  // Should always have 0 bytes if openning in "w" mode
  file->curPos = 0;
  file->curClus = startCluster;
  file->curSect = 0;
  file->curByte = 0;

  // Not appending
  file->appByte = 0;
  file->appClus = 0;
  file->appSect = 0;

  // Empty file, so empty read buffer
  MemSet32CPU(0, file->readBuffer, BYTE_PER_READ);
  file->inUse = true;  // We're using this file now

  return file;
}

