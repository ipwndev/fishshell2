
#define MaxFoldersCount (128)

// ----------------------------------------------------

#define chkdsk_MsgOfsY (8+(16*5))

static void chkdsk_ExeclusiveWaitForPressButton(void)
{
  u16 KEYS_CUR;
  
  KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  while(KEYS_CUR!=0){
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
  while(KEYS_CUR==0){
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
  while(KEYS_CUR!=0){
      KEYS_CUR=(~REG_KEYINPUT)&0x3ff;
  }
}

enum EFatalErrorType {EFET_DuplicatedArea,EFET_BrokenEntry,EFET_IlligalClusterLink,EFET_BrokenDirectoryLink,EFET_BrokenUnicode,EFET_FileSizeError};

static void chkdsk_DuplicateCluster_ShowFatalError(EFatalErrorType FatalErrorType)
{
  CglCanvas *pCanvas=pScreenMain->pBackCanvas;
  
  pCanvas->FillFull(GlobalBGColor15);
  
  u32 x=8,y=8;
  
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_Title"));
  y+=16;
  y+=16;
  switch(FatalErrorType){
    case EFET_DuplicatedArea: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_DuplicatedArea0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_DuplicatedArea1"));
      y+=16;
    } break;
    case EFET_BrokenEntry: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenEntry0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenEntry1"));
      y+=16;
    } break;
    case EFET_IlligalClusterLink: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_IlligalClusterLink0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_IlligalClusterLink1"));
      y+=16;
    } break;
    case EFET_BrokenDirectoryLink: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenDirectoryLink0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenDirectoryLink1"));
      y+=16;
    } break;
    case EFET_BrokenUnicode: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenUnicode0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_BrokenUnicode1"));
      y+=16;
    } break;
    case EFET_FileSizeError: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_FileSizeError0"));
      y+=16;
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_FileSizeError1"));
      y+=16;
    } break;
    default: {
      pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_UnknownError"));
      y+=16;
    } break;
  }
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_RecoveryMsg0"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_RecoveryMsg1"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_RecoveryMsg2"));
  y+=16;
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_StopApplication"));
  
  pScreenMain->Flip(true);
  Sound_Start(WAVFN_Notify);
  
  StopFatalError(16401,"Error found in checking duplicate cluster.\n");
}

static void chkdsk_DuplicateCluster_ShowBufferOverflow(void)
{
  CglCanvas *pCanvas=pScreenMain->pBackCanvas;
  
  pCanvas->FillFull(GlobalBGColor15);
  
  u32 x=8,y=8;
  
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_Title"));
  y+=16;
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_Msg0"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_Msg1"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_Msg2"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_RecoveryMsg0"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_RecoveryMsg1"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_RecoveryMsg2"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_NotSupportDiskSize_RecoveryMsg3"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_StopApplication"));
  
  pScreenMain->Flip(true);
  Sound_Start(WAVFN_Notify);
  
  StopFatalError(16402,"Not support disk size or cluster size.\n");
}

// ----------------------------------------------------

static void chkdsk_DuplicateCluster_ShowProgress(float CurrentPer,u32 CheckedFilesCount,const char *CheckFileName)
{
  CglCanvas *pCanvas=pScreenMain->pViewCanvas;
  
  char str[64];
  
  snprintf(str,64,"progress... %d%% %dfiles",(u32)(CurrentPer*100),CheckedFilesCount);
  if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("%s\n",str);
  
  pCanvas->SetColor(GlobalBGColor15);
  pCanvas->FillBox(16,chkdsk_MsgOfsY+(16*2),ScreenWidth-16,16);
  pCanvas->TextOutA(16,chkdsk_MsgOfsY+(16*2),str);
  
  strncpy(str,CheckFileName,60);
  str[11]='\0';
  
  pCanvas->TextOutA(ScreenWidth-90,chkdsk_MsgOfsY+(16*2),str);
}

static void chkdsk_DuplicateCluster_ClearProgress(void)
{
  CglCanvas *pCanvas=pScreenMain->pViewCanvas;
  
  pCanvas->SetColor(GlobalBGColor15);
  pCanvas->FillBox(16,chkdsk_MsgOfsY+(16*2),ScreenWidth-16,16);
}

#define ClusterFlagsSize ((1*1024*1024)/4)
DATA_IN_IWRAM_ChkDsk static u32 *pClusterFlags;

DATA_IN_IWRAM_ChkDsk static u32 FragmentFileCount,FragmentCount;

static void chkdsk_DuplicateCluster_Init(void)
{
  pClusterFlags=(u32*)safemalloc_chkmem(&MM_Process,ClusterFlagsSize*4);
  
  MemSet32CPU(0,pClusterFlags,ClusterFlagsSize*4);
  
  FragmentFileCount=0;
  FragmentCount=0;
}

static void chkdsk_DuplicateCluster_SetFlag(u32 Cluster)
{
  if(ClusterFlagsSize<(Cluster/32)){
    _consolePrint("File systerm error. Cluster check buffer overflow.\n");
    chkdsk_DuplicateCluster_ShowBufferOverflow();
    return;
  }
  
  u32 flags=pClusterFlags[Cluster/32];
  
  u32 bit=1<<(Cluster&31);
  
  if((flags&bit)!=0){
    _consolePrint("File systerm error. Duplicate cluster link finded.\n");
    chkdsk_DuplicateCluster_ShowFatalError(EFET_DuplicatedArea);
    return;
  }
  
  flags|=bit;
  
  pClusterFlags[Cluster/32]=flags;
}

static void chkdsk_DuplicateCluster_Free(void)
{
  if(pClusterFlags!=NULL){
    safefree(&MM_Process,pClusterFlags); pClusterFlags=NULL;
  }
}

#define CLUSTER_FREE	0x0000
#define	CLUSTER_EOF	0x0FFFFFFF

static void chkdsk_DuplicateCluster_Current_FillUsedCluster(const char *pAlias)
{
  bool chkfragment=false;
  u32 fragment=0;
  
  u32 FileSize=FAT2_CurEntry_GetFileSize();
  
  u32 ClusterSize=FAT2_GetSecPerClus()*512;
  u32 UsedClusterCount=(FileSize+(ClusterSize-1))/ClusterSize;
  u32 ClusterCount=0;
  
  u32 CurClus=FAT2_GetFirstCluster();
  
  Splash_Update();
  
  if((CurClus!=CLUSTER_FREE)&&(CurClus!=CLUSTER_EOF)){
    while(1){
      
      chkdsk_DuplicateCluster_SetFlag(CurClus);
      
      ClusterCount++;
      
      u32 NextClus=FAT2_NextCluster(CurClus);
      if(NextClus==CLUSTER_EOF) break;
      
      if((CurClus+1)!=NextClus) fragment++;
      
      CurClus=NextClus;
      
      if(CurClus==CLUSTER_FREE){
        _consolePrint("File systerm error. Can not search cluster link.\n");
        chkdsk_DuplicateCluster_ShowFatalError(EFET_IlligalClusterLink);
        return;
      }
    }
  }
  
  if(fragment!=0){
      if(VerboseDebugLog==true) _consolePrintf("[%s] %dfragments.\n",pAlias,fragment);
    if(chkfragment==true){
      FragmentFileCount++;
      FragmentCount+=fragment;
    }
  }
  
  if(UsedClusterCount==0){
    if(ClusterCount==1) ClusterCount=0;
  }
  
  if(UsedClusterCount!=ClusterCount){
    _consolePrintf("[%s]\n",pAlias);
    _consolePrintf("Entry clusters: %d, Actual clusters:%d.\n",UsedClusterCount,ClusterCount);
    _consolePrint("A size that the directory entry shows and an actual size of the file are different.\n");
    chkdsk_DuplicateCluster_ShowFatalError(EFET_FileSizeError);
  }
}

DATA_IN_IWRAM_ChkDsk static u32 CheckedFilesCount;

static void chkdsk_DuplicateCluster_ins(float CurrentPer,float DivPer)
{
  u32 FolderCount=0;
  char *pFolderNameAlias=(char*)safemalloc_chkmem(&MM_Temp,MaxFoldersCount*(12+1));
  
  {
    DATA_IN_IWRAM_ChkDsk static const char *pafn;
    DATA_IN_IWRAM_ChkDsk static const UnicodeChar *pufn;
    
    u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
    
    while(FAT_FileType!=FT_NONE){
      Splash_Update(); 
      switch(FAT_FileType){
        case FT_NONE: break;
        case FT_DIR: {
          if((strcmp(pafn,".")!=0)&&(strcmp(pafn,"..")!=0)){
            pufn=FAT2_GetLongFilenameUnicode();
            if(pufn==NULL){
              _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
              chkdsk_DuplicateCluster_ShowFatalError(EFET_BrokenUnicode);
              return;
            }
            strcpy(&pFolderNameAlias[FolderCount*(12+1)],pafn);
            FolderCount++;
          }
        } break;
        case FT_FILE: {
          pufn=FAT2_GetLongFilenameUnicode();
          if(pufn==NULL){
            _consolePrintf("Unicode filename read error.\n Alias='%s'\n",pafn);
            chkdsk_DuplicateCluster_ShowFatalError(EFET_BrokenUnicode);
            return;
          }
          if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("Check file [%s]\n",pafn);
          chkdsk_DuplicateCluster_Current_FillUsedCluster(pafn);
          if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("Filled file [%s]\n",pafn);
          CheckedFilesCount++;
          if(6<=VBlankPassedCount){
            VBlankPassedCount=0;
            chkdsk_DuplicateCluster_ShowProgress(CurrentPer,CheckedFilesCount,pafn);
          }
        } break;
      }
      
      FAT_FileType=FAT2_FindNextFile(&pafn);
    }
  }
  
  DivPer/=FolderCount;
  
  for(u32 idx=0;idx<FolderCount;idx++){
    const char *ptagfolder=&pFolderNameAlias[idx*(12+1)];
    if(FAT2_chdir_Alias(ptagfolder)==false){
      _consolePrintf("Can not chdir to %s.\n",&pFolderNameAlias[idx*(12+1)]);
      chkdsk_DuplicateCluster_ShowFatalError(EFET_BrokenDirectoryLink);
      return;
    }
    if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("----- Start folder [%s]\n",ptagfolder);
    chkdsk_DuplicateCluster_ins(CurrentPer,DivPer);
    if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("----- End folder. [%s]\n",ptagfolder);
    CurrentPer+=DivPer;
    if(FAT2_chdir_Alias("..")==false){
      _consolePrint("Can not chdir to up.\n");
      chkdsk_DuplicateCluster_ShowFatalError(EFET_BrokenDirectoryLink);
      return;
    }
  }
  
  if(pFolderNameAlias!=NULL){
    safefree(&MM_Temp,pFolderNameAlias); pFolderNameAlias=NULL;
  }
}

static void chkdsk_DuplicateCluster(void)
{
  {
    CglCanvas *pCanvas=pScreenMain->pViewCanvas;
    pCanvas->TextOutUTF8(16,chkdsk_MsgOfsY+(16*1),Lang_GetUTF8("CD_CheckDuplicateCluster_Title"));
  }
  
  if(FAT2_chdir_Alias("/")==false){
    _consolePrint("Can not chdir to root.\n");
    chkdsk_DuplicateCluster_ShowFatalError(EFET_BrokenDirectoryLink);
    return;
  }
  
  if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrint("Start duplicate cluster check.\n");
  
  chkdsk_DuplicateCluster_Init();
  
  CheckedFilesCount=0;
  chkdsk_DuplicateCluster_ins(0,1);
  
  chkdsk_DuplicateCluster_ClearProgress();
  
  if(FragmentCount!=0){
    _consolePrintf("%d fragment finded in %dfiles. (NDS/SAV)\n",FragmentCount,FragmentFileCount);
  }
  
  chkdsk_DuplicateCluster_Free();
  
  if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrint("End duplicate cluster check.\n");
}

// ----------------------------------------------------

static void chkdsk_CheckFATType(void)
{
  {
    CglCanvas *pCanvas=pScreenMain->pViewCanvas;
    pCanvas->TextOutUTF8(16,chkdsk_MsgOfsY+(16*2),Lang_GetUTF8("CD_CheckFATType_Title"));
  }
  
  u32 FATType=FAT2_GetFATType();
  
  if(GlobalINI.DiskAdapter.CheckDiskType1==true) _consolePrintf("Check FAT type. [FAT%d]\n",FATType);
  
  if((FATType==16)||(FATType==32)) return;
  
  CglCanvas *pCanvas=pScreenMain->pBackCanvas;
  
  pCanvas->FillFull(GlobalBGColor15);
  
  u32 x=8,y=8;
  
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FATType_UnknownFATType"));
  y+=16;
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_FatalError_StopApplication"));
  
  pScreenMain->Flip(true);
  Sound_Start(WAVFN_Notify);
  
  StopFatalError(16405,"Unknown FAT type. (FAT%d)\n",FATType);
}

// ----------------------------------------------------

static void chkdsk_WriteTest_ShowError(void)
{
  _consoleLogResume();
  
  CglCanvas *pCanvas=pScreenMain->pBackCanvas;
  
  pCanvas->FillFull(GlobalBGColor15);
  
  u32 x=8,y=8;
  
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Title"));
  y+=16;
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg0"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg1"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg2"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg3"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg4"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg5"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg6"));
  y+=16;
  pCanvas->TextOutUTF8(x,y,Lang_GetUTF8("CD_WriteError_Msg7"));
  
  pScreenMain->Flip(true);
  Sound_Start(WAVFN_Notify);
  
  StopFatalError(16406,"Write test error. Please reffer log file.\n");
}

#define chkdsk_TempFilename "/_CHKDSK_._$"

static void chkdsk_WriteTest(void)
{
  {
    CglCanvas *pCanvas=pScreenMain->pViewCanvas;
    pCanvas->TextOutUTF8(16,chkdsk_MsgOfsY+(16*3),Lang_GetUTF8("CD_WriteTest_Title"));
  }
  
  const u32 TestSize=(16*1024)/4;
  u32 *pTestBuf=(u32*)safemalloc_chkmem(&MM_Temp,TestSize*4);
  
  _consoleLogPause();
  _consolePrintf("Disk read-write check. [%s] / ",chkdsk_TempFilename);
  
  FAT_FILE *pfile;
  
  _consolePrint("Open file for write. / ");
  pfile=FAT2_fopen_AliasForWrite(chkdsk_TempFilename);
  if(pfile==NULL){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  for(u32 idx=0;idx<TestSize;idx++){
    pTestBuf[idx]=idx;
  }
  
  _consolePrint("File write. / ");
  if(FAT2_fwrite(pTestBuf,4,TestSize,pfile)!=(TestSize*4)){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Check position. / ");
  if(FAT2_ftell(pfile)!=(TestSize*4)){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("File seek. / ");
  if(FAT2_fseek(pfile,0,SEEK_SET)!=0){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Check position. / ");
  if(FAT2_ftell(pfile)!=0){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("File close. / ");
  if(FAT2_fclose(pfile)==false){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  for(u32 idx=0;idx<TestSize;idx++){
    pTestBuf[idx]=0;
  }
  
  _consolePrint("Open file for read. / ");
  pfile=FAT2_fopen_AliasForRead(chkdsk_TempFilename);
  if(pfile==NULL){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("File read. / ");
  if(FAT2_fread(pTestBuf,4,TestSize,pfile)!=(TestSize*4)){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Check position. / ");
  if(FAT2_ftell(pfile)!=(TestSize*4)){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Check values. / ");
  for(u32 idx=0;idx<TestSize;idx++){
    if(pTestBuf[idx]!=idx){
      chkdsk_WriteTest_ShowError();
      return;
    }
  }
  
  _consolePrint("File close. / ");
  if(FAT2_fclose(pfile)==false){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Remove file. / ");
  if(FAT2_remove(chkdsk_TempFilename)!=0){
    chkdsk_WriteTest_ShowError();
    return;
  }
  
  _consolePrint("Succeeded.\n");
  _consoleLogResume();
  
  safefree(&MM_Temp,pTestBuf);
}

// ----------------------------------------------------

#include "disc_io.h"

extern DISC_INTERFACE* active_interface;

static void chkdsk_Read16bitTest(void)
{
  _consolePrint("Start read 16bit test.\n");
  
  if(GlobalINI.DiskAdapter.Ignore16bitReadTest==true){
    _consolePrint("Ignore this test.\n");
    return;
  }
  
  u32 secidx;
  
  {
    FAT_FILE *pf=Shell_FAT_fopen_Internal(DiskCheck_Read16bitBinFilename);
    secidx=FAT2_ClustToSect(pf->firstCluster);
    FAT2_fclose(pf);
  }
  
  vu32 buf32[(512+4)/4];
  vu8 *pbuf=(vu8*)buf32;
  pbuf+=2;
  
  bool err=false;
  
  if(((u32)pbuf&2)==0) err=true;
  
  pbuf[-2]=0x88;
  pbuf[-1]=0xaa;
  pbuf[512+0]=0x88;
  pbuf[512+1]=0xaa;
  
  active_interface->readSectors(secidx,1,(void*)pbuf);
  
  if((pbuf[-2]!=0x88)||(pbuf[-1]!=0xaa)){
    _consolePrint("Illigal before write.\n");
    err=true;
  }
  
  if((pbuf[512+0]!=0x88)||(pbuf[512+1]!=0xaa)){
    _consolePrint("Illigal last write.\n");
    err=true;
  }
  
  for(u32 idx=0;idx<256;idx++){
    if((pbuf[idx*2+0]!=idx)||(pbuf[idx*2+1]!=(255-idx))){
      _consolePrintf("Illigal body write. 0x%02x!=0x%02x, 0x%02x!=0x%02x\n",pbuf[idx*2+0],idx,pbuf[idx*2+1],255-idx);
      err=true;
      break;
    }
  }
  
  if(err==false) return;
  
  StopFatalError(16408,"16bit read check error. Please refer [/misctools/Additional/Ignore16.txt]\n");
}

