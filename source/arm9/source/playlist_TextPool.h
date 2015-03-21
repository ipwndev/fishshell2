
DATA_IN_AfterSystem static u8 *pTextPoolStart=NULL;
DATA_IN_AfterSystem static u8 *pTextPoolEnd;
DATA_IN_AfterSystem static u8 *pTextPoolPos;

static void FreeTextPool(void)
{
  if(pTextPoolStart!=NULL){
    safefree(&MM_PlayList,pTextPoolStart); pTextPoolStart=NULL;
  }
}

static void InitTextPool(void)
{
  FreeTextPool();
  
  u32 maxsize=GetMaxMemoryBlockSize();
  const u32 decsize=32*1024;
  if(decsize<maxsize) maxsize-=decsize;
  if(maxsize<decsize) maxsize=decsize;
  
  pTextPoolStart=(u8*)safemalloc_chkmem(&MM_PlayList,maxsize);
  
  pTextPoolEnd=&pTextPoolStart[maxsize];
  pTextPoolPos=pTextPoolStart;
}

static void EndTextPool(void)
{
	if(VerboseDebugLog==true) _consolePrintf("pTextPool used.%dkbyte remain.%dkbyte\n",(pTextPoolPos-pTextPoolStart)/1024,(pTextPoolEnd-pTextPoolPos)/1024);
  
  u8 *ptmp=(u8*)saferealloc(&MM_PlayList,pTextPoolStart,pTextPoolPos-pTextPoolStart);
  
  if(pTextPoolStart!=ptmp) StopFatalError(11602,"TextPool saferealloc error. 0x%x!=0x%x\n",pTextPoolStart,ptmp);
  
  if(VerboseDebugLog==true) _consolePrint("Succeeded memory optimization.\n");
  
  pTextPoolStart=ptmp;
  
  pTextPoolEnd=NULL;
  pTextPoolPos=NULL;
}

static u8* GetTextPool(u32 size)
{
  size=(size+3)&~3;
  if(size==0) size=4;
  
  u8 *p=pTextPoolPos;
  
  if(pTextPoolEnd<=(p+size)) StopFatalError(11603,"TextPool memory overflow. size=%dbyte\n",size);
  
  pTextPoolPos+=size;
  
  return(p);
}

static void* GetTextPoolVoid(u32 size)
{
  return((void*)GetTextPool(size));
}

static char* GetTextPoolChar(u32 size)
{
  return((char*)GetTextPool((size+1)*1));
}

static UnicodeChar* GetTextPoolUnicode(u32 size)
{
  return((UnicodeChar*)GetTextPool((size+1)*2));
}

static char* TextPoolChar_AllocateCopy(const char *psrc)
{
  u32 len=strlen(psrc);
  char *pdst=GetTextPoolChar(len);
  strcpy(pdst,psrc);
  return(pdst);
}

static UnicodeChar* TextPoolUnicode_AllocateCopy(const UnicodeChar *psrc)
{
  u32 len=Unicode_GetLength(psrc);
  UnicodeChar *pdst=GetTextPoolUnicode(len);
  Unicode_Copy(pdst,psrc);
  return(pdst);
}

