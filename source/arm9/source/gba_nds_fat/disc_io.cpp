/*

	disc_io.c

	uniformed io-interface to work with Chishm's FAT library

	Written by MightyMax
  
	Modified by Chishm:
	2005-11-06
		* Added WAIT_CR modifications for NDS

	Modified by www.neoflash.com:
	2006-02-03
		* Added SUPPORT_* defines, comment out any of the SUPPORT_* defines in disc_io.h to remove support
		  for the given interface and stop code being linked to the binary

	    * Added support for MK2 MMC interface

		* Added disc_Cache* functions

	Modified by Chishm:
	2006-02-05
		* Added Supercard SD support
*/


#include <nds.h>
#include <stdio.h>
#include "disc_io.h"

#include "unicode.h"
#include "memtool.h"
#include "dllsound.h"

// Include known io-interfaces:
#include <nds/arm9/dldi.h>

// Keep a pointer to the active interface
const DISC_INTERFACE* active_interface = NULL;

bool disc_isGBACartridge;
/*

	Hardware level disc funtions

*/

#include "_console.h"
#include "_const.h"

#include "mediatype.h"

u32 DIMediaType=DIMT_NONE;
const char *DIMediaName="No adapter.";
char DIMediaID[5]="NONE";

#define SystemCacheCount (64)
#define SystemCacheCountMask (SystemCacheCount-1)

typedef struct {
  u32 RingIndex;
  u32 SectorIndex[SystemCacheCount];
  u32 SectorData[(512/4)*SystemCacheCount];
} TSystemCache;

static TSystemCache SystemCache;

#define GetSectorDataPointer(secidx) (&SystemCache.SectorData[(512/4)*secidx])

static void disc_SystemCache_ClearAll(void)
{
  _consolePrint("Clear system cache.\n");
  
  TSystemCache *psc=&SystemCache;
  psc->RingIndex=0;
  for(u32 idx=0;idx<SystemCacheCount;idx++){
    psc->SectorIndex[idx]=(u32)-1;
  }
  MemSet32CPU(0,psc->SectorData,512*SystemCacheCount);
}

void DISCIO_ShowAdapterInfo(void)
{
  if(active_interface==NULL) StopFatalError(11801,"DISCIO_ShowAdapterInfo: Not ready active interface.\n");
  _consolePrintf("DLDI: active_interface= 0x%08x, ID= %s, Name= %s.\n",active_interface,DIMediaID,DIMediaName);
  _consolePrintf("active_interface->features= 0x%02x.\n",active_interface->features);
  _consolePrintf("BlockWriteLimitSectorsCount=%d.\n",BlockWriteLimitSectorsCount);
}

#include "maindef.h"
bool disc_Init(void) 
{
  disc_SystemCache_ClearAll();
  
  if(active_interface!=NULL) return(true);
    
  REG_EXMEMCNT &= ~(ARM7_OWNS_ROM | ARM7_OWNS_CARD);
    
  active_interface=dldiGetInternal();
  if(active_interface==NULL) StopFatalError(0,"could not find a working IO Interface");
  
      DIMediaType=active_interface->ioType;
      DIMediaName=io_dldi_data->friendlyName;
  
  {
    char *p=(char*)&active_interface->ioType;
    DIMediaID[0]=p[0];
    DIMediaID[1]=p[1];
    DIMediaID[2]=p[2];
    DIMediaID[3]=p[3];
    DIMediaID[4]=0;
  }
  
  if(active_interface->features&FEATURE_SLOT_NDS){
    disc_isGBACartridge=false;
    }else{
    disc_isGBACartridge=true;
  }
      
      DISCIO_ShowAdapterInfo();
      
      _consolePrint("DLDI driver stack check.\n");
      DTCM_StackCheck(1);
  if(active_interface->startup()==false) StopFatalError(0,"DLDI start-up failed.");
        DTCM_StackCheck(2);
  u32 buf[512/4];
        active_interface->readSectors(8192,1,buf) ;
        DTCM_StackCheck(3);
    
  return(true);
} 

bool disc_IsInserted(void) 
{
  if(active_interface==NULL) return(false);
  return(active_interface->isInserted());
} 

//static u32 saferwtmp[512/4];
bool disc_ReadSectors(u32 sector, u8 numSecs, void* buffer) 
{
  if(active_interface==NULL) return(false);
/*  
  {
    u32 IME=REG_IME;
    REG_IME=0;
    u8 *ptmp=(u8*)saferwtmp;
    u8 *pbuf=(u8*)buffer;
    while(numSecs!=0){
      active_interface->readSectors(sector,1,ptmp);
      for(u32 idx=0;idx<512;idx++){
        pbuf[idx]=ptmp[idx];
      }
      pbuf+=512;
      sector++;
      numSecs--;
    }
    REG_IME=IME;
    return(true);
  }
*/
  
  return(active_interface->readSectors(sector,numSecs,buffer));
} 

//bool SystemAfterLoaded=false;

bool disc_WriteSectors(u32 sector, u8 numSecs, const void* buffer) 
{
  if(active_interface==NULL) return(false);
/*
  {
    u32 IME=REG_IME;
    REG_IME=0;
    u8 *ptmp=(u8*)saferwtmp;
    u8 *pbuf=(u8*)buffer;
    while(numSecs!=0){
      if(SystemAfterLoaded) DLLSound_UpdateLoop(true);
      for(u32 idx=0;idx<512;idx++){
        ptmp[idx]=pbuf[idx];
      }
      active_interface->writeSectors(sector,1,ptmp);
      pbuf+=512;
      sector++;
      numSecs--;
    }
    REG_IME=IME;
    return(true);
  }
*/ 
  return(active_interface->writeSectors(sector,numSecs,buffer));
} 

static bool SystemWriteCache_Enabled=false;
static u32 SystemWriteCache_TargetSectorIndex=(u32)-1;
static u32 SystemWriteCache_Data[512/4];
void disc_SystemReadSector(u32 sector, void* buffer)
{
  if(active_interface==NULL) StopFatalError(0,"Not startupped.");
  
  if(SystemWriteCache_Enabled==true){
    if(SystemWriteCache_TargetSectorIndex==sector){
      u32 *pdst=(u32*)buffer;
      for(u32 idx=0;idx<512/4;idx++){
        pdst[idx]=SystemWriteCache_Data[idx];
      }
      return;
    }
  }
	
  TSystemCache *psc=&SystemCache;
  
  for(u32 idx=0;idx<SystemCacheCount;idx++){
    if(sector==psc->SectorIndex[idx]){
      MemCopy32CPU(GetSectorDataPointer(idx),buffer,512);
      return;
    }
  }
  
  psc->SectorIndex[psc->RingIndex]=sector;
  active_interface->readSectors(sector,1,buffer);
  MemCopy32CPU(buffer,GetSectorDataPointer(psc->RingIndex),512);
  psc->RingIndex=(psc->RingIndex+1)&SystemCacheCountMask;
}

static void disc_SystemWriteSector_Body(u32 sector, const void* buffer)
{
  if(active_interface==NULL) StopFatalError(0,"Not startupped.");
  
//  _consolePrintf("SW$%x, ",sector);
  
  TSystemCache *psc=&SystemCache;
  
  for(u32 idx=0;idx<SystemCacheCount;idx++){
    if(sector==psc->SectorIndex[idx]){
      u32 *pbuf1=(u32*)buffer;
      u32 *pbuf2=(u32*)GetSectorDataPointer(idx);
      active_interface->writeSectors(sector,1,pbuf1);
      active_interface->readSectors(sector,1,pbuf2);
      for(u32 idx=0;idx<512/4;idx++){
        if(pbuf1[idx]!=pbuf2[idx]) StopFatalError(11802,"System disk write verify error on cache.\n");
      }
      return;
    }
  }
  
  psc->SectorIndex[psc->RingIndex]=sector;
  
  {
    u32 *pbuf1=(u32*)buffer;
    u32 *pbuf2=(u32*)GetSectorDataPointer(psc->RingIndex);
    active_interface->writeSectors(sector,1,pbuf1);
    active_interface->readSectors(sector,1,pbuf2);
    for(u32 idx=0;idx<512/4;idx++){
      if(pbuf1[idx]!=pbuf2[idx]) StopFatalError(11803,"System disk write verify error on direct.\n");
    }
  }
  
  psc->RingIndex=(psc->RingIndex+1)&SystemCacheCountMask;
}

void disc_SystemWriteSector_SetWriteCache(bool Enabled)
{
  SystemWriteCache_Enabled=Enabled;
  
  if(SystemWriteCache_Enabled==true){
    _consolePrint("Start system write-back cache.\n");
    }else{
    _consolePrint("End system write-back cache.\n");
  }
  
  if(SystemWriteCache_Enabled==false){
    if(SystemWriteCache_TargetSectorIndex!=(u32)-1){
      disc_SystemWriteSector_Body(SystemWriteCache_TargetSectorIndex,SystemWriteCache_Data);
      SystemWriteCache_TargetSectorIndex=(u32)-1;
    }
  }
}
void disc_SystemWriteSector(u32 sector, const void* buffer)
{
  if(SystemWriteCache_Enabled==false){
    disc_SystemWriteSector_Body(sector,buffer);
    return;
  }
  
  if(SystemWriteCache_TargetSectorIndex!=(u32)-1){
    if(SystemWriteCache_TargetSectorIndex!=sector){
      disc_SystemWriteSector_Body(SystemWriteCache_TargetSectorIndex,SystemWriteCache_Data);
      SystemWriteCache_TargetSectorIndex=(u32)-1;
    }
  }
  
  const u32 *psrc=(u32*)buffer;
  
  for(u32 idx=0;idx<512/4;idx++){
    SystemWriteCache_Data[idx]=psrc[idx];
  }
  
  SystemWriteCache_TargetSectorIndex=sector;
}
bool disc_ClearStatus(void) 
{
  if(active_interface==NULL) return(false);
  return(active_interface->clearStatus());
} 

bool disc_Shutdown(void) 
{
  if(active_interface==NULL) return(true);
  active_interface->shutdown();
  active_interface=NULL;
  return(true);
} 

u32	disc_HostType (void)
{
  if(active_interface==NULL) return(0);
  return(active_interface->ioType);
}

