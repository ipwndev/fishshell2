
#pragma pack(1)
typedef struct {
  char tag [3];
  byte vers;
  byte track_count;
  byte first_track;
  byte load_addr [2];
  byte init_addr [2];
  byte play_addr [2];
  byte stack_ptr [2];
  byte timer_modulo;
  byte timer_mode;
  char game [32];
  char author [32];
  char copyright [32];
} TGBSInfo;
#pragma pack()

DATA_IN_ITCM_GME static TGBSInfo GBSInfo;

CODE_IN_ITCM_GME static bool LoadGBSInfo(u8 *pb,u32 size)
{
  if(size<sizeof(TGBSInfo)){
    _consolePrintf("DataSize:%d<%d\n",size,sizeof(TGBSInfo));
    return(false);
  }
  
  MemCopy8CPU(pb,&GBSInfo,sizeof(TGBSInfo));
  
  if((GBSInfo.tag[0]!='G')||(GBSInfo.tag[1]!='B')||(GBSInfo.tag[2]!='S')){
    _consolePrint("Illigal GBS format.\n");
    _consolePrint("tag[]=");
    for(u32 i=0;i<8;i++){
      _consolePrintf("%02x,",pb[i]);
    }
    _consolePrint("\n");
    return(false);
  }
  
  return(true);
}

static void ShowGBSInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%.32s\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  _consolePrintf("ID:%.3s\n",GBSInfo.tag);
  
  au8("VersionNumber",GBSInfo.vers);
  au8("track_count",GBSInfo.track_count);
  au8("first_track",GBSInfo.first_track);
  au16("load_addr",(GBSInfo.load_addr[0]<<8)|GBSInfo.load_addr[1]);
  au16("init_addr",(GBSInfo.init_addr[0]<<8)|GBSInfo.init_addr[1]);
  au16("play_addr",(GBSInfo.play_addr[0]<<8)|GBSInfo.play_addr[1]);
  au16("stack_ptr",(GBSInfo.stack_ptr[0]<<8)|GBSInfo.stack_ptr[1]);
  au8("timer_modulo",GBSInfo.timer_modulo);
  au8("timer_mode",GBSInfo.timer_mode);
  astr("strGame",GBSInfo.game);
  astr("strAuthor",GBSInfo.author);
  astr("strCopyright",GBSInfo.copyright);

#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMEGBS_GetInfoIndexCount(void)
{
  return(4);
}

static bool GMEGBS_GetInfoStrL(int idx,char *str,int len)
{
  switch(idx){
    case 0: snprintf(str,len,"%.32s",GBSInfo.game); return(true); break;
    case 1: snprintf(str,len,"%.32s",GBSInfo.author); return(true); break;
    case 2: snprintf(str,len,"%.32s",GBSInfo.copyright); return(true); break;
    case 3: snprintf(str,len,"track_count: %d/%d (first=%d)",TrackNum,GBSInfo.track_count,GBSInfo.first_track); return(true); break;
  }
  return(false);
}

