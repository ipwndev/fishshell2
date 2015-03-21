
#define ESCF_VRCVI (BIT0)
#define ESCF_VRCVII (BIT1)
#define ESCF_FDS (BIT2)
#define ESCF_MMC5 (BIT3)
#define ESCF_Namco106 (BIT4)
#define ESCF_SunsoftFME07 (BIT5)
#define ESCF_Future6 (BIT6)
#define ESCF_Future7 (BIT7)

#define ESCF_SupportFlag (ESCF_VRCVI | ESCF_Namco106 | ESCF_SunsoftFME07)

typedef struct {
  u8 ID[5];
  u8 VersionNumber;
  u8 TotalSongs;
  u8 StartingSong;
  u16 LoadAddress;
  u16 InitAddress;
  u16 PlayAddress;
  u8 *strName;
  u8 *strArtist;
  u8 *strCopyright;
  u16 NTSCTicksCount;
  u8 BankswitchInitValues[8];
  bool UseBankswitch;
  u16 PALTicksCount;
  u8 VSyncMode;
  u8 ExtraSoundChipFlags;
  u8 *Data;
  u32 DataSize;
  u32 PageCount;
} TNSFInfo;

DATA_IN_ITCM_GME static TNSFInfo NSFInfo;

CODE_IN_ITCM_GME static bool LoadNSFInfo(u8 *pb,u32 size)
{
#define writebyte(pos,data) (pb[pos]=data)
#define readbyte(pos) (pb[pos])
#define readword(pos) (readbyte(pos)+((u16)readbyte(pos+1) << 8))
#define readstr(pos) (&pb[pos])
  
  if(size<sizeof(TNSFInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TNSFInfo));
    return(false);
  }
  
  NSFInfo.DataSize=size-sizeof(TNSFInfo);
  NSFInfo.Data=&pb[sizeof(TNSFInfo)];
  NSFInfo.PageCount=NSFInfo.DataSize/0x1000;
  
  for(u32 i=0;i<5;i++){
    NSFInfo.ID[i]=readbyte(0x00+i);
  }
  
  if((NSFInfo.ID[0]!='N')||(NSFInfo.ID[1]!='E')||(NSFInfo.ID[2]!='S')||(NSFInfo.ID[3]!='M')||(NSFInfo.ID[4]!=0x1a)){
    _consolePrint("Illigal NSF format.\n");
    _consolePrint("ID[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrint("\n");
    return(false);
  }
  
  NSFInfo.TotalSongs=readbyte(0x06);
  NSFInfo.StartingSong=readbyte(0x07)-1;
  
  if(NSFInfo.TotalSongs==0){
    _consolePrintf("TotalSongs 0x%2x==0x00\n",NSFInfo.TotalSongs);
    return(false);
  }
  
  if(NSFInfo.StartingSong<=NSFInfo.TotalSongs){
    NSFInfo.StartingSong=NSFInfo.TotalSongs-1;
  }
  
  NSFInfo.LoadAddress=readword(0x08);
  
  if(NSFInfo.LoadAddress<0x8000){
    _consolePrintf("LoadAddress 0x%4x<0x8000\n",NSFInfo.LoadAddress);
    return(false);
  }
  
  NSFInfo.InitAddress=readword(0x0a);
  NSFInfo.PlayAddress=readword(0x0c);
  NSFInfo.strName=readstr(0x0e);
  NSFInfo.strArtist=readstr(0x2e);
  NSFInfo.strCopyright=readstr(0x4e);
  NSFInfo.NTSCTicksCount=readword(0x6e);
  
  NSFInfo.UseBankswitch=false;
  for(u32 i=0;i<8;i++){
    NSFInfo.BankswitchInitValues[i]=readbyte(0x70+i);
    if(NSFInfo.BankswitchInitValues[i]!=0x00) NSFInfo.UseBankswitch=true;
  }
  
  NSFInfo.PALTicksCount=readword(0x78);
  
  NSFInfo.VSyncMode=readbyte(0x7a) & BIT0;
  
  NSFInfo.ExtraSoundChipFlags=readbyte(0x7b);
  
  u8 excf=NSFInfo.ExtraSoundChipFlags;
  
  _consolePrint("Defined extra sound chips: ");
  if((excf&ESCF_VRCVI)!=0) _consolePrint("VRCVI,");
  if((excf&ESCF_VRCVII)!=0) _consolePrint("VRCVII,");
  if((excf&ESCF_FDS)!=0) _consolePrint("FDS,");
  if((excf&ESCF_MMC5)!=0) _consolePrint("MMC5,");
  if((excf&ESCF_Namco106)!=0) _consolePrint("Namco106,");
  if((excf&ESCF_SunsoftFME07)!=0) _consolePrint("SunsoftFME07,");
  if((excf&ESCF_Future6)!=0) _consolePrint("Future6,");
  if((excf&ESCF_Future7)!=0) _consolePrint("Future7,");
  _consolePrint("\n");
  
  if((excf&~ESCF_SupportFlag)!=0){
    _consolePrint("notsupport ExtraSoundChip\n");
  }
  
  // overwrite
  
  writebyte(0x7b,NSFInfo.ExtraSoundChipFlags & ESCF_SupportFlag);
  
  return(true);

#undef writebyte
#undef readbyte
#undef readword
#undef readstr
}

static void ShowNSFInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  _consolePrintf("ID:%4s $%02x\n",(u32*)NSFInfo.ID,NSFInfo.ID[4]);

  au8("VersionNumber",NSFInfo.VersionNumber);

  au8("TotalSongs",NSFInfo.TotalSongs);
  au8("StartingSong",NSFInfo.StartingSong);
  au16("LoadAddress",NSFInfo.LoadAddress);
  au16("InitAddress",NSFInfo.InitAddress);
  au16("PlayAddress",NSFInfo.PlayAddress);
  astr("strName",NSFInfo.strName);
  astr("strArtist",NSFInfo.strArtist);
  astr("strCopyright",NSFInfo.strCopyright);
  au16("NTSCTicksCount",NSFInfo.NTSCTicksCount);
  
  astr("BankswitchInitValues[8]","...");
  if(NSFInfo.UseBankswitch==false){
    _consolePrint("notused.\n");
    }else{
    for(u32 cnt=0;cnt<8;cnt++){
      _consolePrintf("%02x,",NSFInfo.BankswitchInitValues[cnt]);
    }
    _consolePrint("\n");
  }
  
  au16("PALTicksCount",NSFInfo.PALTicksCount);
  au8("VSyncMode",NSFInfo.VSyncMode);
  au8("ExtraSoundChipFlags",NSFInfo.ExtraSoundChipFlags);

  au32("DataPtr",(u32)&NSFInfo.Data[0]);
  au32("DataSize",NSFInfo.DataSize);
  au32("PageCount",NSFInfo.PageCount);

#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMENSF_GetInfoIndexCount(void)
{
  return(6);
}

static bool GMENSF_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"Name=%s",NSFInfo.strName); return(true); break;
    case 1: snprintf(str,len,"Artist=%s",NSFInfo.strArtist); return(true); break;
    case 2: snprintf(str,len,"Copyright=%s",NSFInfo.strCopyright); return(true); break;
    case 3: {
      snprintf(str,len,"Tracks=%d/%d ",TrackNum,NSFInfo.TotalSongs);
      u8 excf=NSFInfo.ExtraSoundChipFlags;
      if(excf==0) strcat(str,"not use extra chips.");
      if((excf&ESCF_VRCVI)!=0) strcat(str,"VRCVI,");
      if((excf&ESCF_VRCVII)!=0) strcat(str,"VRCVII,");
      if((excf&ESCF_FDS)!=0) strcat(str,"FDS,");
      if((excf&ESCF_MMC5)!=0) strcat(str,"MMC5,");
      if((excf&ESCF_Namco106)!=0) strcat(str,"Namco106,");
      if((excf&ESCF_SunsoftFME07)!=0) strcat(str,"SunsoftFME07,");
      if((excf&ESCF_Future6)!=0) strcat(str,"Future6,");
      if((excf&ESCF_Future7)!=0) strcat(str,"Future7,");
      return(true);
    } break;
    case 4: snprintf(str,len,"Load:0x%x Init:0x%x Play:0x%x",NSFInfo.LoadAddress,NSFInfo.InitAddress,NSFInfo.PlayAddress); return(true); break;
    case 5: snprintf(str,len,"NTSCTicks: %d PALTicks: %d",NSFInfo.NTSCTicksCount,NSFInfo.PALTicksCount); return(true); break;
  }
  return(false);
}

