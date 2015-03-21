
#define LogFile_BufSize (32768)
typedef struct {
  u32 WriteTopSector;
  u32 TopPos;
  u32 BufPos;
  u32 BufSize;
  char Buf[LogFile_BufSize];
  bool Pause;
} TLogFile;

static TLogFile LogFile;

void _consoleInitLogFile(void)
{
	TLogFile *plf=&LogFile;
	
  plf->WriteTopSector=0;
  plf->TopPos=0;
  plf->BufPos=0;
  plf->BufSize=0;
  plf->Buf[0]=0;
  plf->Pause=false;
}

void _consoleSetLogFile(void *_pf)
{
  FAT_FILE *pf=(FAT_FILE*)_pf;
  
	TLogFile *plf=&LogFile;
	
	u32 spc=FAT2_GetSecPerClus();
	if((LogFile_BufSize/512)<spc) spc=LogFile_BufSize/512;
	
  if(FAT2_GetFileSize(pf)<(spc*512)) StopFatalError(11001,"Log file size error!!\n");
  
  plf->BufPos=0;
  plf->BufSize=spc*512;
  plf->Buf[0]=0;
  plf->Pause=false;
  
  // 初期化してから書き込み先セクタ番号を設定する。
  plf->WriteTopSector=FAT2_ClustToSect(pf->firstCluster);
  
  for(u32 idx=0;idx<plf->BufSize;idx++){
    plf->Buf[idx]='-';
  }
  
  disc_WriteSectors(plf->WriteTopSector,plf->BufSize/512,plf->Buf);
  
  LogOutFlag=false;
  
  _consolePrintf("Start log file. topsector=%d size=%dbyte\n",plf->WriteTopSector,plf->BufSize);
//  _consolePrintf("%08x\n",data);
  _consolePrintf("AppName %s %s\n%s\n%s\n%s\n",ROMTITLE,ROMVERSION,ROMDATE,ROMENV,ROMWEB);
  _consolePrintf("__current pc=0x%08x sp=0x%08x\n",__current_pc(),__current_sp());
  
  DISCIO_ShowAdapterInfo();
  
  plf->TopPos=plf->BufPos;
}

void _consoleClearLogFile(void)
{
	TLogFile *plf=&LogFile;
	
	plf->WriteTopSector=0;
	plf->TopPos=0;
	plf->BufPos=0;
	plf->BufSize=0;
	plf->Buf[0]=0;
	plf->Pause=false;
	
	LogOutFlag=false;
}
bool _consoleGetLogFile(void)
{
	TLogFile *plf=&LogFile;
  if(plf->WriteTopSector==0) return(false);
  return(true);
}

void _consoleSetLogOutFlag(bool f)
{
  LogOutFlag=f;
}

static u32 wslastidx;

void _consoleLogPause(void)
{
	TLogFile *plf=&LogFile;
	plf->Pause=true;
  wslastidx=plf->BufPos/512;
}

void _consoleLogResume(void)
{
	TLogFile *plf=&LogFile;
	
	if(plf->Pause==false) return;
	plf->Pause=false;
	
  if(plf->WriteTopSector==0) return;
  
  u32 wsidx=plf->BufPos/512;
  u32 wscnt=plf->BufSize/512;
  
  if(wslastidx<=wsidx){
    disc_WriteSectors(plf->WriteTopSector+wslastidx,wsidx-wslastidx+1,&plf->Buf[wslastidx*512]);
    }else{
    disc_WriteSectors(plf->WriteTopSector+0,wscnt,&plf->Buf[0*512]);
  }
}

static void StoreLogSector(const char *s)
{
	TLogFile *plf=&LogFile;
	
  if(plf->WriteTopSector==0) return;
  
  u32 wsidx=plf->BufPos/512;
  
  while(*s!=0){
    if(*s=='\n') plf->Buf[plf->BufPos++]='\r';
    plf->Buf[plf->BufPos++]=*s++;
    if(plf->BufPos+1==plf->BufSize) plf->BufPos=plf->TopPos;
  }
  plf->Buf[plf->BufPos]='!';
  plf->Buf[plf->BufPos+1]='\0';
  
  if(plf->Pause==false) disc_WriteSectors(plf->WriteTopSector+wsidx,1,&plf->Buf[wsidx*512]);
}
