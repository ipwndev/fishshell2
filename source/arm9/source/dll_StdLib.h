
static void _ShowLogHalt(void)
{
  __StopFatalError__("InternalPlugin",0,"unknown",99999,"Plugin internal error.");
}

static long int FileSys_ftell (int hFile)
{
  return(FAT2_ftell((FAT_FILE*)hFile));
}

static int FileSys_fseek(int hFile, u32 offset, int origin)
{
  return(FAT2_fseek((FAT_FILE*)hFile,offset,origin));
}

static u32 FileSys_fread (void* buffer, u32 size, u32 count, int hFile)
{
  if((size==0)||(count==0)) return(0);
  
//  _consolePrintf("fread:0x%x,%d -> 0x%08x\n",FAT2_ftell((FAT_FILE*)hFile),size*count,buffer);
  u32 res=FAT2_fread(buffer,size,count,(FAT_FILE*)hFile);

  return(res);
    
  char* data = (char*)buffer;
  for(u32 idx=0;idx<32;idx++){
    _consolePrintf("%02x,",data[idx]);
  }
  _consolePrint("\n");
  
  return(res);
}

static u32 FileSys_fskip (u32 size, u32 count, int hFile)
{
  if((size==0)||(count==0)) return(0);
  
  u32 res=FAT2_fskip(size,count,(FAT_FILE*)hFile);
  
  return(res);
}

static void* _memchr(const void *buf, int ch, size_t n)
{
  return((void*)memchr(buf,ch,n));
}

static char* _strchr(const char *s, int c)
{
  return((char*)strchr(s,c));
}

static char* _strpbrk(const char *s, const char *accept)
{
  return((char*)strpbrk(s,accept));
}

static char* _strrchr(const char *s, int c)
{
  return((char*)strrchr(s,c));
}

static char* _strsep(char **stringp, const char *delim)
{
  return(NULL);
  //return((char*)strsep(stringp,delim));
}

static char* _strstr(const char *haystack, const char *needle)
{
  return((char*)strstr(haystack,needle));
}

#define PluginGardMemorySize (64*1024)

static void* plugin_safemalloc(TMM *pMM,int size)
{
//  _consolePrintf("%x M%d, ",pMM,size);
  
  void *p=safemalloc(pMM,size+PluginGardMemorySize);
  
  if(p==NULL){
    PrintFreeMem();
    _consolePrintf("plugin_safemalloc(%d) fail. ",size);
    return(NULL);
  }
  
  safefree(pMM,p);
//  _consolePrintf("plugin_safemalloc(%d) ok. ",size);
  return(safemalloc(pMM,size));
}

static void plugin_safefree(TMM *pMM,void *ptr)
{
//  _consolePrint("F, ");
  
  safefree(pMM,ptr);
}

static void* plugin_safecalloc(TMM *pMM,int nmemb, int size)
{
//  _consolePrintf("C%d, ",nmemb*size);
  
  void *p=safemalloc(pMM,(nmemb*size)+PluginGardMemorySize);
  
  if(p==NULL){
    PrintFreeMem();
    _consolePrintf("plugin_safecalloc(%d,%d) fail. ",nmemb,size);
    return(NULL);
  }
  
  safefree(pMM,p);
//  _consolePrintf("plugin_safecalloc(%d,%d) ok. ",nmemb,size);
  return(safecalloc(pMM,1,nmemb*size));
}

static void* plugin_saferealloc(TMM *pMM,void *ptr, int size)
{
//  _consolePrintf("R%d, ",size);
  
  return(saferealloc(pMM,ptr,size));
}

DATA_IN_AfterSystem static EPluginType MWin_CurrentPluginType=EPT_None;
DATA_IN_AfterSystem static s32 MWin_Max=0;
DATA_IN_AfterSystem static char *MWin_pTitleStr=NULL;

static void MWin_ProgressShow(const char *TitleStr,s32 Max)
{
//  _consolePrintf("PrgShow: %s,%d\n",TitleStr,Max);
  
  MWin_Max=Max;
  
  if(MWin_pTitleStr!=NULL) StopFatalError(11201,"MWin_ProgressShow: Already showed.\n");
  
  if(str_isEmpty(TitleStr)==true) TitleStr="";
  MWin_pTitleStr=str_AllocateCopy(&MM_Temp,TitleStr);
  
  CallBack_MWin_ProgressShow(MWin_pTitleStr,MWin_Max);
}

static void MWin_ProgressSetPos(s32 Position)
{
//  _consolePrintf("PrgSetPos: %d\n",Position);
  
  if(MWin_Max==0) return;
  if(MWin_Max<Position) return;
  
  if(MWin_pTitleStr==NULL) StopFatalError(11202,"MWin_ProgressSetPos: MWin_pTitleStr is NULL.\n");
  
  CallBack_MWin_ProgressSetPos(MWin_pTitleStr,Position,MWin_Max);
}

static void MWin_ProgressHide(void)
{
//  _consolePrint("PrgHide: ok.\n");
  
  if(MWin_pTitleStr==NULL) StopFatalError(11203,"MWin_ProgressHide: No show.\n");
  
  if(MWin_pTitleStr!=NULL){
    safefree(&MM_Temp,MWin_pTitleStr); MWin_pTitleStr=NULL;
  }
  
  MWin_Max=0;
  
  CallBack_MWin_ProgressHide();
}

static inline const TPlugin_StdLib *Plugin_GetStdLib(void)
{
	DATA_IN_AfterSystem static TPlugin_StdLib sPlugin_StdLib={
    _consolePrint,_consolePrintf,
    _consolePrintSet,
    _ShowLogHalt,
    MWin_ProgressShow,MWin_ProgressSetPos,MWin_ProgressHide,
    
    MemCopy8CPU,MemCopy16CPU,MemCopy32CPU,
    MemSet16CPU,MemSet32CPU,
    plugin_safemalloc,plugin_safefree,plugin_safecalloc,plugin_saferealloc,
    
    rand,
    FileSys_fread,FileSys_fskip,FileSys_fseek,FileSys_ftell,
    sprintf,snprintf,
    _memchr,memcmp,memcpy,memmove,memset,
    abs,labs,llabs,fabs,fabsf,
    atof,atoi,atol,atoll,
    strcat,_strchr,strcmp,strcoll,strcpy,strcspn,strlen,strncat,strncmp,strncpy,_strpbrk,_strrchr,_strsep,strspn,_strstr,strtok,strxfrm,
    
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  };
  
  return(&sPlugin_StdLib);
}
