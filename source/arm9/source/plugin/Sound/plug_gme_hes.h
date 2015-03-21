
#define HES_header_size (0x20)
#define HES_text_pos (HES_header_size+0x20)
#define HES_text_max (0x80)

#define HESM_TAG (0x4d534548)

typedef struct {
  u32 tag;
  u32 tag_term;
  byte vers;
  byte first_track;
  u16 init_addr;
  byte banks[8];
  u32 data_tag;
  u32 size;
  u32 addr;
  u32 unused;
  char game[HES_text_max],author[HES_text_max],copyright[HES_text_max];
} THESInfo;

DATA_IN_ITCM_GME static THESInfo HESInfo;

CODE_IN_ITCM_GME static bool LoadHESInfo(u8 *pb,u32 size)
{
#define writebyte(pos,data) (pb[pos]=data)
#define readbyte(pos) (pb[pos])
#define readword(pos) (readbyte(pos)|((u16)readbyte(pos+1) << 8))
#define readdword(pos) (readbyte(pos)|((u32)readbyte(pos+1) << 8)|((u32)readbyte(pos+2) << 16)|((u32)readbyte(pos+3) << 24))
#define readstr(pos) (&pb[pos])
  
  if(size<HES_text_pos){
    _consolePrintf("DataSize:%d<%d\n",size,HES_text_pos);
    return(false);
  }
  
  THESInfo *phi=&HESInfo;
  
  phi->tag=readdword(0x00);
  phi->tag_term=0;
  phi->vers=readbyte(0x04);
  phi->first_track=readbyte(0x05);
  phi->init_addr=readword(0x06);
  for(u32 idx=0;idx<8;idx++){
    phi->banks[idx]=readbyte(0x08+idx);
  }
  phi->data_tag=readbyte(0x10);
  phi->size=readbyte(0x14);
  phi->addr=readbyte(0x18);
  phi->unused=readbyte(0x1c);
  
  if((phi->tag!=HESM_TAG)||(phi->vers!=0)){
    _consolePrint("Unknown HES format.\n");
    return(false);
  }
  
  u32 pos=HES_text_pos;
  
  {
    while(readbyte(pos)==0x00){
      pos++;
    }
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      phi->game[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==HES_text_max) break;
    }
    phi->game[idx]=0;
  }
  
  {
    while(readbyte(pos)==0x00){
      pos++;
    }
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      phi->author[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==HES_text_max) break;
    }
    phi->author[idx]=0;
  }
  
  {
    while(readbyte(pos)==0x00){
      pos++;
    }
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      phi->copyright[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==HES_text_max) break;
    }
    phi->copyright[idx]=0;
  }
  
  return(true);

#undef writebyte
#undef readbyte
#undef readword
#undef readdword
#undef readstr
}

static void ShowHESInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define ad(ttl,data) _consolePrintf("%s:%d\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  THESInfo *phi=&HESInfo;
  
  _consolePrintf("ID:%s Version.%d\n",(char*)&phi->tag,phi->vers);
  
  ad("FirstTrack",phi->first_track);
  au16("InitialAddr",phi->init_addr);
  
  _consolePrint("Banks:");
  for(u32 idx=0;idx<8;idx++){
    _consolePrintf("%02x,",phi->banks[idx]);
  }
  _consolePrint("\n");
  
  _consolePrintf("ROM Addr:%08x Size:%08x\n",phi->addr,phi->size);
  
  astr("Game",phi->game);
  astr("Author",phi->author);
  astr("Copyright",phi->copyright);
  
#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMEHES_GetInfoIndexCount(void)
{
  return(8);
}

static bool GMEHES_GetInfoStrL(int idx,char *str,int len)
{
  THESInfo *phi=&HESInfo;
  
  switch(idx){
    case 0: snprintf(str,len,"ID: %s Version%d",(char*)&phi->tag,phi->vers); return(true); break;
    case 1: snprintf(str,len,"Game: %s",phi->game); return(true); break;
    case 2: snprintf(str,len,"Author: %s",phi->author); return(true); break;
    case 3: snprintf(str,len,"Copyright: %s",phi->copyright); return(true); break;
    case 4: snprintf(str,len,"FirstTrack: %d",phi->first_track); return(true); break;
    case 5: snprintf(str,len,"Banks: %02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",phi->banks[0],phi->banks[1],phi->banks[2],phi->banks[3],phi->banks[4],phi->banks[5],phi->banks[6],phi->banks[7]); return(true); break;
    case 6: snprintf(str,len,"InitialAddr: %08x",phi->init_addr); return(true); break;
    case 7: snprintf(str,len,"ROM Addr:%08x Size:%08x",phi->addr,phi->size); return(true); break;
  }
  return(false);
}

