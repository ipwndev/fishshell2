
#define AY_header_size (0x14)
#define AY_text_max (0x80)

typedef struct {
  byte tag[8];
  u32 tag_term;
  byte vers;
  byte player;
  u16 unused;
  byte max_track;
  byte first_track;
  char Author[HES_text_max],Comment[HES_text_max],TrackName[HES_text_max];
} TAYInfo;

DATA_IN_ITCM_GME static TAYInfo AYInfo;

CODE_IN_ITCM_GME static bool LoadAYInfo(u8 *pb,u32 size,u32 TrackNum)
{
#define writebyte(pos,data) (pb[pos]=data)
#define readbyte(pos) (pb[pos])
#define readword(pos) (((u16)readbyte(pos) << 8)|((u16)readbyte(pos+1) << 0))
#define readdword(pos) (((u32)readbyte(pos) << 24)|((u32)readbyte(pos+1) << 16)|((u32)readbyte(pos+2) << 8)|((u32)readbyte(pos+3) << 0))
#define readstr(pos) (&pb[pos])
  
  if(size<AY_header_size){
    _consolePrintf("DataSize:%d<%d\n",size,AY_header_size);
    return(false);
  }
  
  TAYInfo *pai=&AYInfo;
  
  const char ID[]="ZXAYEMUL";
  
  for(u32 idx=0;idx<8;idx++){
    pai->tag[idx]=readbyte(idx);
    if(pai->tag[idx]!=ID[idx]){
      _consolePrint("Unknown format.\n");
      return(false);
    }
  }
  pai->tag_term=0;
  
  pai->vers=readbyte(0x08);
  pai->player=readbyte(0x09);
  pai->unused=readword(0x0a);
  u32 AuthorPos=0x0c;
  AuthorPos+=readword(0x0c);
  u32 CommentPos=0x0e;
  CommentPos+=readword(0x0e);
  pai->max_track=readbyte(0x10);
  pai->first_track=readbyte(0x11);
  u32 TrackDataPos=0x12+readword(0x12)+(TrackNum*4+0);
  u32 TrackNamePos=TrackDataPos+readword(TrackDataPos);
  
  if(2<pai->vers){
    _consolePrint("Unknown version.\n");
    return(false);
  }
  
  {
    u32 pos=AuthorPos;
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      pai->Author[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==AY_text_max) break;
    }
    pai->Author[idx]=0;
  }
  
  {
    u32 pos=CommentPos;
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      pai->Comment[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==AY_text_max) break;
    }
    pai->Comment[idx]=0;
  }
  
  {
    u32 pos=TrackNamePos;
    u32 idx=0;
    while(readbyte(pos)!=0x00){
      pai->TrackName[idx]=readbyte(pos);
      idx++; pos++;
      if(idx==AY_text_max) break;
    }
    pai->TrackName[idx]=0;
  }
  
  return(true);

#undef writebyte
#undef readbyte
#undef readword
#undef readdword
#undef readstr
}

static void ShowAYInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define ad(ttl,data) _consolePrintf("%s:%d\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  TAYInfo *pai=&AYInfo;
  
  _consolePrintf("ID:%s Version.%d\n",(char*)&pai->tag,pai->vers);
  
  ad("PlayerID",pai->player);
  ad("MaxTrack",pai->max_track);
  ad("FirstTrack",pai->first_track);
  
  astr("Author",pai->Author);
  astr("Comment",pai->Comment);
  astr("TrackName",pai->TrackName);
  
#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMEAY_GetInfoIndexCount(void)
{
  return(7);
}

static bool GMEAY_GetInfoStrL(int idx,char *str,int len)
{
  TAYInfo *pai=&AYInfo;
  
  switch(idx){
    case 0: snprintf(str,len,"ID: %s Version%d",(char*)&pai->tag,pai->vers); return(true); break;
    case 1: snprintf(str,len,"Author: %s",pai->Author); return(true); break;
    case 2: snprintf(str,len,"Comment: %s",pai->Comment); return(true); break;
    case 3: snprintf(str,len,"%s",pai->TrackName); return(true); break;
    case 4: snprintf(str,len,"CurrentTrack: %d/%d",TrackNum,pai->max_track); return(true); break;
    case 5: snprintf(str,len,"FirstTrack: %d",pai->first_track); return(true); break;
    case 6: snprintf(str,len,"PlayerID: %d",pai->player); return(true); break;
  }
  return(false);
}

