
#define KSS_header_size (0x20)

#define KSS_TAG_KSCC (0x4343534b)
#define KSS_TAG_KSSX (0x5853534b)

#pragma pack(1)
typedef struct {
  u32 tag;
  u16 load_addr;
  u16 load_size;
  u16 init_addr;
  u16 play_addr;
  u8 first_bank;
  u8 bank_mode;
  u8 extra_header;
  u8 device_flags;
  u32 data_size;
  u32 unused;
  u16 first_track;
  u16 last_tack;
  s8 psg_vol;
  s8 scc_vol;
  s8 msx_music_vol;
  s8 msx_audio_vol;
} TKSSInfo;
#pragma pack()

DATA_IN_ITCM_GME static TKSSInfo KSSInfo;

CODE_IN_ITCM_GME static bool LoadKSSInfo(u8 *pb,u32 size)
{
  if(size<KSS_header_size){
    _consolePrintf("DataSize:%d<%d\n",size,KSS_header_size);
    return(false);
  }
  
  {
    u8 *pdst=(u8*)&KSSInfo;
    for(u32 idx=0;idx<KSS_header_size;idx++){
      pdst[idx]=pb[idx];
    }
  }
  
  TKSSInfo *pki=&KSSInfo;
  MemCopy32CPU(pb,pki,KSS_header_size);
  
  u32 hsize=0;
  
  if(pki->tag==KSS_TAG_KSCC){
    hsize=0x10;
  }
  if(pki->tag==KSS_TAG_KSSX){
    hsize=0x10+pki->extra_header;
  }
  
  if((hsize==0)||(KSS_header_size<hsize)){
    _consolePrint("Unknown KSS format.\n");
    return(false);
  }
  
  if(hsize<KSS_header_size){
    u8 *pdst=(u8*)&KSSInfo;
    for(u32 idx=hsize;idx<KSS_header_size;idx++){
      pdst[idx]=0;
    }
  }
  
  if((pki->device_flags & 0x01)!=0){
    _consolePrint("not support FMPAC.\n");
    return(false);
  }
  if((pki->device_flags & 0x08)!=0){
    _consolePrint("not support MSX-AUDIO.\n");
    return(false);
  }
  if((pki->device_flags & 0xf0)!=0){
    _consolePrint("Unknown extra sound chip.\n");
    return(false);
  }
  
  return(true);
}

static void ShowKSSInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define ad(ttl,data) _consolePrintf("%s:%d\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  TKSSInfo *pki=&KSSInfo;
  
  u32 ts[2];
  ts[0]=pki->tag;
  ts[1]=0;
  
  astr("ID",(char*)ts);
  
  au16("load_addr",pki->load_addr);
  au16("load_size",pki->load_size);
  au16("init_addr",pki->init_addr);
  au16("play_addr",pki->play_addr);
  au8("first_bank",pki->first_bank);
  au8("bank_mode",pki->bank_mode);
  au8("extra_header",pki->extra_header);
  au8("device_flags",pki->device_flags);
  au32("data_size",pki->data_size);
  au32("unused",pki->unused);
  au16("first_track",pki->first_track);
  au16("last_tack",pki->last_tack);
  ad("psg_vol",pki->psg_vol);
  ad("scc_vol",pki->scc_vol);
  ad("msx_music_vol",pki->msx_music_vol);
  ad("msx_audio_vol",pki->msx_audio_vol);
  
#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMEKSS_GetInfoIndexCount(void)
{
  return(7);
}

static bool GMEKSS_GetInfoStrL(int idx,char *str,int len)
{
  TKSSInfo *pki=&KSSInfo;
  
  u32 ts[2];
  ts[0]=pki->tag;
  ts[1]=0;
  
  switch(idx){
    case 0: snprintf(str,len,"ID: %s devices: $%02x",(char*)ts,pki->device_flags); return(true); break;
    case 1: snprintf(str,len,"load_addr: $%04x,load_size: $%04x",pki->load_addr,pki->load_size); return(true); break;
    case 2: snprintf(str,len,"init_addr: $%04x play_addr: $%04x",pki->init_addr,pki->play_addr); return(true); break;
    case 3: snprintf(str,len,"tracks: %d/%d first_track: %d",TrackNum,pki->last_tack,pki->first_track); return(true); break;
    case 4: snprintf(str,len,"PSG:%d SCC:%d MSXM:%d MSXA:%d",pki->psg_vol,pki->scc_vol,pki->msx_music_vol,pki->msx_audio_vol); return(true); break;
    case 5: snprintf(str,len,"first_bank: $%02x bank_mode: $%02x",pki->first_bank,pki->bank_mode); return(true); break;
    case 6: snprintf(str,len,"extra_header: $%02x data_size: $%02x",pki->extra_header,pki->data_size); return(true); break;
  }
  return(false);
}

