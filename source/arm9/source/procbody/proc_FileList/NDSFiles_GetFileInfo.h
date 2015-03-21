
#define FREAD(rbuf) if(FAT2_fread_fast(rbuf,1,512,pfh)!=512) return;

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_MP3(FAT_FILE *pfh,TNDSFile *pndsf)
{
  u32 bitrate_index=(u32)-1;
  
  {
    u8 rbuf[512];
    u32 rbufpos=0;
    FREAD(rbuf);
    
    u32 lastseccnt=64;
    
    u32 ID=0;
    
    while(1){
      ID=(ID<<8)|rbuf[rbufpos++];
      if(rbufpos==512){
        lastseccnt--;
        if(lastseccnt==0) break;
        FREAD(rbuf);
        rbufpos=0;
      }
      if((ID&0xfffe)==0xfffa){
        u32 header=rbuf[rbufpos];
        bitrate_index=(header>>4)&0x0f;
        //_consolePrintf("Found Header. 0x%04x, 0x%04x, bitrate_index=%d.\n",ID,rbuf[rbufpos],bitrate_index);
        break;
      }
    }
  }
  
  if(bitrate_index==(u32)-1) return;
  
  u32 bitrate_table[0x10]={0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,1};
  u32 bitrate=bitrate_table[bitrate_index];
  
  if(bitrate==0){
    strcpy(pndsf->pFileInfo,"VBR");
    return;
  }
  
  if(bitrate==1){
    strcpy(pndsf->pFileInfo,"Error");
    return;
  }
  
  char str[32+1];
  
  u32 sec=pndsf->FileSize/(bitrate*1000/8);
  if(sec<=((59*60)+59)){
    snprintf(str,32,"%2d:%02d",sec/60,sec%60);
    }else{
    snprintf(str,32,"%d:%02d:%02d",sec/60/60,(sec/60)%60,sec%60);
  }
  strcpy(pndsf->pFileInfo,str);
}

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_Jpeg(FAT_FILE *pfh,TNDSFile *pndsf)
{
  u32 x=0,y=0;
  
  {
    u8 rbuf[512];
    u32 rbufpos=0;
    FREAD(rbuf);
    
    if((rbuf[0]!=0xff)||(rbuf[1]!=0xd8)) return; // Illigal SOI
    rbufpos+=2;
    
    u32 lastmrkcnt=32;
    
    while(1){
      lastmrkcnt--;
      if(lastmrkcnt==0) break;
      
      while(rbuf[rbufpos]==0xff){
        rbufpos++;
        if(rbufpos==512){
          FREAD(rbuf);
          rbufpos=0;
        }
      }
      u8 Marker=rbuf[rbufpos++];
      if(rbufpos==512){
        FREAD(rbuf);
        rbufpos=0;
      }
      
      if((Marker&0xf0)==0xc0){
        u32 SOFNum=Marker&0xf;
        if((SOFNum!=0x4)&&(SOFNum!=0x8)&&(SOFNum!=0xc)){
          u8 header[8];
          for(u32 idx=0;idx<8;idx++){
            header[idx]=rbuf[rbufpos++];
            if(rbufpos==512){
              FREAD(rbuf);
              rbufpos=0;
            }
          }
          x=(header[5+0]<<8)|header[5+1];
          y=(header[3+0]<<8)|header[3+1];
          break;
        }
      }
      
      u16 MarkerSize=0;
      MarkerSize=rbuf[rbufpos++];
      if(rbufpos==512){
        FREAD(rbuf);
        rbufpos=0;
      }
      MarkerSize=(MarkerSize<<8)|rbuf[rbufpos++];
      if(rbufpos==512){
        FREAD(rbuf);
        rbufpos=0;
      }
      
//      _consolePrintf("Skip marker. 0xff%02x, %d.\n",Marker,MarkerSize);
      
      rbufpos+=MarkerSize-2;
      while(1024<=rbufpos){
        if(FAT2_fskip(1,512,pfh)!=512) return;
        rbufpos-=512;
      }
      while(512<=rbufpos){
        FREAD(rbuf);
        rbufpos-=512;
      }
    }
  }
  
  if((x==0)||(y==0)) return;
  
  char str[32+1];
  snprintf(str,32,"%dx%d",x,y);
  strcpy(pndsf->pFileInfo,str);
}

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_BMP(FAT_FILE *pfh,TNDSFile *pndsf)
{
  u32 x=0,y=0;
  
  {
    u8 rbuf[512];
    FREAD(rbuf);
    
    if((rbuf[0]!='B')||(rbuf[1]!='M')) return;
    
    x=(rbuf[0x12+3]<<24)|(rbuf[0x12+2]<<16)|(rbuf[0x12+1]<<8)|(rbuf[0x12+0]<<0);
    y=(rbuf[0x16+3]<<24)|(rbuf[0x16+2]<<16)|(rbuf[0x16+1]<<8)|(rbuf[0x16+0]<<0);
  }
  
  if((x==0)||(y==0)) return;
  
  char str[32+1];
  snprintf(str,32,"%dx%d",x,y);
  strcpy(pndsf->pFileInfo,str);
}

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_PSD(FAT_FILE *pfh,TNDSFile *pndsf)
{
  u32 x=0,y=0;
  
  {
    u8 rbuf[512];
    FREAD(rbuf);
    
    if((rbuf[0]!='8')||(rbuf[1]!='B')||(rbuf[2]!='P')||(rbuf[3]!='S')) return;
    
    x=(rbuf[0xe + 0]<<24)|(rbuf[0xe + 1]<<16)|(rbuf[0xe + 2]<<8)|(rbuf[0xe + 3]<<0);
    y=(rbuf[0x12+0]<<24)|(rbuf[0x12+1]<<16)|(rbuf[0x12+2]<<8)|(rbuf[0x12+3]<<0);
  }
  
  if((x==0)||(y==0)) return;
  
  char str[32+1];
  snprintf(str,32,"%dx%d",x,y);
  strcpy(pndsf->pFileInfo,str);
}

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_DPG(FAT_FILE *pfh,TNDSFile *pndsf)
{
  u32 TotalFrame=0,FPS=0;
  
  {
    u32 Header[512/4];
    FREAD((u8*)Header);
    
    const u32 DPG0ID=0x30475044;
    const u32 DPG1ID=0x31475044;
    const u32 DPG2ID=0x32475044;
    const u32 DPG3ID=0x33475044;
    const u32 DPG4ID=0x34475044;
    u32 ID=Header[0];
    if((ID!=DPG0ID)&&(ID!=DPG1ID)&&(ID!=DPG2ID)&&(ID!=DPG3ID)&&(ID!=DPG4ID)) return;
    
    TotalFrame=Header[1];
    FPS=Header[2]>>8;
  }
  
  if((TotalFrame==0)||(FPS==0)) return;
  
  char str[32+1];
  
  u32 sec=TotalFrame/FPS;
  if(sec<=((59*60)+59)){
    snprintf(str,32,"%2d:%02d",sec/60,sec%60);
    }else{
    snprintf(str,32,"%d:%02d:%02d",sec/60/60,(sec/60)%60,sec%60);
  }
  strcpy(pndsf->pFileInfo,str);
}

#undef FREAD

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_FileSize(TNDSFile *pndsf)
{
  char str[32+1];
  
  u32 fsize=pndsf->FileSize;
  
  if(fsize<1*1024){
    snprintf(str,32,"%dByte",fsize);
    }else{
    if(fsize<1*1024*1024){
      snprintf(str,32,"%dKB",fsize/1024);
      }else{
      if(fsize<1*1024*1024*1024){
        snprintf(str,32,"%dMB",fsize/1024/1024);
        }else{
        snprintf(str,32,"%dGB",fsize/1024/1024/1024);
      }
    }
  }

  strcpy(pndsf->pFileInfo,str);
}

static void NDSFiles_RefreshCurrentFolder_ins_GetFileInfo(TNDSFile *pndsf)
{
	if(!ProcState.FileList.EnableFileInfo) return;
	
  u32 Ext32=pndsf->Ext32;
  
#define FOPEN \
  FAT_FILE *pfh=FAT2_fopen_AliasForRead(pndsf->pFilenameAlias); \
  if(pfh==NULL) return;
  
#define FCLOSE FAT2_fclose(pfh);
  
  switch(pndsf->FileType){
    case ENFFT_UnknownFile: break;
    case ENFFT_UpFolder: return; 
    case ENFFT_Folder: return; 
    case ENFFT_Sound: {
      if(Ext32==MakeExt32(0,'M','P','3')){
        FOPEN
        NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_MP3(pfh,pndsf);
        FCLOSE
        return;
      }
    } break;
    case ENFFT_PlayList: break;
    case ENFFT_Image: {
      if((Ext32==MakeExt32(0,'J','P','G'))||(Ext32==MakeExt32(0,'J','P','E'))){
        FOPEN
        NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_Jpeg(pfh,pndsf);
        FCLOSE
        return;
      }
      if(Ext32==MakeExt32(0,'B','M','P')){
        FOPEN
        NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_BMP(pfh,pndsf);
        FCLOSE
        return;
      }
      if(Ext32==MakeExt32(0,'P','S','D')){
        FOPEN
        NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_PSD(pfh,pndsf);
        FCLOSE
        return;
      }
    } break;
    case ENFFT_Text: break;
    case ENFFT_Video: {
      FOPEN
      NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_DPG(pfh,pndsf);
      FCLOSE
      return;
    } break;
    case ENFFT_NDSROM: break;
    case ENFFT_Skin: break;
  }
  
#undef FOPEN
#undef FCLOSE
  
  NDSFiles_RefreshCurrentFolder_ins_GetFileInfo_ins_FileSize(pndsf);
}

