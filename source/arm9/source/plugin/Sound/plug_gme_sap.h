
typedef struct {
  u32 track_count;
  char author[256],name[256],date[32];
} TSAPInfo;

DATA_IN_ITCM_GME static TSAPInfo SAPInfo;

CODE_IN_ITCM_GME static void persestr(const char *psrc,char *pdst)
{
  bool df=false;
  
  if(*psrc=='\"'){
    df=true;
    psrc++;
  }
  
  while(*psrc!=0){
    *pdst++=*psrc++;
  }
  
  *pdst=0;
  
  if(df==true){
    pdst--;
    if(*pdst=='\"') *pdst=0;
  }
}

CODE_IN_ITCM_GME static u32 perseu32(const char *psrc)
{
  if(*psrc=='\"') return(0);
  
  u32 res=0;
  
  while(*psrc!=0){
    res<<=4;
    char ch=*psrc++;
    if(('0'<=ch)&&(ch<='9')) res+=((u32)ch)-((u32)'0');
    if(('a'<=ch)&&(ch<='f')) res+=0x10+(((u32)ch)-((u32)'a'));
    if(('A'<=ch)&&(ch<='F')) res+=0x10+(((u32)ch)-((u32)'A'));
  }
  
  return(res);
}

CODE_IN_ITCM_GME static bool LoadSAPInfo(u8 *pb,u32 size)
{
#define writebyte(pos,data) (pb[pos]=data)
#define readbyte(pos) (pb[pos])
#define readword(pos) (((u16)readbyte(pos) << 8)|((u16)readbyte(pos+1) << 0))
#define readdword(pos) (((u32)readbyte(pos) << 24)|((u32)readbyte(pos+1) << 16)|((u32)readbyte(pos+2) << 8)|((u32)readbyte(pos+3) << 0))
#define readstr(pos) (&pb[pos])
  
  if(size<16){
    _consolePrintf("DataSize:%d<%d\n",size,16);
    return(false);
  }
  
  const char ID[]="SAP\x0D\x0A";
  
  for(u32 idx=0;idx<5;idx++){
    if(readbyte(idx)!=ID[idx]){
      _consolePrint("Unknown format.\n");
      return(false);
    }
  }
  
  TSAPInfo *pai=&SAPInfo;
  
  pai->author[0]=0;
  pai->name[0]=0;
  pai->date[0]=0;
  pai->track_count=1;
  
  bool isSection;
  char Section[128],Value[128];
  int SectionCount,ValueCount;
  u32 pos=0;
  
  isSection=true;
  Section[0]=0;
  SectionCount=0;
  Value[0]=0;
  ValueCount=0;
  
  while(1){
    char c0=readbyte(pos+0),c1=readbyte(pos+1);
    if((c0==0xff)&&(c1==0xff)) break;
    
    if((c0!=0x0d)||(c1!=0x0a)){
      pos+=1;
      if(c0==' '){
        isSection=false;
        }else{
        if(isSection==true){
          Section[SectionCount++]=c0;
          }else{
          Value[ValueCount++]=c0;
        }
      }
      }else{
      pos+=2;
      Section[SectionCount]=0;
      Value[ValueCount]=0;
      
      _consolePrintf("sec=%s, val=%s\n",Section,Value);
      if(strcmp(Section,"AUTHOR")==0) persestr(Value,pai->author);
      if(strcmp(Section,"NAME")==0) persestr(Value,pai->name);
      if(strcmp(Section,"DATE")==0) persestr(Value,pai->date);
      if(strcmp(Section,"SONGS")==0) pai->track_count=perseu32(Value);
      
      isSection=true;
      Section[0]=0;
      SectionCount=0;
      Value[0]=0;
      ValueCount=0;
    }
  }
  
  return(true);

#undef writebyte
#undef readbyte
#undef readword
#undef readdword
#undef readstr
}

static void ShowSAPInfo(void)
{
#define astr(ttl,data) _consolePrintf("%s:%s\n",ttl,data);
#define ad(ttl,data) _consolePrintf("%s:%d\n",ttl,data);
#define au8(ttl,data) _consolePrintf("%s:$%02x\n",ttl,data);
#define au16(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
#define au32(ttl,data) _consolePrintf("%s:$%04x\n",ttl,data);
  
  TSAPInfo *pai=&SAPInfo;
  
  astr("Author",pai->author);
  astr("Name",pai->name);
  astr("Date",pai->date);
  ad("TrackCount",pai->track_count);
  
#undef astr
#undef au8
#undef au16
#undef au32
}

static int GMESAP_GetInfoIndexCount(void)
{
  return(4);
}

static bool GMESAP_GetInfoStrL(int idx,char *str,int len)
{
  TSAPInfo *pai=&SAPInfo;
  
  switch(idx){
    case 0: snprintf(str,len,"Author: %s",pai->author); return(true); break;
    case 1: snprintf(str,len,"Name: %s",pai->name); return(true); break;
    case 2: snprintf(str,len,"Date: %s",pai->date); return(true); break;
    case 3: snprintf(str,len,"track_count: %d/%d",TrackNum,pai->track_count); return(true); break;
  }
  return(false);
}

