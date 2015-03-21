
static void AjustInsideSrcForBG(TRect *pr,u32 HeightPadding)
{
  s32 sw=(ScreenWidth*2)*0x100/MultipleFix8;
  s32 sh=((ScreenHeight*2)+HeightPadding)*0x100/MultipleFix8;
  
  pr->w=sw;
  pr->h=sh;
  
  if((SrcWidth-sw)<pr->x) pr->x=SrcWidth-sw;
  if((SrcHeight-sh)<pr->y) pr->y=SrcHeight-sh;
  if(pr->x<0){
    pr->x=0;
    if(SrcWidth<pr->w) pr->w=SrcWidth;
  }
  if(pr->y<0){
    pr->y=0;
    if(SrcHeight<pr->h) pr->h=SrcHeight;
  }
}

static void CreateBG(void)
{
    if(VerboseDebugLog==true) _consolePrint("CreateBG start.\n");
  
//  MCU_ClearAllCache();
  
  u32 HeightPadding=0;
  if(ProcState.Image.EffectHeightPadding==true) HeightPadding=72;
  
  CglCanvas *pcan=new CglCanvas(&MM_Temp,NULL,ScreenWidth*2,(ScreenHeight*2)+HeightPadding,pf15bit);
  pcan->FillFull(RGB15(16,16,16)|BIT15);
  
  TRect rect=DstRect;
  AjustInsideSrcForBG(&rect,HeightPadding);
  
  s32 xalign=0,yalign=0;
  
  if((rect.x&MCUSizeMask)!=0){
    xalign=rect.x&MCUSizeMask;
    rect.w+=xalign;
    rect.x-=xalign;
  }
  if((rect.y&MCUSizeMask)!=0){
    yalign=rect.y&MCUSizeMask;
    rect.h+=yalign;
    rect.y-=yalign;
  }
  
  u32 MCUShowSize=MCUSize*MultipleFix8/0x100;
  xalign=xalign*MultipleFix8/0x100;
  yalign=yalign*MultipleFix8/0x100;
  
  u32 xofs=rect.x/MCUSize;
  u32 yofs=rect.y/MCUSize;
  u32 xcnt=(rect.w+(MCUSize-1))/MCUSize;
  u32 ycnt=(rect.h+(MCUSize-1))/MCUSize;
  
//  if(xalign!=0) xcnt++;
//  if(yalign!=0) ycnt++;
  
  if(MCUXCount<xcnt){
    xofs=0;
    xcnt=MCUXCount;
    }else{
    if(MCUXCount<(xofs+xcnt)){
      xcnt=MCUXCount-xofs;
    }
  }
  if(MCUYCount<ycnt){
    yofs=0;
    ycnt=MCUYCount;
    }else{
    if(MCUYCount<(yofs+ycnt)){
      ycnt=MCUYCount-yofs;
    }
  }
  
  if(VerboseDebugLog==true) _consolePrint("CreateBG get image.\n");
  
  CglCanvas *pDrawTempBM0=new CglCanvas(&MM_Temp,pDrawTempMemory0,MCUShowSize,MCUShowSize,pf15bit);
  CglCanvas *pDrawTempBM1=new CglCanvas(&MM_Temp,pDrawTempMemory1,MCUShowSize,MCUShowSize,pf15bit);
  
  xalign=xalign&~1;
  
  CglCanvas *pprgcan=pScreenMain->pViewCanvas;
  
  {
    CglB15 *pprgoff=ImageView_GetSkin(EIVS_prgbar_off);
    CglB15 *pprgon=ImageView_GetSkin(EIVS_prgbar_on);
    u32 prgh=pprgoff->GetHeight();
    u32 prgy=ScreenHeight-prgh;
    pprgoff->BitBlt(pprgcan,0,prgy,ScreenWidth,prgh,0,0);
    
    u32 prglx=0;
    
    s32 ypos=-yalign;
    for(u32 yidx=0;yidx<ycnt;yidx++){
      u32 prgx=(yidx*ScreenWidth)/ycnt;
      if(prglx!=prgx){
        pprgon->BitBlt(pprgcan,prglx,prgy,prgx-prglx,prgh,prglx,0);
        prglx=prgx;
      }
      s32 xpos=-xalign;
      ReadMCUBlock(MCUSize*(yofs+yidx+1));
      for(u32 xidx=0;xidx<xcnt;xidx++){
        u8 *pMCUBuf=MCU_GetMCU(xofs+xidx,yofs+yidx);
        u16 *_ptmpbm0=pDrawTempBM0->GetVRAMBuf();
        u16 *_ptmpbm1=pDrawTempBM1->GetVRAMBuf();
        
        switch(MultipleFix8){
          case (u32)(0.5*0x100): RedrawImage_ins_x50(pMCUBuf,_ptmpbm0,_ptmpbm1); break;
          case 1*0x100: RedrawImage_ins_x100(pMCUBuf,_ptmpbm0,_ptmpbm1,0); break;
          case 2*0x100: RedrawImage_ins_x200(pMCUBuf,_ptmpbm0,_ptmpbm1); break;
        }
        
        pDrawTempBM1->BitBlt(pcan,xpos,ypos,MCUShowSize,MCUShowSize,0,0,false);
        
        xpos+=MCUShowSize;
      }
      ypos+=MCUShowSize;
    }
  }
  
  if(pDrawTempBM0!=NULL){
    delete pDrawTempBM0; pDrawTempBM0=NULL;
  }
  if(pDrawTempBM1!=NULL){
    delete pDrawTempBM1; pDrawTempBM1=NULL;
  }
  
  if((SrcWidth*MultipleFix8/0x100)<(ScreenWidth*2)){
    pcan->BitBlt(pcan,ScreenWidth,0,ScreenWidth,(ScreenHeight*2)+HeightPadding,0,0,false);
  }
  
  if(ProcState.Image.EffectPastelForTopBG==true){
      if(VerboseDebugLog==true) _consolePrint("CreateBG pastel effect for top bg.\n");
    for(u32 y=0;y<ScreenHeight;y++){
      u16 *pbuf=pcan->GetScanLine(y);
      for(u32 x=0;x<ScreenWidth;x++){
        pbuf[x]=(((pbuf[x]&RGB15(30,30,30))>>1)+RGB15(16,16,16)) | BIT15;
      }
    }
  }
  if(ProcState.Image.EffectPastelForBottomBG==true){
      if(VerboseDebugLog==true) _consolePrint("CreateBG pastel effect for bottom bg.\n");
    for(u32 y=ScreenHeight+HeightPadding;y<(ScreenHeight*2)+HeightPadding;y++){
      u16 *pbuf=pcan->GetScanLine(y);
      for(u32 x=0;x<ScreenWidth;x++){
        u32 col=pbuf[x];
        u32 b=(col>>10)&0x1f;
        u32 g=(col>>5)&0x1f;
        u32 r=(col>>0)&0x1f;
        u32 per=12; // 37.5%
        b=(b*per)/32; g=(g*per)/32; r=(r*per)/32; 
        u32 ofs=32-per;
        pbuf[x]=RGB15(ofs+r,ofs+g,ofs+b)|BIT15;
//        pbuf[x]=(((pbuf[x]&RGB15(28,28,28))>>2)+RGB15(24,24,24)) | BIT15;
      }
    }
  }
  
  if(VerboseDebugLog==true) _consolePrintf("CreateBG write to %s.\n",BGBMPFilename);
  
  {
    FAT_FILE *pwfh=Shell_FAT_fopenwrite_Internal(BGBMPFilename);
    if(pwfh==NULL) StopFatalError(17501,"Open for write failed.\n");
    
    u32 BGBMPType=EBGBT_15bit;
    FAT2_fwrite(&BGBMPType,1,4,pwfh);
    
    {
      CglB15 *pprgoff=ImageView_GetSkin(EIVS_prgbar_off);
      CglB15 *pprgon=ImageView_GetSkin(EIVS_prgbar_on);
      u32 prgh=pprgoff->GetHeight();
      u32 prgy=ScreenHeight-prgh;
      pprgoff->BitBlt(pprgcan,0,prgy,ScreenWidth,prgh,0,0);
      
      u8 *pbuf=(u8*)pcan->GetVRAMBuf();
      u32 bufsize=pcan->GetWidth()*2;
      u32 prglx=0;
      
      for(u32 y=0;y<ScreenHeight;y++){
        FAT2_fwrite(pbuf,1,bufsize,pwfh);
        pbuf+=bufsize;
        u32 prgx=(y*(ScreenWidth/2))/ScreenHeight;
        if(prglx!=prgx){
          pprgon->BitBlt(pprgcan,prglx,prgy,prgx-prglx,prgh,prglx,0);
          prglx=prgx;
        }
      }
      pbuf+=bufsize*HeightPadding;
      for(u32 y=0;y<ScreenHeight;y++){
        FAT2_fwrite(pbuf,1,bufsize,pwfh);
        pbuf+=bufsize;
        u32 prgx=(ScreenWidth/2)+((y*(ScreenWidth/2))/ScreenHeight);
        if(prglx!=prgx){
          pprgon->BitBlt(pprgcan,prglx,prgy,prgx-prglx,prgh,prglx,0);
          prglx=prgx;
        }
      }
    }
    
    FAT2_fclose(pwfh);
  }
  
  if(pcan!=NULL){
    delete pcan; pcan=NULL;
  }
  
  if(VerboseDebugLog==true) _consolePrint("CreateBG end.\n");
}
