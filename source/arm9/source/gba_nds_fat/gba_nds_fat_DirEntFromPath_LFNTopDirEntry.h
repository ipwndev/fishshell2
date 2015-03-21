static u8 FAT_DirEntFromPath_LFNTopDirEntry_ins_GetCheckSum(DIR_ENT *dir)
{
  // Calculate file checksum
  u8 chkSum = 0;
  for (u32 aliasPos=0; aliasPos < 8; aliasPos++){
    // NOTE: The operation is an unsigned char rotate right
    chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + dir->name[aliasPos];
  }
  for (u32 aliasPos=0; aliasPos < 3; aliasPos++){
    // NOTE: The operation is an unsigned char rotate right
    chkSum = ((chkSum & 1) ? 0x80 : 0) + (chkSum >> 1) + dir->ext[aliasPos];
  }
  
  return(chkSum);
}
    
static DIR_ENT FAT_DirEntFromPath_LFNTopDirEntry (const char* path)
{
  // Start at beginning of path
  u32 pathPos = 0;
  
  u32 dirCluster = curWorkDirCluster;  // Start at current working dir
  if(path[pathPos] == '/') dirCluster = filesysRootDirClus;  // Start at root directory
  
  // Eat any slash /
  while ((path[pathPos] == '/') && (path[pathPos] != '\0')){
    pathPos++;
  }
  
  if(path[pathPos]==0) StopFatalError(12301,"Illigal path. [%s]\n",path);
  
  u32 LFNTopCluster=(u32)-1;
  u32 LFNTopIndex=(u32)-1;
  
  // Search until can't continue
  while (1){
    char name[13]; // alias only.
    {
      // Copy name from path
      int namePos=0;
      ucase_start();
      
      if ((path[pathPos] == '.') && ((path[pathPos + 1] == '\0') || (path[pathPos + 1] == '/'))) {
        // Dot entry
        name[namePos++] = '.';
        pathPos+=1;
        }else{
        if ((path[pathPos] == '.') && (path[pathPos + 1] == '.') && ((path[pathPos + 2] == '\0') || (path[pathPos + 2] == '/'))){
          // Double dot entry
          name[namePos++] = '.';
          name[namePos++] = '.';
          pathPos+=2;
          }else{
          // Copy name from path
          bool dotSeen=false;
          bool flagLFN=false;
          if (path[pathPos] == '.') flagLFN = true;
          while (1) {
            if((path[pathPos] == '\0') || (path[pathPos] == '/')) break;
            
            name[namePos] = ucase(path[pathPos]);
            if ((name[namePos] <= ' ') || ((name[namePos] >= ':') && (name[namePos] <= '?'))) flagLFN = true; // Invalid character
            if (name[namePos] == '.') {
              if (!dotSeen) {
                dotSeen = true;
                }else{
                flagLFN = true;
              }
            }
            namePos++;
            if(namePos==MAX_FILENAME_LENGTH) StopFatalError(12302,"name buffer overflow.\n");
            pathPos++;
          }
          // Check if a long filename was specified
          if (namePos > 12) flagLFN = true;
          if(flagLFN==true) StopFatalError(12303,"Not support find from LFN path.\n");
        }
      }
      
      // Add end of string char. max length is 13chars.
      name[namePos] = '\0';
      // _consolePrintf("Target name= %s\n",name);
    }
    
    // Move through path to correct place
    while ((path[pathPos] != '/') && (path[pathPos] != '\0')){
      pathPos++;
    }
    // Eat any slash /
    while ((path[pathPos] == '/') && (path[pathPos] != '\0')){
      pathPos++;
    }

    // Search current Dir for correct entry
    DIR_ENT *dirEntry = FAT_GetAllDirEntry (dirCluster, 1, SEEK_SET);
    LFNTopCluster=dirCluster;
    LFNTopIndex=(u32)-1;
    
    u32 CurEntryIndex=0;
    u32 LFNCheckSum=(u32)-1;
    
    while (1) {
      if(dirEntry==NULL){ // Couldn't find specified file
        StopFatalError(12304,"Not found dir entry.\n");
      }
      
      // Match filename
      bool found=false;

      // Check against alias as well.
      if ((dirEntry->name[0] != FILE_FREE) && ((dirEntry->name[0] > 0x20) || (dirEntry->name[0] == 0x05)) && ((dirEntry->attrib & ATTRIB_VOL) != ATTRIB_VOL) && ((dirEntry->attrib & ATTRIB_LFN) != ATTRIB_LFN)){
        u8 CheckSum=FAT_DirEntFromPath_LFNTopDirEntry_ins_GetCheckSum(dirEntry);
        // _consolePrintf("Check LFN %x,%x\n",CheckSum,LFNCheckSum);
        if(CheckSum!=LFNCheckSum){
          LFNCheckSum=(u32)-1;
          LFNTopIndex=(u32)-1;
        }
        char alias[13];
        FAT_GetFilename(*dirEntry, alias);
        found=true;
        ucase_start();
        u32 namePos;
        for (namePos = 0; (namePos < 13) && found && (name[namePos] != '\0') && (alias[namePos] != '\0'); namePos++){
          if (name[namePos] != ucase(alias[namePos])) found = false;
        }
        if ((name[namePos] == '\0') != (alias[namePos] == '\0')) found = false;
      }
      
      if(found==true) break;
      
      if((dirEntry->attrib & ATTRIB_LFN)==ATTRIB_LFN){
        DIR_ENT_LFN *lfn = (DIR_ENT_LFN*)dirEntry;
        if(LFNCheckSum!=lfn->checkSum){
          LFNTopIndex=CurEntryIndex;
          LFNCheckSum=lfn->checkSum;
        }
        }else{
        LFNTopIndex=(u32)-1;
        LFNCheckSum=(u32)-1;
      }
      
      CurEntryIndex++;
      dirEntry = FAT_GetAllDirEntry (dirCluster, 1, SEEK_CUR);
    }
    
    if((dirEntry->name[0] != FILE_FREE) && ((dirEntry->attrib & ATTRIB_DIR)!=ATTRIB_DIR)){
      if(path[pathPos] != '\0') StopFatalError(12305,"found path is dir. [%s, %d]\n",path,pathPos);
      // Founded target.
      break;
    }
    
    // It has found a directory from within the path that needs to be followed
    dirCluster = dirEntry->startCluster | (dirEntry->startClusterHigh << 16);
  }
  
  // Back to LFN entry.
  if(LFNTopIndex==(u32)-1){
    _consolePrint("FAT_DirEntFromPath_LFNTopDirEntry: Not exists LFN entry.\n");
    DIR_ENT dirEntry;
    dirEntry.name[0] = FILE_FREE;
    dirEntry.attrib = 0x00;
    return (dirEntry);
  }
  
  DIR_ENT *dirEntry = FAT_GetAllDirEntry (LFNTopCluster, 1, SEEK_SET);
  for(u32 idx=0;idx<LFNTopIndex;idx++){
    dirEntry=FAT_GetAllDirEntry(dirCluster, 1, SEEK_CUR);
  }
  
  return (*dirEntry);
}
