
u16* TextView_DrawID3Tag_asm_Fill25per(u16 *pbuf,u32 size)
{
	asm volatile (
	"stmfd sp!, {r4,r5,r6,lr} \n\t"
	" \n\t"
	"ldr r2,=0x739C739C \n\t" // RGB15(28,28,28)<<16 | RGB15(28,28,28)
	"ldr r3,=(1<<15)|(1<<31) \n\t"
	" \n\t"
	"TextView_DrawID3Tag_asm_Fill25per_loop: \n\t"
	" \n\t"
	"ldmia r0,{r4,r5,r6,lr} \n\t"
	"and r4,r4,r2 \n\t"
	"orr r4,r3,r4,lsr #2 \n\t"
	"and r5,r5,r2 \n\t"
	"orr r5,r3,r5,lsr #2 \n\t"
	"and r6,r6,r2 \n\t"
	"orr r6,r3,r6,lsr #2 \n\t"
	"and lr,lr,r2 \n\t"
	"orr lr,r3,lr,lsr #2 \n\t"
	"stmia r0!,{r4,r5,r6,lr} \n\t"
	" \n\t"
	"subs r1,#2*4 \n\t"
	"bne TextView_DrawID3Tag_asm_Fill25per_loop \n\t"
	" \n\t"
	"ldmfd sp!, {r4,r5,r6,pc} \n\t"
	".ltorg"
	:::"memory"
	);
}
u16* TextView_DrawID3Tag_asm_Fill50per(u16 *pbuf,u32 size)
{
	asm volatile (
	"stmfd sp!, {r4,r5,r6,lr} \n\t"
	" \n\t"
	"ldr r2,=0x7BDE7BDE \n\t" // RGB15(30,30,30)<<16 | RGB15(30,30,30)
	"ldr r3,=(1<<15)|(1<<31) \n\t"
	" \n\t"
	"TextView_DrawID3Tag_asm_Fill50per_loop: \n\t"
	" \n\t"
	"ldmia r0,{r4,r5,r6,lr} \n\t"
	"and r4,r4,r2 \n\t"
	"orr r4,r3,r4,lsr #1 \n\t"
	"and r5,r5,r2 \n\t"
	"orr r5,r3,r5,lsr #1 \n\t"
	"and r6,r6,r2 \n\t"
	"orr r6,r3,r6,lsr #1 \n\t"
	"and lr,lr,r2 \n\t"
	"orr lr,r3,lr,lsr #1 \n\t"
	"stmia r0!,{r4,r5,r6,lr} \n\t"
	" \n\t"
	"subs r1,#2*4 \n\t"
	"bne TextView_DrawID3Tag_asm_Fill50per_loop \n\t"
	" \n\t"
	"ldmfd sp!, {r4,r5,r6,pc} \n\t"
	".ltorg"
	:::"memory"
	);
}

static bool TextView_DrawID3Tag(CglCanvas *pDstBM)
{
  if(PlayList_isOpened()==false) return(false);

  u32 LinesCount=1; // for Filename.
  
  if(ID3Tag.LinesCount==0){
    LinesCount+=1; // for Not exists text.
    }else{
    LinesCount+=ID3Tag.LinesCount;
    if(LinesCount==2) LinesCount++;
  }
  
  u32 x=8,y=8,h=glCanvasTextHeight+2;
  
  if(true){//DrawFrame
    const u32 DstBMWidth=ScreenWidth; // pDstBM->GetWidth();
    
    u16 *pb=pDstBM->GetVRAMBuf();
    u32 size;
    
    size=4*DstBMWidth;
    pb+=size;
    
    size=2*DstBMWidth;
    pb=TextView_DrawID3Tag_asm_Fill50per(pb,size);
    
    size=(2+(h*LinesCount)+0)*DstBMWidth;
    pb=TextView_DrawID3Tag_asm_Fill25per(pb,size);
    
    {
      size=DstBMWidth;
      s32 posx;
      {
        u32 ofs=DLLSound_GetPosOffset(),max=DLLSound_GetPosMax();
        if((8*1024*1024)<=max){
          ofs/=1024;
          max/=1024;
        }
        posx=(ofs*size)/max;
        if(size<posx) posx=size;
      }
      for(u32 idx=0;idx<posx;idx++){
        pb[size*0]=((pb[size*0]&RGB15(30,30,30))>>1) |BIT15;
        pb[size*1]=((pb[size*1]&RGB15(30,30,30))>>1) |BIT15;
        pb[size*2]=((pb[size*2]&RGB15(30,30,30))>>1) |BIT15;
        pb++;
      }
      for(u32 idx=posx;idx<size;idx++){
        pb[size*0]=((pb[size*0]&RGB15(28,28,28))>>2) |BIT15;
        pb[size*1]=((pb[size*1]&RGB15(28,28,28))>>2) |BIT15;
        pb[size*2]=((pb[size*2]&RGB15(28,28,28))>>2) |BIT15;
        pb++;
      }
      pb+=size*2;
    }
    
    size=3*DstBMWidth;
    pb=TextView_DrawID3Tag_asm_Fill25per(pb,size);
    
    size=2*DstBMWidth;
    pb=TextView_DrawID3Tag_asm_Fill50per(pb,size);
  }
  
  y++;
  
//#define UseShadow

#ifdef UseShadow
  u16 collow=RGB15(0,0,0)|BIT15;
#endif
  u16 colhigh=ColorTable.FileList.ID3TagText;
  
  { // for Filename.
    char idxstr[32];
    snprintf(idxstr,32,"%d/%d",1+PlayList_GetCurrentIndex(),PlayList_GetFilesCount());
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,idxstr);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,idxstr);
    u32 w=pDstBM->GetTextWidthA(idxstr)+4;

    const UnicodeChar *pstr=PlayList_GetCurrentFilename();
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutW(x+w+1,y+1,pstr);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutW(x+w+0,y+0,pstr);
    y+=h;
    LinesCount--;
  }
  
  const u32 volper=(strpcmGetAudioVolume64()*100)/64;
  
  if(ID3Tag.LinesCount!=0){
    u32 strwofs=ID3Tag.LinesCount;
    if(strwofs<=2){
      strwofs=0;
      }else{
      strwofs-=2;
    }
    
    for(u32 idx=0;idx<2;idx++){
      const char *pforma=NULL;
      switch(idx){
        case 0: pforma="0:00:00"; break;
        case 1: pforma="%d%% 0:00:00"; break;
      }
      
      UnicodeChar *pstrw=NULL;
      u32 strwidx=strwofs+idx;
      if(strwidx<ID3Tag.LinesCount) pstrw=ID3Tag.ppLines[strwidx];
      
      if((pforma!=NULL)&&(pstrw!=NULL)&&(pstrw[0]!=0)){
        char stra[128];
        snprintf(stra,64,pforma,volper);
        u32 w=ScreenWidth-4-pDstBM->GetTextWidthA(stra)-x;
        while(w<pDstBM->GetTextWidthW(pstrw)){
          u32 lastpos=0;
          while(pstrw[lastpos]!=0){
            lastpos++;
          }
          pstrw[lastpos-1]=0;
        }
      }
    }
  }
  
  if(ID3Tag.LinesCount==0){
/*
    const char *pstr="Not found file information.";
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,pstr);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,pstr);
*/
    y+=h;
    
    }else{
    u32 lineindex=0;
    
    for(u32 idx=0;idx<LinesCount;idx++){
      const UnicodeChar *pstr=NULL;
      
      if(idx<ID3Tag.LinesCount) pstr=ID3Tag.ppLines[idx];
      
      if(pstr!=NULL){
#ifdef UseShadow
        pDstBM->SetFontTextColor(collow);
        pDstBM->TextOutW(x+1,y+1,pstr);
#endif
        pDstBM->SetFontTextColor(colhigh);
        pDstBM->TextOutW(x+0,y+0,pstr);
      }
      y+=h;
      
      lineindex++;
    }
  }
  
  u32 cursec,playsec;
  bool ShowTime=DLLSound_GetTimeSec(&cursec,&playsec);
  
  if(ID3Tag.LinesCount==0){
    y-=h*1;
    u32 x,w;
    char str[128];
    
    {
      w=pDstBM->GetTextWidthA("%d%% 0:00:00 / 0:00:00");
      x=ScreenWidth-4-w;
    }
    
    if(ShowTime==false){
      snprintf(str,64,"%d%% 0:00:00",volper);
      }else{
      snprintf(str,64,"%d%% %d:%02d:%02d",volper,cursec/60/60,(cursec/60)%60,cursec%60);
    }
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,str);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,str);
    
    snprintf(str,64,"%d%% 0:00:00",volper);
    x+=pDstBM->GetTextWidthA(str);
    
    if(ShowTime==false){
      snprintf(str,64," / 0:00:00");
      }else{
      snprintf(str,64," / %d:%02d:%02d",playsec/60/60,(playsec/60)%60,playsec%60);
    }
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,str);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,str);
    
    y+=h;
    
    }else{
    y-=h*2;
    u32 x,w;
    char str[64];
    
    {
      w=pDstBM->GetTextWidthA("0:00:00");
      x=ScreenWidth-4-w;
    }
    if(ShowTime==false){
      snprintf(str,64,"0:00:00");
      }else{
      snprintf(str,64,"%d:%02d:%02d",cursec/60/60,(cursec/60)%60,cursec%60);
    }
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,str);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,str);
    y+=h;
    
    {
      snprintf(str,64,"%d%% 0:00:00",volper);
      w=pDstBM->GetTextWidthA(str);
      x=ScreenWidth-4-w;
    }
    if(ShowTime==false){
      snprintf(str,64,"%d%% 0:00:00",volper);
      }else{
      snprintf(str,64,"%d%% %d:%02d:%02d",volper,playsec/60/60,(playsec/60)%60,playsec%60);
    }
#ifdef UseShadow
    pDstBM->SetFontTextColor(collow);
    pDstBM->TextOutA(x+1,y+1,str);
#endif
    pDstBM->SetFontTextColor(colhigh);
    pDstBM->TextOutA(x+0,y+0,str);
    y+=h;
  }
  
#undef UseShadow
  return(true);
}

