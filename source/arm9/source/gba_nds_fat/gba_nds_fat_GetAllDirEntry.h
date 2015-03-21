
static DIR_ENT* FAT_GetAllDirEntry ( u32 dirCluster, int entry, int origin)
{
  static DIR_ENT dir;
  int firstSector = 0;
  bool found = false;
  int maxSectors;
  int lfnPos, aliasPos;
  u8 lfnChkSum, chkSum;

  dir.name[0] = FILE_FREE; // default to no file found
  dir.attrib = 0x00;

  // Check if fat has been initialised
  if (filesysBytePerSec == 0)
  {
    return (NULL);
  }
  
  switch (origin) 
  {
  case SEEK_SET:
    wrkDirCluster = dirCluster;
    wrkDirSector = 0;
    wrkDirOffset = -1;
    break;
  case SEEK_CUR:  // Don't change anything
    break;
  case SEEK_END:  // Find entry signifying end of directory
    // Subtraction will never reach 0, so it keeps going 
    // until reaches end of directory
    wrkDirCluster = dirCluster;
    wrkDirSector = 0;
    wrkDirOffset = -1;
    entry = -1;
    break;
  default:
    return NULL;
  }

  lfnChkSum = 0;
  maxSectors = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? (filesysData - filesysRootDir) : filesysSecPerClus);

  // Scan Dir for correct entry
  firstSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster));
  if(wrkDirOffset==-1) disc_SystemReadSector (firstSector + wrkDirSector, globalBuffer);
  found = false;
  do {
    wrkDirOffset++;
    if (wrkDirOffset == BYTE_PER_READ / sizeof (DIR_ENT))
    {
      wrkDirOffset = 0;
      wrkDirSector++;
      if ((wrkDirSector == filesysSecPerClus) && (wrkDirCluster != FAT16_ROOT_DIR_CLUSTER))
      {
        wrkDirSector = 0;
        wrkDirCluster = FAT_NextCluster(wrkDirCluster);
        if (wrkDirCluster == CLUSTER_EOF) return(NULL); // not found.
        firstSector = FAT_ClustToSect(wrkDirCluster);    
      }
      else if ((wrkDirCluster == FAT16_ROOT_DIR_CLUSTER) && (wrkDirSector == (filesysData - filesysRootDir)))
      {
        return(NULL);  // Got to end of root dir
      }
      disc_SystemReadSector (firstSector + wrkDirSector, globalBuffer);
    }
    dir = ((DIR_ENT*) globalBuffer)[wrkDirOffset];
    
    if ((dir.name[0] != FILE_FREE) && ((dir.name[0] > 0x20) || (dir.name[0] == 0x05)) && ((dir.attrib & ATTRIB_VOL) != ATTRIB_VOL) && ((dir.attrib & ATTRIB_LFN) != ATTRIB_LFN))
    {
      entry--;
      if (entry == 0) found = true;
    }
    else if (dir.name[0] == FILE_LAST)
    {
      if (origin == SEEK_END)
      {
        found = true;
      }
      else
      {
        return(NULL);
      }
    }
    else if (dir.attrib == ATTRIB_LFN)
    {
      entry--;
      if (entry == 0) found = true;
    }
  } while (!found);
  
  return (&dir);
}
