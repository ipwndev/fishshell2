
typedef struct {
  u16 wFormatTag;
  u16 nChannels;
  u32 nSamplesPerSec;
  u32 nAvgBytesPerSec;
  u16 nBlockAlign;
  u16 wBitsPerSample;
  u16 cbSize;
} WAVEFORMATEX;

static bool WaveFile_CheckWaveChunk(WAVEFORMATEX *pwfex)
{
  if(pwfex->wFormatTag!=0x0001){
    _consolePrintf("Illigal CompressFormat Error. wFormatTag=0x%x\n",pwfex->wFormatTag);
    return(false);
  }
  
  if((pwfex->nChannels!=1)&&(pwfex->nChannels!=2)){
    _consolePrintf("Channels Error. nChannels=%d\n",pwfex->nChannels);
    return(false);
  }
  
  if(pwfex->wBitsPerSample!=8){
    _consolePrintf("Not support bits size. (%dbits)\n",pwfex->wBitsPerSample);
    return(false);
  }
  
  if(VerboseDebugLog==true){
    _consolePrintf("fmt:0x%x chns:%d\n",pwfex->wFormatTag,pwfex->nChannels);
    _consolePrintf("Smpls/Sec:%d AvgBPS:%d\n",pwfex->nSamplesPerSec,pwfex->nAvgBytesPerSec);
    _consolePrintf("BlockAlign:%d Bits/Smpl:%d\n",pwfex->nBlockAlign,pwfex->wBitsPerSample);
  }
  
  return(true);
}

static bool LoadWaveFile(TSound *pSnd,u8 *pbuf)
{
  WAVEFORMATEX wfex=*(WAVEFORMATEX*)&pbuf[0x14];
  if(WaveFile_CheckWaveChunk(&wfex)==false) return(false);
  
  u32 DataTopOfs=0;
  
  while(true){
    if(pbuf[DataTopOfs+0]=='d'){
      if((pbuf[DataTopOfs+1]=='a')&&(pbuf[DataTopOfs+2]=='t')&&(pbuf[DataTopOfs+3]=='a')){
        DataTopOfs+=4;
        break;
      }
    }
    DataTopOfs++;
    if(DataTopOfs==256){
      _consolePrintf("Can not find data chunk.\n");
      return(false);
    }
  }
  
  u32 DataSize=0;
  DataSize|=pbuf[DataTopOfs+0]<<0;
  DataSize|=pbuf[DataTopOfs+1]<<8;
  DataSize|=pbuf[DataTopOfs+2]<<16;
  DataSize|=pbuf[DataTopOfs+3]<<24;
  if(VerboseDebugLog==true) _consolePrintf("DataSize: %dbyte.\n",DataSize);
  DataTopOfs+=4;
  
  // 8bit 1/2ch only.
  u32 SamplesCount=DataSize;
  u32 Channels=wfex.nChannels;
  if(Channels==2) SamplesCount/=2;
  
  pSnd->Freq=wfex.nSamplesPerSec;
  pSnd->Channels=Channels;
  pSnd->BufCount=SamplesCount;
  
  pSnd->lbuf=(u8*)safemalloc_chkmem(&MM_System,SamplesCount);
  pSnd->rbuf=(u8*)safemalloc_chkmem(&MM_System,SamplesCount);
  
  u8 *psrc=&pbuf[DataTopOfs];
  
  if(Channels==1){
    for(u32 idx=0;idx<SamplesCount;idx++){
      pSnd->lbuf[idx]=(*psrc++)-128;
      pSnd->rbuf[idx]=0;
    }
    }else{
    for(u32 idx=0;idx<SamplesCount;idx++){
      pSnd->lbuf[idx]=(*psrc++)-128;
      pSnd->rbuf[idx]=(*psrc++)-128;
    }
  }
  
  return(true);
}

