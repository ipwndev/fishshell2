
FAT_FILE* FAT2_fopen_CurrentForRead(void)
{
  int fileNum;
  FAT_FILE* file;
  DIR_ENT dirEntry;

  dirEntry = ((DIR_ENT*) globalBuffer)[wrkDirOffset];
  
  // Find a free file buffer
  for (fileNum = 0; (fileNum < MAX_FILES_OPEN) && (openFiles[fileNum].inUse == true); fileNum++);
  
  if (fileNum == MAX_FILES_OPEN) StopFatalError(12201,"Can not open. not allocate files area.\n");
  
  file = &openFiles[fileNum];
  // Remember where directory entry was
  file->dirEntSector = (wrkDirCluster == FAT16_ROOT_DIR_CLUSTER ? filesysRootDir : FAT_ClustToSect(wrkDirCluster)) + wrkDirSector;
  file->dirEntOffset = wrkDirOffset;

  { // if ( strpbrk(mode, "rR") != NULL )  //(ucase(mode[0]) == 'R')
    if (dirEntry.name[0] == FILE_FREE)  // File must exist
    {
      StopFatalError(12202,"Directory entry error. [FILE_FREE]\n");
    }
    
    file->read = true;
    file->write = false;
    file->append = false;
    
    // Store information about position within the file, for use
    // by FAT_fread, FAT_fseek, etc.
    file->firstCluster = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);
  
    file->length = dirEntry.fileSize;
    file->curPos = 0;
    file->curClus = dirEntry.startCluster | (dirEntry.startClusterHigh << 16);
    file->curSect = 0;
    file->curByte = 0;

    // Not appending
    file->appByte = 0;
    file->appClus = 0;
    file->appSect = 0;
  
    disc_ReadSector( FAT_ClustToSect( file->curClus), file->readBuffer);
    file->inUse = true;  // We're using this file now

    return file;
  }  // mode "r"

}

