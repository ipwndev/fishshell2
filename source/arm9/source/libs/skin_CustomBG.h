
static CglCanvas* LoadCustomBG(void)
{
  FAT_FILE *pfh=Shell_FAT_fopen_Internal(BGBMPFilename);
  if(pfh==NULL){
    _consolePrint("Open for read failed.\n");
    return(NULL);
  }
  
  s32 w=ScreenWidth*2;
  s32 h=ScreenHeight*2;
  
  u32 BGBMPType;
  FAT2_fread(&BGBMPType,1,4,pfh);
  
  CglCanvas *pCustomBG=new CglCanvas(&MM_Skin,NULL,w,h,pf15bit);
  if(pCustomBG==NULL){
    _consolePrint("LoadCustomBG: pCustomBG Memory overflow.\n");
    FAT2_fclose(pfh);
    return(false);
  }
  
  switch(BGBMPType){
    case EBGBT_None: {
      delete pCustomBG; pCustomBG=NULL;
    } break;
    case EBGBT_8bit: {
      _consolePrint("Load 8bit CustomBG.\n");
      u16 pal15[256];
      FAT2_fread(pal15,1,256*2,pfh);
      Splash_Update();
      {
        u32 *ptmp=(u32*)safemalloc_chkmem(&MM_Temp,w*h);
        FAT2_fread(ptmp,1,w*h,pfh);
        Splash_Update();
        u32 *pDstBuf=(u32*)pCustomBG->GetVRAMBuf();
        for(u32 idx=0;idx<w*h/4;idx++){
          u32 palidx=*ptmp++;
          u32 col;
          col=pal15[(palidx>>0)&0xff]<<0;
          col|=pal15[(palidx>>8)&0xff]<<16;
          *pDstBuf++=col;
          col=pal15[(palidx>>16)&0xff]<<0;
          col|=pal15[(palidx>>24)&0xff]<<16;
          *pDstBuf++=col;
        }
        ptmp-=w*h/4;
        safefree(&MM_Temp,ptmp); ptmp=NULL;
        Splash_Update();
      }
    } break;
    case EBGBT_15bit: {
      _consolePrint("Load 15bit CustomBG.\n");
      for(u32 y=0;y<h;y+=64){
    	  Splash_Update();
        u16 *pDstBuf=pCustomBG->GetScanLine(y);
        FAT2_fread_fast(pDstBuf,1,w*64*2,pfh);
        Splash_Update();
      }
    } break;
    default: {
      delete pCustomBG; pCustomBG=NULL;
    } break;
  }
  
  FAT2_fclose(pfh);
  
  return(pCustomBG);
}

