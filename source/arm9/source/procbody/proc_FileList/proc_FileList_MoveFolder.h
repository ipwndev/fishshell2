
DATA_IN_IWRAM_FileList static s32 TopPosStacks[16]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

static void TopPosStacks_Set(UnicodeChar *pPathUnicode,s32 TopPos)
{
//  const char *papath=StrConvert_Unicode2Ank_Test(pPathUnicode);
  
  u32 pathdepth=0;
  if((pPathUnicode[0]==0)||(pPathUnicode[1]==0)){
    }else{
    while(*pPathUnicode!=(UnicodeChar)0){
      UnicodeChar uc=*pPathUnicode++;
      if(uc==(UnicodeChar)'/') pathdepth++;
    }
  }
  
  if(16<=pathdepth) return;
  
//  _consolePrintf("TopPosStacks_Set: pathdepth=%d TopPos=%d [%s]\n",pathdepth,TopPos,papath);
  
  TopPosStacks[pathdepth]=TopPos;
}

static s32 TopPosStacks_Get(UnicodeChar *pPathUnicode)
{
//  const char *papath=StrConvert_Unicode2Ank_Test(pPathUnicode);
  
  u32 pathdepth=0;
  if((pPathUnicode[0]==0)||(pPathUnicode[1]==0)){
    }else{
    while(*pPathUnicode!=(UnicodeChar)0){
      UnicodeChar uc=*pPathUnicode++;
      if(uc==(UnicodeChar)'/') pathdepth++;
    }
  }
  
  if(16<=pathdepth) return(-1);
  
//  _consolePrintf("TopPosStacks_Get: pathdepth=%d TopPos=%d [%s]\n",pathdepth,TopPosStacks[pathdepth],papath);
  
  return(TopPosStacks[pathdepth]);
}

static void MoveUpFolder(void)
{
  if((ProcState.FileList.CurrentPathUnicode[0]==0)||(ProcState.FileList.CurrentPathUnicode[1]==0)) return;
  
  if(ProcState.FileList.MoveFolderLocked==true){
    Sound_Start(WAVFN_Notify);
    return;
  }
  
  Sound_Start(WAVFN_MovePage);
  ChangedCurrentPath=true;
  
  TopPosStacks_Set(ProcState.FileList.CurrentPathUnicode,-1);
  
  UnicodeChar *ptaguni=ProcState.FileList.CurrentPathUnicode;
  
  u32 slashidx=0;
  u32 idx=0;
  while(ptaguni[idx]!=(UnicodeChar)0){
    if(ptaguni[idx]==(UnicodeChar)'/') slashidx=idx;
    idx++;
  }
  
  Unicode_Copy(ProcState.FileList.SelectFilenameUnicode,&ptaguni[slashidx+1]);
  ProcState.FileList.SelectWindowTopOffset=ScrollBar.ItemHeight;
  
  if(slashidx==0){
    ptaguni[0]=(UnicodeChar)'/';
    ptaguni[1]=(UnicodeChar)0;
    }else{
    ptaguni[slashidx]=(UnicodeChar)0;
  }
  
  FileListInit();
  
  s32 TopPos=TopPosStacks_Get(ProcState.FileList.CurrentPathUnicode);
  if(TopPos!=-1){
    ScrollBar_SetDirectTopPos(&ScrollBar,TopPos);
    ScrollBar.ShowPos=ScrollBar.TopPos;
  }
  
  ProcState_RequestSave=true;
  
  RequestRefreshPlayCursorIndex=true;
  
  SetProcFadeEffect(EPFE_CrossFade);
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
}

static void MoveFolder(void)
{
  if(ProcState.FileList.MoveFolderLocked==true){
    Sound_Start(WAVFN_Notify);
    return;
  }
  
  Sound_Start(WAVFN_MovePage);
  ChangedCurrentPath=true;
  
  TopPosStacks_Set(ProcState.FileList.CurrentPathUnicode,ScrollBar.TopPos);
  
  TNDSFile *pndsf=NDSFiles_GetFileBody(ScrollBar.SelectedIndex);
  
  UnicodeChar *ptaguni=ProcState.FileList.CurrentPathUnicode;
  
  if(ptaguni[1]!=(UnicodeChar)0){
    UnicodeChar uslash[2]={(UnicodeChar)'/',(UnicodeChar)0};
    Unicode_Add(ptaguni,uslash);
  }
  
  Unicode_Add(ptaguni,pndsf->pFilenameUnicode);
  
  ProcState.FileList.SelectFilenameUnicode[0]=(UnicodeChar)0;
  ProcState.FileList.SelectWindowTopOffset=0;
  
  FileListInit();
  
  s32 TopPos=TopPosStacks_Get(ProcState.FileList.CurrentPathUnicode);
  if(TopPos!=-1){
    ScrollBar_SetDirectTopPos(&ScrollBar,TopPos);
    ScrollBar.ShowPos=ScrollBar.TopPos;
  }
  
  ProcState_RequestSave=true;
  
  RequestRefreshPlayCursorIndex=true;
  
  SetProcFadeEffect(EPFE_CrossFade);
  ScreenRedrawFlag=true;
  ForceUpdateSubScreenFlag=true;
}

