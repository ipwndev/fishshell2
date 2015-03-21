
u32 FAT_fskip (u32 size, u32 count, FAT_FILE* file)
{
  int curByte;
  int curSect;
  u32 curClus;
  u32 tempNextCluster;
  
  int tempVar;

  u32 length = size * count;
  u32 remain;

  bool flagNoError = true;

  // Can't read non-existant files
  if ((file == NULL) || (file->inUse == false)) StopFatalError(12601,"Can not read. not opened.\n");
  if (length == 0) StopFatalError(12602,"Can not read. Illigal size. (%d)\n",length);

  // Can only read files openned for reading
  if (!file->read) StopFatalError(12603,"Can not read. for not read.\n");

  // Don't read past end of file
  if (length + file->curPos > file->length) length = file->length - file->curPos;

  remain = length;

  curByte = file->curByte;
  curSect = file->curSect;
  curClus = file->curClus;

  // Align to sector
  tempVar = BYTE_PER_READ - curByte;
  if (tempVar > remain) tempVar = remain;

  if ((tempVar < BYTE_PER_READ) && flagNoError) 
  {
    remain -= tempVar;

    curByte += tempVar;
    if (curByte >= BYTE_PER_READ) 
    {
      curByte = 0;
      curSect++;
    }
  }

  // align to cluster
  // tempVar is number of sectors to read
  if (remain > (filesysSecPerClus - curSect) * BYTE_PER_READ) 
  {
    tempVar = filesysSecPerClus - curSect;
  } else {
    tempVar = remain / BYTE_PER_READ;
  }

  if ((tempVar > 0) && flagNoError)
  {
//    disc_ReadSectors ( curSect + FAT_ClustToSect(curClus), tempVar, data);

    remain -= tempVar * BYTE_PER_READ;

    curSect += tempVar;
  }

  // Move onto next cluster
  // It should get to here without reading anything if a cluster is due to be allocated
  if (curSect >= filesysSecPerClus)
  {
    tempNextCluster = FAT_NextCluster(curClus);
    if ((remain == 0) && (tempNextCluster == CLUSTER_EOF))
    {
      curSect = filesysSecPerClus;
    } else {
      curSect = 0;
      curClus = tempNextCluster;
      if (curClus == CLUSTER_FREE)
      {
        flagNoError = false;
      }
    }
  }

  // Read in whole clusters
  while ((remain >= filesysBytePerClus) && flagNoError)
  {
//    disc_ReadSectors (FAT_ClustToSect(curClus), filesysSecPerClus, data);

    remain -= filesysBytePerClus;

    // Advance to next cluster
    tempNextCluster = FAT_NextCluster(curClus);
    if ((remain == 0) && (tempNextCluster == CLUSTER_EOF))
    {
      curSect = filesysSecPerClus;
    } else {
      curSect = 0;
      curClus = tempNextCluster;
      if (curClus == CLUSTER_FREE)
      {
        flagNoError = false;
      }
    }
  }

  // Read remaining sectors
  tempVar = remain / BYTE_PER_READ; // Number of sectors left
  if ((tempVar > 0) && flagNoError)
  {
//    disc_ReadSectors (FAT_ClustToSect(curClus), tempVar, data);

    remain -= tempVar * BYTE_PER_READ;
    curSect += tempVar;
  }

  // Last remaining sector
  // Check if sector wanted is different to the one started with
  if ( ((file->curByte + length) >= BYTE_PER_READ) && flagNoError)
  {
    disc_ReadSector( curSect + FAT_ClustToSect( curClus), file->readBuffer);
    if (remain > 0)
    {
//      MemCopy8CPU(file->readBuffer, data, remain);
      curByte += remain;
      remain = 0;
    }
  }

  // Length read is the wanted length minus the stuff not read
  length = length - remain;

  // Update file information
  file->curByte = curByte;
  file->curSect = curSect;
  file->curClus = curClus;
  file->curPos = file->curPos + length;
  
  return length;
}

#undef CheckMemAlign
