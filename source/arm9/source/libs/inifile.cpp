
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nds.h>

#include "_console.h"
#include "_const.h"

#include "arm9tcm.h"

#include "inifile.h"
#include "strtool.h"
#include "memtool.h"
#include "shell.h"

DATA_IN_AfterSystem TGlobalINI GlobalINI;

void InitINI(void)
{
    {
        TiniSystem *ps=&GlobalINI.System;
        ps->DetailsDebugLog=true;
        ps->UseGBACartForSwapMemory=true;
        ps->VRAMCacheEnabled=true;
        ps->ARM7_DebugLogFlag=true;
        ps->ChildrenMode=false;
    }
  
    {
        TiniDiskAdapter *pda=&GlobalINI.DiskAdapter;
        pda->SlowDiskAccess=false;
        pda->CheckDiskType1=false;
        pda->Ignore16bitReadTest=false;
        pda->AutoDetect1632bitsReadAccessMode=false;
        pda->AlwaysDisabledCheckDisk=false;
    }
    
    {
        TiniKeyRepeat *pkr=&GlobalINI.KeyRepeat;
        pkr->DelayCount=40;
        pkr->RateCount=10;
    }
    
    {
        TiniPlayList *ppl=&GlobalINI.PlayList;
        ppl->IgnoreComplexDecoderFlag=false;
        ppl->IgnorePlayMode_AlwaysOne_FromUserRequest=false;
    }
    
    {
        TiniFileList *pfl=&GlobalINI.FileList;
        pfl->CarSupplyMode=false;
        pfl->SwapTopBottomDisplay=false;
        pfl->PowerOffTimerWhileNoInput=3600;
        pfl->WhenMusicShowMP3Cnt=false;
    }
    
    {
        TiniGMEPlugin *pgp=&GlobalINI.GMEPlugin;
        pgp->ReverbLevel=64;
        pgp->SimpleLPF=EGMESimpleLPF_Lite;
        pgp->DefaultLengthSec=90;
        pgp->NSF_EnabledMultiTrack=true;
        pgp->GBS_EnabledMultiTrack=true;
        pgp->AY_EnabledMultiTrack=true;
        pgp->KSS_EnabledMultiTrack=true;
        pgp->HES_MaxTrackNumber=32;
    }
    
    {
        TiniMIDPlugin *pmp=&GlobalINI.MIDPlugin;
        pmp->ShowEventMessage=false;
        pmp->MaxVoiceCount=32;
        pmp->GenVolume=100;
        pmp->ReverbFactor_ToneMap=64;
        pmp->ReverbFactor_DrumMap=48;
        pmp->ShowInfomationMessages=true;
    }
    
    {
        TiniTextPlugin *ptp=&GlobalINI.TextPlugin;
        ptp->AlwayUseTextEditor=false;
    }
}

DATA_IN_IWRAM_MainPass static char section[128];
DATA_IN_IWRAM_MainPass static u32 readline;

static void readsection(char *str)
{
    str++;
  
    u32 ofs;
  
    ofs=0;
    while(*str!=']'){
        if((128<=ofs)||(*str==0)) StopFatalError(0,"line%d error.\nThe section name doesn't end correctly.\n",readline);
        section[ofs]=*str;
        str++;
        ofs++;
    }
    section[ofs]=0;
}

static u16 GetColorCoord(const char *value)
{
  u32 v=0;
  
  while(1){
    char c=*value;
    if(c==0) break;
    
    bool use=false;
    
    if(('0'<=c)&&(c<='9')){
      use=true;
      v<<=4;
      v|=0x00+(c-'0');
    }
    if(('a'<=c)&&(c<='f')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'a');
    }
    if(('A'<=c)&&(c<='F')){
      use=true;
      v<<=4;
      v|=0x0a+(c-'A');
    }
    
    if(use==false) break;
    
    value++;
  }
  
  u32 r,g,b;
  
  r=(v >> 16) & 0xff;
  g=(v >> 8) & 0xff;
  b=(v >> 0) & 0xff;
  
  return(RGB15(r/8,g/8,b/8) | BIT(15));
}

static void readkey(char *str)
{
    if(section[0]==0){
        //if(VerboseDebugLog==true) _consolePrintf("line%d error.\nThere is a key ahead of the section name.\n",readline);  
        return;
    }
  
    char key[128],value[128];
  
    u32 ofs;
  
    ofs=0;
    while(*str!='='){
        if((128<=ofs)||(*str==0)) StopFatalError(0,"line%d error.\nThe key name doesn't end correctly.\n",readline);
        key[ofs]=*str;
        str++;
        ofs++;
    }
    key[ofs]=0;
  
    str++;
  
    ofs=0;
    while(*str!=0){
        if(128<=ofs) StopFatalError(0,"line%d error.\nThe value doesn't end correctly.\n",readline);
        value[ofs]=*str;
           str++;
           ofs++;
    }
    value[ofs]=0;
  
    s32 ivalue=atoi(value);
    bool bvalue;
  
    if(ivalue==0){
        bvalue=false;
    }else{
        bvalue=true;
    }
    
    if(strcmp(section,"System")==0){
        TiniSystem *ps=&GlobalINI.System;
    
        if(strcmp(key,"DetailsDebugLog")==0){
            ps->DetailsDebugLog=bvalue;
            return;
        }
        if(strcmp(key,"UseGBACartForSwapMemory")==0){
            ps->UseGBACartForSwapMemory=bvalue;
            return;
        }
        if(strcmp(key,"VRAMCacheEnabled")==0){
            ps->VRAMCacheEnabled=bvalue;
            return;
        }
        if(strcmp(key,"ARM7_DebugLogFlag")==0){
        	ps->ARM7_DebugLogFlag=bvalue;
        	return;
        }
        if(strcmp(key,"ChildrenMode")==0){
        	ps->ChildrenMode=bvalue;
        	return;
        }
    }
  
    if(strcmp(section,"DiskAdapter")==0){
        TiniDiskAdapter *pda=&GlobalINI.DiskAdapter;
    
        if(strcmp(key,"SlowDiskAccess")==0){
            pda->SlowDiskAccess=bvalue;
            return;
        }
        if(strcmp(key,"CheckDiskType1")==0){
            pda->CheckDiskType1=bvalue;
            return;
        }
        if(strcmp(key,"Ignore16bitReadTest")==0){
            pda->Ignore16bitReadTest=bvalue;
            return;
        }
        if(strcmp(key,"AutoDetect1632bitsReadAccessMode")==0){
            pda->AutoDetect1632bitsReadAccessMode=bvalue;
            return;
        }
        if(strcmp(key,"AlwaysDisabledCheckDisk")==0){
            pda->AlwaysDisabledCheckDisk=bvalue;
            return;
        }
    }
  
    if(strcmp(section,"KeyRepeat")==0){
        TiniKeyRepeat *pkr=&GlobalINI.KeyRepeat;
        
        if(strcmp(key,"DelayCount")==0){
            pkr->DelayCount=ivalue;
            return;
        }
        if(strcmp(key,"RateCount")==0){
            pkr->RateCount=ivalue;
            return;
        }
    }
    
    if(strcmp(section,"KeyRepeat")==0){
        TiniKeyRepeat *p=&GlobalINI.KeyRepeat;
        
        if(strcmp(key,"DelayCount")==0){
            p->DelayCount=ivalue;
            return;
        }
    }
    
    if(strcmp(section,"PlayList")==0){
        TiniPlayList *ppl=&GlobalINI.PlayList;
            
        if(strcmp(key,"IgnoreComplexDecoderFlag")==0){
            ppl->IgnoreComplexDecoderFlag=bvalue;
            return;
        }
        if(strcmp(key,"IgnorePlayMode_AlwaysOne_FromUserRequest")==0){
            ppl->IgnorePlayMode_AlwaysOne_FromUserRequest=bvalue;
            return;
        }
    }
    
    if(strcmp(section,"FileList")==0){
        TiniFileList *pfl=&GlobalINI.FileList;
                
        if(strcmp(key,"CarSupplyMode")==0){
            pfl->CarSupplyMode=bvalue;
            return;
        }
        if(strcmp(key,"SwapTopBottomDisplay")==0){
            pfl->SwapTopBottomDisplay=bvalue;
            return;
        }
        if(strcmp(key,"PowerOffTimerWhileNoInput")==0){
            pfl->PowerOffTimerWhileNoInput=ivalue;
            return;
        }
        if(strcmp(key,"WhenMusicShowMP3Cnt")==0){
            pfl->WhenMusicShowMP3Cnt=bvalue;
            return;
        }
    }
    
    if(strcmp(section,"GMEPlugin")==0){
    	TiniGMEPlugin *pgp=&GlobalINI.GMEPlugin;
    	
    	if(strcmp(key,"ReverbLevel")==0){
    		pgp->ReverbLevel=ivalue;
    		return;
    	}
    	if(strcmp(key,"SimpleLPF")==0){
    		pgp->SimpleLPF=(EGMEPluginSimpleLPF)ivalue;
    		return;
    	}
    	if(strcmp(key,"DefaultLengthSec")==0){
    		pgp->DefaultLengthSec=ivalue;
    		return;
    	}
    	if(strcmp(key,"NSF_EnabledMultiTrack")==0){
    		pgp->NSF_EnabledMultiTrack=bvalue;
    		return;
    	}
    	if(strcmp(key,"GBS_EnabledMultiTrack")==0){
    		pgp->GBS_EnabledMultiTrack=bvalue;
    		return;
    	}
    	if(strcmp(key,"AY_EnabledMultiTrack")==0){
    		pgp->AY_EnabledMultiTrack=bvalue;
    		return;
    	}
    	if(strcmp(key,"KSS_EnabledMultiTrack")==0){
    		pgp->KSS_EnabledMultiTrack=bvalue;
    		return;
    	}
    	if(strcmp(key,"HES_MaxTrackNumber")==0){
    		pgp->HES_MaxTrackNumber=ivalue;
    		return;
    	}
    }

    if(strcmp(section,"MIDPlugin")==0){
        TiniMIDPlugin *pmp=&GlobalINI.MIDPlugin;
                                    
        if(strcmp(key,"ShowEventMessage")==0){
            pmp->ShowEventMessage=bvalue;
            return;
        }
        if(strcmp(key,"MaxVoiceCount")==0){
            pmp->MaxVoiceCount=ivalue;
            return;
        }
        if(strcmp(key,"GenVolume")==0){
            pmp->GenVolume=ivalue;
            return;
        }
        if(strcmp(key,"ReverbFactor_ToneMap")==0){
            pmp->ReverbFactor_ToneMap=ivalue;
            return;
        }
        if(strcmp(key,"ReverbFactor_DrumMap")==0){
            pmp->ReverbFactor_DrumMap=ivalue;
            return;
        }
        if(strcmp(key,"ShowInfomationMessages")==0){
            pmp->ShowInfomationMessages=bvalue;
            return;
        }
    }
    
    if(strcmp(section,"TextPlugin")==0){
        TiniTextPlugin *ptp=&GlobalINI.TextPlugin;
                                        
        if(strcmp(key,"AlwayUseTextEditor")==0){
            ptp->AlwayUseTextEditor=bvalue;
            return;
        }
    }
    //if(VerboseDebugLog==true) _consolePrintf("line%d error.\ncurrent section [%s] unknown key=%s value=%s\n",readline,section,key,value);
}

static void LoadINI_ins_loadbody(char *pini,u32 inisize)
{
    section[0]=0;
    readline=0;
  
    u32 iniofs=0;
  
    while(iniofs<inisize){
    
        readline++;
    
        u32 linelen=0;
    
        // Calc Line Length
        {
            char *s=&pini[iniofs];
      
            while(0x20<=*s){
                linelen++;
                s++;
                if(inisize<=(iniofs+linelen)) break;
            }
            *s=0;
        }
    
        if(linelen!=0){
            char c=pini[iniofs];
            if((c==';')||(c=='/')||(c=='!')){
                // comment line
            }else{
                if(c=='['){
                    readsection(&pini[iniofs]);
                }else{
                    readkey(&pini[iniofs]);
                }
            }
        }
    
        iniofs+=linelen;
    
        // skip NULL,CR,LF
        {
            char *s=&pini[iniofs];
      
            while(*s<0x20){
                iniofs++;
                s++;
                if(inisize<=iniofs) break;
            }
        }
        
    }
}

void LoadINI(void)
{
    InitINI();
  
    FAT_FILE *pf=Shell_FAT_fopen_Root_WithCheckExists(INIFilename);
    if(pf==NULL) return;

    u32 inisize=FAT2_GetFileSize(pf);
    char *pini=(char*)safemalloc_chkmem(&MM_Temp,inisize);
  
    FAT2_fread(pini,1,inisize,pf);
  
    FAT2_fclose(pf); pf=NULL;
  
    if(pini!=NULL){
        LoadINI_ins_loadbody(pini,inisize);
        safefree(&MM_Temp,pini); pini=NULL;
    }
  
    //GlobalINI.System.DetailsDebugLog=true;
}

