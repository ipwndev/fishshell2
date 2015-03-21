
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "shell.h"
#include "strtool.h"
#include "splash.h"
#include "arm9tcm.h"

#include "lang.h"

typedef struct {
	char *pItem;
	u32 ItemHash;
	char *pValue;
} TLangData;

typedef struct {
	char *pLangName;
	char CodePageStr[4];
} TLangList;

#define LangListMaxCount (20)
#define LangDataMaxCount (1024)

DATA_IN_AfterSystem static u32 LangListCount;
DATA_IN_AfterSystem static u32 LangDataCount;
DATA_IN_AfterSystem static TLangList *pLangList;
DATA_IN_AfterSystem static TLangData *pLangData;

#include "lang_TextPool.h"

// ---------------------------------------------------------

static u32 CalcItemHash(const char *pItem)
{
	u32 hash=0;
	while(*pItem!=0){
		hash+=*pItem++;
	}
	return(hash);
}

// ---------------------------------------------------------
static bool isFileNameEqual(const char *s1,const char *s2)
{
	if((s1==0)&&(s2==0)) return(true);
	if((s1==0)||(s2==0)) return(false);
  
	while(1){
		char c1=*s1++;
		char c2=*s2++;
		if((0x61<=c1)&&(c1<=0x7a)) c1-=0x20;
		if((0x61<=c2)&&(c2<=0x7a)) c2-=0x20;
		
		if((c1=='.')||(c1==0)||(c2==0)){
			if((c1==0)&&(c2==0)){
				return(true);
			}else if((c1=='.')&&(c2==0)){
				return(true);
			}else{
				return(false);
			}
		}
		
		if(c1!=c2) return(false);
	}
}

// ---------------------------------------------------------

DATA_IN_IWRAM_MainPass void Lang_Load(void)
{
	Splash_Update();
	FAT_FILE *pf=Shell_FAT_fopen_Language_messages();
	if(pf==NULL) StopFatalError(13201,"Not found language file.\n");
  
	u32 bufsize=FAT2_GetFileSize(pf);
	u8 *pbuf=(u8*)safemalloc_chkmem(&MM_Temp,bufsize);
  
	FAT2_fread_fast(pbuf,1,bufsize,pf);
	FAT2_fclose(pf);
	
	Splash_Update();
	
	LangDataCount=0;
	pLangData=(TLangData*)safemalloc_chkmem(&MM_System,sizeof(TLangData)*LangDataMaxCount);
  
	LangListCount=0;
	pLangList=(TLangList*)safemalloc_chkmem(&MM_System,sizeof(TLangList)*LangListMaxCount);
	
	InitTextPool();
        
	char linebuf[512+1];
	u32 linelen=0;
	linebuf[linelen]=0;
	  
	s32 ofs=0;
	  
	while(ofs<bufsize){
		char ch=pbuf[ofs++];
		if(ch==0) break;
	    
		if(ch<0x20){
			Splash_Update();
			if((ch==0x0d)||(ch==0x0a)){
				linebuf[linelen]=0;
	        
				if((linelen!=0)&&(linebuf[0]!=';')&&(linebuf[0]!='#')){
					u32 eqofs=0;
					for(u32 idx=0;idx<linelen;idx++){
						if(linebuf[idx]=='='){
							eqofs=idx;
							break;
						}
					}
					if(eqofs!=0){
						linebuf[eqofs]=0;
	            
						if(LangDataCount==LangDataMaxCount) StopFatalError(13202,"Language data buffer overflow.\n");
	            
						TLangData *pld=&pLangData[LangDataCount];
						pld->pItem=TextPoolChar_AllocateCopy(&linebuf[0]);
						pld->ItemHash=CalcItemHash(pld->pItem);
						pld->pValue=TextPoolChar_AllocateCopy(&linebuf[eqofs+1]);
						LangDataCount++;
					}
				}
	        
				linelen=0;
			}
		}else{
			if((linelen+1)<512) linebuf[linelen++]=ch;
		}
	}
	
	if(FAT2_chdir_Alias(DefaultLanguageDataPath)==true){
		const char *pafn;
		u32 FAT_FileType=FAT2_FindFirstFile(&pafn);
            
		while(FAT_FileType!=FT_NONE){
			Splash_Update();
			switch(FAT_FileType){
				case FT_NONE: break;
				case FT_DIR: break;
				case FT_FILE: {
					if(isFileNameEqual(pafn,"messages")==true){
						const char *ptmp=pafn;
						while(*ptmp!=0 && *ptmp!='.') ptmp++;
						if(*ptmp!=0) {
							TLangList *pll=&pLangList[LangListCount];
							pll->CodePageStr[0]=*ptmp++;
							pll->CodePageStr[1]=*ptmp++;
							pll->CodePageStr[2]=*ptmp++;
							pll->CodePageStr[3]=0;
							LangListCount++;
						}
					}
				} break;
			}
              
			FAT_FileType=FAT2_FindNextFile(&pafn);
		}
	}
	
	EndTextPool();
  
	pLangData=(TLangData*)saferealloc(&MM_System,pLangData,sizeof(TLangData)*LangDataCount);
	pLangList=(TLangList*)saferealloc(&MM_System,pLangList,sizeof(TLangList)*LangListCount);
  
	if(pbuf!=NULL){
		safefree(&MM_Temp,pbuf); pbuf=NULL;
	}
	Splash_Update();
	_consolePrintf("Language count= %d\n",LangListCount);
	_consolePrintf("Language materials count= %d\n",LangDataCount);
}

void Lang_Free(void)
{
	if(pLangList!=NULL){
		for(u32 idx=0;idx<LangListCount;idx++){
			TLangList *pld=&pLangList[idx];
			pld->pLangName=NULL;
			pld->CodePageStr[0]=0;
			pld->CodePageStr[1]=0;
			pld->CodePageStr[2]=0;
			pld->CodePageStr[3]=0;
		}
 	   	safefree(&MM_System,pLangList); pLangList=NULL;
	}
  
	if(pLangData!=NULL){
		for(u32 idx=0;idx<LangDataCount;idx++){
			TLangData *pld=&pLangData[idx];
			pld->pItem=NULL;
			pld->ItemHash=0;
			pld->pValue=NULL;
		}
		safefree(&MM_System,pLangData); pLangData=NULL;
	}
  
	LangListCount=0;
	LangDataCount=0;
  
	FreeTextPool();
}

const char* Lang_GetUTF8_internal(const char *pItemName,const char *pErrorMsg)
{
	if(LangDataCount==0) StopFatalError(13203,"Lang_GetUTF8: Language file not loaded.\n");
  
	u32 hash=CalcItemHash(pItemName);
  
	for(u32 idx=0;idx<LangDataCount;idx++){
		TLangData *pld=&pLangData[idx];
		if(hash==pld->ItemHash){
			if(isStrEqual(pItemName,pld->pItem)==true) return(pld->pValue);
		}
	}
  
	_consolePrintf("Lang_GetUTF8: Can not found language item. [%s]\n",pItemName);
	return(pErrorMsg);
}

