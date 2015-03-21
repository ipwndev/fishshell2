
static u32 ClusterBuffer[32*1024/4]; // max ClusterSize=32kb

static bool is64kCluster=true; // startup default is slow mode.

//#define CheckMemAlign(ptr) ((u32)ptr&1)
#define CheckMemAlign(ptr) (1)

extern void FAT_Disabled64kClusterMode(void);
DATA_IN_IWRAM_MainPass void FAT_Disabled64kClusterMode(void)
{
  if((32*1024/512)<filesysSecPerClus) StopFatalError(12401,"The cluster size that exceeds 32kbyte is not supported.\n");
  
  is64kCluster=false;
  
  _consolePrint("FAT2: Disabled 64k cluster mode. Start fast mode.\n");
}

u32 FAT_fread (void* buffer, u32 size, u32 count, FAT_FILE* file)
{
  int curByte;
  int curSect;
  u32 curClus;
  u32 tempNextCluster;
  
  int tempVar;

  char* data = (char*)buffer;

  u32 length = size * count;
  u32 remain;

  bool flagNoError = true;

  // Can't read non-existant files
  if ((file == NULL) || (file->inUse == false)) StopFatalError(12402,"Can not read. not opened.\n");
  if (length == 0 || buffer == NULL) StopFatalError(12403,"Can not read. Illigal size or buffer. (%d,%x)\n",length,buffer);

  // Can only read files openned for reading
  if (!file->read) StopFatalError(12404,"Can not read. for not read.\n");

  // Don't read past end of file
  if (length + file->curPos > file->length)
    length = file->length - file->curPos;

  remain = length;

  curByte = file->curByte;
  curSect = file->curSect;
  curClus = file->curClus;

  // Align to sector
  tempVar = BYTE_PER_READ - curByte;
  if (tempVar > remain)
    tempVar = remain;

  if ((tempVar < BYTE_PER_READ) && flagNoError) 
  {
    MemCopy8CPU(&(file->readBuffer[curByte]), data, tempVar);
    remain -= tempVar;
    data += tempVar;

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
    if(CheckMemAlign(data)==0){
      disc_ReadSectors ( curSect + FAT_ClustToSect(curClus), tempVar, data);
      }else{
      if(is64kCluster==false){
        disc_ReadSectors ( curSect + FAT_ClustToSect(curClus), tempVar, ClusterBuffer);
        MemCopy8CPU(ClusterBuffer, data, tempVar * BYTE_PER_READ);
        }else{
        for(u32 idx=0;idx<tempVar;idx++){
          disc_ReadSector ( curSect + FAT_ClustToSect(curClus) + idx, ClusterBuffer);
          MemCopy8CPU(ClusterBuffer, &data[BYTE_PER_READ*idx], BYTE_PER_READ);
        }
      }
    }
    
    data += tempVar * BYTE_PER_READ;
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
    if(CheckMemAlign(data)==0){
      disc_ReadSectors (FAT_ClustToSect(curClus), filesysSecPerClus, data);
      }else{
      if(is64kCluster==false){
        disc_ReadSectors (FAT_ClustToSect(curClus), filesysSecPerClus, ClusterBuffer);
        MemCopy8CPU(ClusterBuffer, data, filesysBytePerClus);
        }else{
        for(u32 idx=0;idx<filesysSecPerClus;idx++){
          disc_ReadSector ( FAT_ClustToSect(curClus) + idx, ClusterBuffer);
          MemCopy8CPU(ClusterBuffer, &data[BYTE_PER_READ*idx], BYTE_PER_READ);
        }
      }
    }
    
    data += filesysBytePerClus;
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
    if(CheckMemAlign(data)==0){
      disc_ReadSectors (FAT_ClustToSect(curClus), tempVar, data);
      }else{
      if(is64kCluster==false){
        disc_ReadSectors (FAT_ClustToSect(curClus), tempVar, ClusterBuffer);
        MemCopy8CPU(ClusterBuffer, data, tempVar * BYTE_PER_READ);
        }else{
        for(u32 idx=0;idx<tempVar;idx++){
          disc_ReadSector ( FAT_ClustToSect(curClus) + idx, ClusterBuffer);
          MemCopy8CPU(ClusterBuffer, &data[BYTE_PER_READ*idx], BYTE_PER_READ);
        }
      }
    }
    
    data += tempVar * BYTE_PER_READ;
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
      MemCopy8CPU(file->readBuffer, data, remain);
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
