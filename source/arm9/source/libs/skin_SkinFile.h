
static const char SkinFileID[]="Skin files package for FishShell2 type.4\0\0";

static bool SkinFile_Check(const UnicodeChar *pSkinFilenameUnicode)
{
    if(VerboseDebugLog==true) _consolePrint("SkinFile_Check(...);\n");
  
  if(Unicode_isEmpty(pSkinFilenameUnicode)==true) return(false);
  
  if(FullPath_FileExistsUnicode(pSkinFilenameUnicode)==false){
    _consolePrint("Skin file not found.\n");
    return(false);
  }
  
  FAT_FILE *pSkinFile=Shell_FAT_fopen_FullPath(pSkinFilenameUnicode);
  
  if(pSkinFile==NULL){
    _consolePrint("Open file failed.\n");
    return(false);
  }
  
  CStreamFS *pSkinFileStream=new CStreamFS(pSkinFile);

  if(pSkinFileStream==NULL){
    _consolePrint("Can not create file stream.\n");
    FAT2_fclose(pSkinFile);
    return(false);
  }
  
  {
    u32 IDLength=pSkinFileStream->Readu8();
    if(64<IDLength) IDLength=64;
    char chkid[64+1];
    pSkinFileStream->ReadBuffer(chkid,IDLength+1);
    
    if(isStrEqual(SkinFileID,chkid)==false){
      _consolePrintf("Illigal ID. [%s]\n",chkid);
      _consolePrintf("Request ID. [%s]\n",SkinFileID);
      delete pSkinFileStream; pSkinFileStream=NULL;
      FAT2_fclose(pSkinFile);
      return(false);
    }
  }
  
  delete pSkinFileStream; pSkinFileStream=NULL;
  FAT2_fclose(pSkinFile);
  
  return(true);
}

typedef struct {
  u32 FilenameOffset;
  u32 DecompSize;
  u32 CompSize;
  u32 CompDataOffset;
} TSkinFileInfo;

typedef struct {
  void *pData;
  u32 DataSize;
} TSkinFileCache;

DATA_IN_AfterSystem static FAT_FILE *pSkinFile=NULL;
DATA_IN_AfterSystem static CStreamFS *pSkinFileStream=NULL;
DATA_IN_AfterSystem static u32 SkinFileCount=0;
DATA_IN_AfterSystem static TSkinFileInfo *pSkinFileInfo=NULL;
DATA_IN_AfterSystem static TSkinFileCache *pSkinFileCache=NULL;

static bool SkinFile_Open(const UnicodeChar *pSkinFilenameUnicode)
{
  extmem_Start();
  
  if(VerboseDebugLog==true) _consolePrint("SkinFile_Open(...);\n");
  
  if(Unicode_isEmpty(pSkinFilenameUnicode)==true) return(false);
  
  if(FullPath_FileExistsUnicode(pSkinFilenameUnicode)==false){
    _consolePrint("Skin file not found.\n");
    return(false);
  }
  
  pSkinFile=Shell_FAT_fopen_FullPath(pSkinFilenameUnicode);
  
  if(pSkinFile==NULL){
    _consolePrint("Open file failed.\n");
    return(false);
  }
  
  pSkinFileStream=new CStreamFS(pSkinFile);

  if(pSkinFileStream==NULL){
    _consolePrint("Can not create file stream.\n");
    FAT2_fclose(pSkinFile);
    return(false);
  }
  
  {
    u32 IDLength=pSkinFileStream->Readu8();
    if(64<IDLength) IDLength=64;
    char chkid[64+1];
    pSkinFileStream->ReadBuffer(chkid,IDLength+1);
    
    if(isStrEqual(SkinFileID,chkid)==false){
      _consolePrintf("Illigal ID. [%s]\n",chkid);
      _consolePrintf("Request ID. [%s]\n",SkinFileID);
      delete pSkinFileStream; pSkinFileStream=NULL;
      FAT2_fclose(pSkinFile);
      return(false);
    }
  }
  
  SkinFileCount=pSkinFileStream->Readu32();
  
  u32 HeaderSize=pSkinFileStream->Readu32();
  if(VerboseDebugLog==true) _consolePrintf("Header size=0x%x\n",HeaderSize);
  
  u32 ItemMaxCount=280;
  u32 HeaderMaxSize=(ItemMaxCount*sizeof(TSkinFileInfo))+2048; // with filename buffer.
  if(HeaderMaxSize<HeaderSize) StopFatalError(14401,"Skin header size too long. %d,%d\n",HeaderSize,HeaderMaxSize);
  pSkinFileInfo=(TSkinFileInfo*)safemalloc_chkmem(&MM_SystemAfter,HeaderMaxSize);
  pSkinFileCache=(TSkinFileCache*)safemalloc_chkmem(&MM_SystemAfter,ItemMaxCount*sizeof(TSkinFileCache));
  pSkinFileStream->ReadBuffer(pSkinFileInfo,HeaderSize);
  MemSet32CPU(0,pSkinFileCache,ItemMaxCount*sizeof(TSkinFileCache));
  
  _consolePrintf("Skin file size: %dbyte.\n",pSkinFileStream->GetSize());
  
  return(true);
}

static void SkinFile_Close(void)
{
  if(VerboseDebugLog==true) _consolePrint("SkinFile_Close();\n");
  
  if(pSkinFileStream!=NULL){
    delete pSkinFileStream; pSkinFileStream=NULL;
  }
  
  if(pSkinFile!=NULL){
    FAT2_fclose(pSkinFile); pSkinFile=NULL;
  }
  
  if(pSkinFileInfo!=NULL){
    safefree(&MM_SystemAfter,pSkinFileInfo); pSkinFileInfo=NULL;
  }
  if(pSkinFileCache!=NULL){
    safefree(&MM_SystemAfter,pSkinFileCache); pSkinFileCache=NULL;
  }
  
  SkinFileCount=0;
}

static u32 SkinFile_GetFileIndexFromFilename(const char *pFilename)
{
  u32 FilenameLength=strlen(pFilename);
  
  for(u32 idx=0;idx<SkinFileCount;idx++){
    TSkinFileInfo *psfi=&pSkinFileInfo[idx];
    
    char *pfn=(char*)pSkinFileInfo;
    pfn=&pfn[psfi->FilenameOffset];
    
    u32 fnlen=(u8)*pfn;
    pfn++;
    
//    _consolePrintf("%d %d %d %s\n",psfi->FilenameOffset,idx,fnlen,pfn);
    
    if(FilenameLength==fnlen){
      if(isStrEqual(pFilename,pfn)==true) return(idx);
    }
  }
  
  return((u32)-1);
}

static void SkinFile_LoadFileAllocate(u32 SkinFileIndex,void **pbuf,s32 *psize)
{
	Splash_Update();
	
  TSkinFileInfo *psfi=&pSkinFileInfo[SkinFileIndex];
  
  void *pdummy=safemalloc_chkmem(&MM_Temp,psfi->DecompSize*2);
  
  if(0){
    u32 tmpbufsize=psfi->DecompSize;
    u8 *ptmpbuf=(u8*)safemalloc_chkmem(&MM_Temp,tmpbufsize);
    
    safefree(&MM_Temp,pdummy); pdummy=NULL;
    
    pSkinFileStream->SetOffset(psfi->CompDataOffset);  
    pSkinFileStream->ReadBuffer(ptmpbuf,tmpbufsize);
    
    *pbuf=ptmpbuf;
    *psize=tmpbufsize;
    return;
  }
  
  TZLIBData zd;
  
  zd.DstSize=psfi->DecompSize;
  zd.pDstBuf=(u8*)safemalloc_chkmem(&MM_Temp,zd.DstSize);
  zd.SrcSize=psfi->CompSize;
  zd.pSrcBuf=(u8*)safemalloc_chkmem(&MM_Temp,zd.SrcSize);
  
  safefree(&MM_Temp,pdummy); pdummy=NULL;
  
  pSkinFileStream->SetOffset(psfi->CompDataOffset);  
  pSkinFileStream->ReadBuffer(zd.pSrcBuf,zd.SrcSize);
  
  Splash_Update();
  
  if((zd.pSrcBuf[0]==0x78)&&(zd.pSrcBuf[1]==0x01)){
    if(zlibdecompress(&zd)==false) StopFatalError(14404,"ZLIB decompress error.\n");
    }else{
    MemCopy32CPU(zd.pSrcBuf,zd.pDstBuf,zd.DstSize);
  }
  
  Splash_Update();
  
  zd.SrcSize=0;
  safefree(&MM_Temp,zd.pSrcBuf); zd.pSrcBuf=NULL;
  
  *pbuf=zd.pDstBuf;
  *psize=zd.DstSize;
}

static void SkinFile_LoadB15(const char *pFilename,CglB15 **ppB15)
{
//  _consolePrintf("Load skin %s.\n",pFilename);
  
  u32 fidx=SkinFile_GetFileIndexFromFilename(pFilename);
  
  if(fidx==(u32)-1) StopFatalError(14405,"Skin file B15 '%s' not found.\n",pFilename);
  
  TSkinFileCache *psfc=&pSkinFileCache[fidx];
  
  if(psfc->pData!=NULL){
    *ppB15=new CglB15(&MM_Skin,(u8*)psfc->pData,psfc->DataSize);
    return;
  }
  
  u8 *pbuf=NULL;
  s32 bufsize=0;
  
  SkinFile_LoadFileAllocate(fidx,(void**)&pbuf,&bufsize);
  *ppB15=new CglB15(&MM_Skin,pbuf,bufsize);
  
  psfc->pData=extmem_malloc(bufsize);
  if(psfc->pData!=NULL){
    psfc->DataSize=bufsize;
    MemCopy32CPU(pbuf,psfc->pData,(psfc->DataSize+3)&~3);
  }
  
  safefree(&MM_Temp,pbuf); pbuf=NULL;
}

static void SkinFile_LoadTGF(const char *pFilename,CglTGF **ppTGF)
{
//  _consolePrintf("Load skin %s.\n",pFilename);
  
  u32 fidx=SkinFile_GetFileIndexFromFilename(pFilename);
  
  if(fidx==(u32)-1) StopFatalError(14406,"Skin file TGF '%s' not found.\n",pFilename);
  
  TSkinFileCache *psfc=&pSkinFileCache[fidx];
  
  if(psfc->pData!=NULL){
    *ppTGF=new CglTGF(&MM_Skin,(u8*)psfc->pData,psfc->DataSize);
    return;
  }
  
  u8 *pbuf=NULL;
  s32 bufsize=0;
  
  SkinFile_LoadFileAllocate(fidx,(void**)&pbuf,&bufsize);
  *ppTGF=new CglTGF(&MM_Skin,pbuf,bufsize);
  
  psfc->pData=extmem_malloc(bufsize);
  if(psfc->pData!=NULL){
    psfc->DataSize=bufsize;
    MemCopy32CPU(pbuf,psfc->pData,(psfc->DataSize+3)&~3);
  }
  
  safefree(&MM_Temp,pbuf); pbuf=NULL;
}

