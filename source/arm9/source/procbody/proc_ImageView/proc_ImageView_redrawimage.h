void RedrawImage_ins_x50(u8 *pMCUBuf,u16 *ptmpbm0,u16 *ptmpbm1)
{
	asm volatile (
	"MCUSize = 32\n\t"
	"stmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,lr} \n\t"
	" \n\t"
	"x50_pMCUBuf .req r0 \n\t"
	"x50_ptmpbm0 .req r1 \n\t"
	"x50_ptmpbm1 .req r2 \n\t"
	"x50_Count .req r3 \n\t"
	"mov x50_Count,#((MCUSize/2)*(MCUSize/4))-1 \n\t"
	" \n\t"
	"x50_col .req r4 \n\t"
	"x50_cr0 .req r5 \n\t"
	"x50_cg0 .req r6 \n\t"
	"x50_cb0 .req r7 \n\t"
	"x50_cr1 .req r8 \n\t"
	"x50_cg1 .req r9 \n\t"
	"x50_cb1 .req r10 \n\t"
	" \n\t"
	"x50_tmp0 .req r11 \n\t"
	"x50_tmp1 .req r12 \n\t"
	" \n\t"
	"x50_RGB15Mask .req lr \n\t"
	"ldr x50_RGB15Mask,=(1<<15)|(1<<31) \n\t"
	" \n\t"
	"RedrawImage_ins_x50_loop: \n\t"
	" \n\t"
	"ldrb x50_cr0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*0)+0] \n\t"
	"ldrb x50_cg0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*0)+1] \n\t"
	"ldrb x50_cb0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*0)+2] \n\t"
	" \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*1)+0] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*0)+(3*1)+1] \n\t"
	"add x50_cr0,x50_tmp0 \n\t"
	"add x50_cg0,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*1)+2] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*0)+0] \n\t"
	"add x50_cb0,x50_tmp0 \n\t"
	"add x50_cr0,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*0)+1] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*0)+2] \n\t"
	"add x50_cg0,x50_tmp0 \n\t"
	"add x50_cb0,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*1)+0] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*1)+1] \n\t"
	"add x50_cr0,x50_tmp0 \n\t"
	"add x50_cg0,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*1)+2] \n\t"
	"add x50_cb0,x50_tmp0 \n\t"
	" \n\t"
	"ldrb x50_cr1,[x50_pMCUBuf,#(MCUSize*3*0)+(3*2)+0] \n\t"
	"ldrb x50_cg1,[x50_pMCUBuf,#(MCUSize*3*0)+(3*2)+1] \n\t"
	"ldrb x50_cb1,[x50_pMCUBuf,#(MCUSize*3*0)+(3*2)+2] \n\t"
	" \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*3)+0] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*0)+(3*3)+1] \n\t"
	"add x50_cr1,x50_tmp0 \n\t"
	"add x50_cg1,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*0)+(3*3)+2] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*2)+0] \n\t"
	"add x50_cb1,x50_tmp0 \n\t"
	"add x50_cr1,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*2)+1] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*2)+2] \n\t"
	"add x50_cg1,x50_tmp0 \n\t"
	"add x50_cb1,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*3)+0] \n\t"
	"ldrb x50_tmp1,[x50_pMCUBuf,#(MCUSize*3*1)+(3*3)+1] \n\t"
	"add x50_cr1,x50_tmp0 \n\t"
	"add x50_cg1,x50_tmp1 \n\t"
	"ldrb x50_tmp0,[x50_pMCUBuf,#(MCUSize*3*1)+(3*3)+2] \n\t"
	"add x50_cb1,x50_tmp0 \n\t"
	" \n\t"
	"add x50_pMCUBuf,#12 \n\t"
	" \n\t"
	"mov x50_col,x50_cr0,lsr #3 \n\t"
	"mov x50_tmp0,x50_cg0,lsr #3 \n\t"
	"orr x50_col,x50_tmp0,lsl #5 \n\t"
	"mov x50_tmp0,x50_cb0,lsr #3 \n\t"
	"orr x50_col,x50_tmp0,lsl #10 \n\t"
	" \n\t"
	"mov x50_tmp0,x50_cr1,lsr #3 \n\t"
	"orr x50_col,x50_tmp0,lsl #16+0 \n\t"
	"mov x50_tmp0,x50_cg1,lsr #3 \n\t"
	"orr x50_col,x50_tmp0,lsl #16+5 \n\t"
	"mov x50_tmp0,x50_cb1,lsr #3 \n\t"
	"orr x50_col,x50_tmp0,lsl #16+10 \n\t"
	" \n\t"
	"orr x50_col,x50_RGB15Mask \n\t"
	" \n\t"
	"str x50_col,[x50_ptmpbm0],#4 \n\t"
	" \n\t"
	"tsts x50_cr0,#4 \n\t"
	"addne x50_col,#1<<0 \n\t"
	"tsts x50_cg0,#4 \n\t"
	"addne x50_col,#1<<5 \n\t"
	"tsts x50_cb0,#4 \n\t"
	"addne x50_col,#1<<10 \n\t"
	" \n\t"
	"tsts x50_cr1,#4 \n\t"
	"addne x50_col,#1<<(16+0) \n\t"
	"tsts x50_cg1,#4 \n\t"
	"addne x50_col,#1<<(16+5) \n\t"
	"tsts x50_cb1,#4 \n\t"
	"addne x50_col,#1<<(16+10) \n\t"
	" \n\t"
	"str x50_col,[x50_ptmpbm1],#4 \n\t"
	" \n\t"
	"tsts x50_Count,#(MCUSize/4)-1 \n\t"
	"addeq x50_pMCUBuf,#MCUSize*3 \n\t"
	" \n\t"
	"subs x50_Count,#1 \n\t"
	"bge RedrawImage_ins_x50_loop \n\t"
	" \n\t"
	"ldmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc} \n\t"
	".ltorg \n\t"
	:::"memory"
	);
}

void RedrawImage_ins_x100(u8 *pMCUBuf,u16 *ptmpbm0,u16 *ptmpbm1,u32 Strict)
{
	asm volatile (
	"stmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,lr} \n\t"
	" \n\t"
	"x100_pMCUBuf .req r0 \n\t"
	"x100_ptmpbm0 .req r1 \n\t"
	"x100_ptmpbm1 .req r2 \n\t"
	"x100_Strict .req r3 \n\t"
	" \n\t"
	"x100_col .req r4 \n\t"
	"x100_cr0 .req r5 \n\t"
	"x100_cg0 .req r6 \n\t"
	"x100_cb0 .req r7 \n\t"
	"x100_cr1 .req r8 \n\t"
	"x100_cg1 .req r9 \n\t"
	"x100_cb1 .req r10 \n\t"
	" \n\t"
	"x100_tmp .req r11 \n\t"
	" \n\t"
	"x100_Count .req r12 \n\t"
	"mov x100_Count,#MCUSize*MCUSize \n\t"
	"sub x100_Count,#2 \n\t"
	" \n\t"
	"x100_RGB15Mask .req lr \n\t"
	"ldr x100_RGB15Mask,=(1<<15)|(1<<31) \n\t"
	" \n\t"
	"RedrawImage_ins_x100_loop: \n\t"
	" \n\t"
	"ldrb x100_cr0,[x100_pMCUBuf],#1 \n\t"
	"ldrb x100_cg0,[x100_pMCUBuf],#1 \n\t"
	"ldrb x100_cb0,[x100_pMCUBuf],#1 \n\t"
	"ldrb x100_cr1,[x100_pMCUBuf],#1 \n\t"
	"ldrb x100_cg1,[x100_pMCUBuf],#1 \n\t"
	"ldrb x100_cb1,[x100_pMCUBuf],#1 \n\t"
	" \n\t"
	"mov x100_col,x100_cr0,lsr #1 \n\t"
	"mov x100_tmp,x100_cg0,lsr #1 \n\t"
	"orr x100_col,x100_tmp,lsl #5 \n\t"
	"mov x100_tmp,x100_cb0,lsr #1 \n\t"
	"orr x100_col,x100_tmp,lsl #10 \n\t"
	" \n\t"
	"mov x100_tmp,x100_cr1,lsr #1 \n\t"
	"orr x100_col,x100_tmp,lsl #16+0 \n\t"
	"mov x100_tmp,x100_cg1,lsr #1 \n\t"
	"orr x100_col,x100_tmp,lsl #16+5 \n\t"
	"mov x100_tmp,x100_cb1,lsr #1 \n\t"
	"orr x100_col,x100_tmp,lsl #16+10 \n\t"
	" \n\t"
	"orr x100_col,x100_RGB15Mask \n\t"
	" \n\t"
	"str x100_col,[x100_ptmpbm0],#4 \n\t"
	" \n\t"
	"tsts x100_cr0,#1 \n\t"
	"addne x100_col,#1<<0 \n\t"
	"tsts x100_cg0,#1 \n\t"
	"addne x100_col,#1<<5 \n\t"
	"tsts x100_cb0,#1 \n\t"
	"addne x100_col,#1<<10 \n\t"
	" \n\t"
	"tsts x100_cr1,#1 \n\t"
	"addne x100_col,#1<<(16+0) \n\t"
	"tsts x100_cg1,#1 \n\t"
	"addne x100_col,#1<<(16+5) \n\t"
	"tsts x100_cb1,#1 \n\t"
	"addne x100_col,#1<<(16+10) \n\t"
	" \n\t"
	"str x100_col,[x100_ptmpbm1],#4 \n\t"
	" \n\t"
	"tsts x100_Count,#MCUSize-1 \n\t"
	"addeq x100_ptmpbm0,x100_Strict \n\t"
	"addeq x100_ptmpbm1,x100_Strict \n\t"
	" \n\t"
	"subs x100_Count,#2 \n\t"
	"bge RedrawImage_ins_x100_loop \n\t"
	" \n\t"
	"ldmfd sp!, {r4,r5,r6,r7,r8,r9,r10,r11,r12,pc} \n\t"
	".ltorg \n\t"
	:::"memory"
	);
}

static void RedrawImage_ins_x200(u8 *pMCUBuf,u16 *ptmpbm0,u16 *ptmpbm1)
{
  for(u32 y=0;y<MCUSize-1;y++){
    u32 r00,g00,b00,r10,g10,b10;
    r00=pMCUBuf[(MCUSize*3*0)+0];
    g00=pMCUBuf[(MCUSize*3*0)+1];
    b00=pMCUBuf[(MCUSize*3*0)+2];
    r10=pMCUBuf[(MCUSize*3*1)+0];
    g10=pMCUBuf[(MCUSize*3*1)+1];
    b10=pMCUBuf[(MCUSize*3*1)+2];
    for(u32 x=0;x<MCUSize;x++){
      u32 r01,g01,b01,r11,g11,b11;
      r01=pMCUBuf[(MCUSize*3*0)+0];
      g01=pMCUBuf[(MCUSize*3*0)+1];
      b01=pMCUBuf[(MCUSize*3*0)+2];
      r11=pMCUBuf[(MCUSize*3*1)+0];
      g11=pMCUBuf[(MCUSize*3*1)+1];
      b11=pMCUBuf[(MCUSize*3*1)+2];
      pMCUBuf+=3;
      
      u32 r,g,b;
      u32 col;
      
      r=r00; g=g00; b=b00;
      col=RGB15(r>>1,g>>1,b>>1)|BIT15;
      ptmpbm0[(MCUSize*2*0)+0]=col;
      if((r&1)!=0) col+=RGB15(1,0,0);
      if((g&1)!=0) col+=RGB15(0,1,0);
      if((b&1)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*0)+0]=col;
      
      r=r00+r01; g=g00+g01; b=b00+b01;
      col=RGB15(r>>2,g>>2,b>>2)|BIT15;
      ptmpbm0[(MCUSize*2*0)+1]=col;
      if((r&2)!=0) col+=RGB15(1,0,0);
      if((g&2)!=0) col+=RGB15(0,1,0);
      if((b&2)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*0)+1]=col;
      
      r=r00+r10; g=g00+g10; b=b00+b10;
      col=RGB15(r>>2,g>>2,b>>2)|BIT15;
      ptmpbm0[(MCUSize*2*1)+0]=col;
      if((r&2)!=0) col+=RGB15(1,0,0);
      if((g&2)!=0) col+=RGB15(0,1,0);
      if((b&2)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*1)+0]=col;
      
      r=r00+r01+r10+r11; g=g00+g01+g10+g11; b=b00+b01+b10+b11;
      col=RGB15(r>>3,g>>3,b>>3)|BIT15;
      ptmpbm0[(MCUSize*2*1)+1]=col;
      if((r&4)!=0) col+=RGB15(1,0,0);
      if((g&4)!=0) col+=RGB15(0,1,0);
      if((b&4)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*1)+1]=col;
      
      ptmpbm0+=2;
      ptmpbm1+=2;
      
      r00=r01;
      g00=g01;
      b00=b01;
      r10=r11;
      g10=g11;
      b10=b11;
    }
    ptmpbm0+=MCUSize*2;
    ptmpbm1+=MCUSize*2;
  }
  
  {
    u32 r00,g00,b00;
    r00=pMCUBuf[(MCUSize*3*0)+0];
    g00=pMCUBuf[(MCUSize*3*0)+1];
    b00=pMCUBuf[(MCUSize*3*0)+2];
    for(u32 x=0;x<MCUSize;x++){
      u32 r01,g01,b01;
      r01=pMCUBuf[(MCUSize*3*0)+0];
      g01=pMCUBuf[(MCUSize*3*0)+1];
      b01=pMCUBuf[(MCUSize*3*0)+2];
      pMCUBuf+=3;
      
      u32 r,g,b;
      u32 col;
      
      r=r00; g=g00; b=b00;
      col=RGB15(r>>1,g>>1,b>>1)|BIT15;
      ptmpbm0[(MCUSize*2*0)+0]=col;
      ptmpbm0[(MCUSize*2*1)+0]=col;
      if((r&1)!=0) col+=RGB15(1,0,0);
      if((g&1)!=0) col+=RGB15(0,1,0);
      if((b&1)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*0)+0]=col;
      ptmpbm1[(MCUSize*2*1)+0]=col;
      
      r=r00+r01; g=g00+g01; b=b00+b01;
      col=RGB15(r>>2,g>>2,b>>2)|BIT15;
      ptmpbm0[(MCUSize*2*0)+1]=col;
      ptmpbm0[(MCUSize*2*1)+1]=col;
      if((r&2)!=0) col+=RGB15(1,0,0);
      if((g&2)!=0) col+=RGB15(0,1,0);
      if((b&2)!=0) col+=RGB15(0,0,1);
      ptmpbm1[(MCUSize*2*0)+1]=col;
      ptmpbm1[(MCUSize*2*1)+1]=col;
      
      ptmpbm0+=2;
      ptmpbm1+=2;
      
      r00=r01;
      g00=g01;
      b00=b01;
    }
    ptmpbm0+=MCUSize*2;
    ptmpbm1+=MCUSize*2;
  }
}

static void RedrawImage(void)
{
  TRect dr=DstRect;
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  {
    u16 color=RGB15(16,16,16)|BIT15;
    
    u32 bx=dr.w*MultipleFix8/0x100;
    u32 by=dr.h*MultipleFix8/0x100;
    
    if(ScreenWidth<bx) bx=ScreenWidth;
    if(ScreenHeight<by) by=ScreenHeight;
    
    if(bx<ScreenWidth){
      for(u32 y=0;y<by;y++){
        u16 *pbm0=pScreenMain->pBackCanvas->GetScanLine(y);
        u16 *pbm1=pScreenMain->pViewCanvas->GetScanLine(y);
        for(u32 x=bx;x<ScreenWidth;x++){
          pbm0[x]=color;
          pbm1[x]=color;
        }
      }
    }
    for(u32 y=by;y<ScreenHeight;y++){
      u16 *pbm0=pScreenMain->pBackCanvas->GetScanLine(y);
      u16 *pbm1=pScreenMain->pViewCanvas->GetScanLine(y);
      for(u32 x=0;x<ScreenWidth;x++){
        pbm0[x]=color;
        pbm1[x]=color;
      }
    }
  }
  
  s32 xalign=0,yalign=0;
  
  if((dr.x&MCUSizeMask)!=0){
    xalign=dr.x&MCUSizeMask;
    dr.w+=xalign;
    dr.x-=xalign;
  }
  if((dr.y&MCUSizeMask)!=0){
    yalign=dr.y&MCUSizeMask;
    dr.h+=yalign;
    dr.y-=yalign;
  }
  
  u32 MCUShowSize=MCUSize*MultipleFix8/0x100;
  xalign=xalign*MultipleFix8/0x100;
  yalign=yalign*MultipleFix8/0x100;
  
  u32 xofs=dr.x/MCUSize;
  u32 yofs=dr.y/MCUSize;
  u32 xcnt=(dr.w+(MCUSize-1))/MCUSize;
  u32 ycnt=(dr.h+(MCUSize-1))/MCUSize;
  
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
  
  CglCanvas *pDrawTempBM0=new CglCanvas(&MM_Temp,pDrawTempMemory0,MCUShowSize,MCUShowSize,pf15bit);
  CglCanvas *pDrawTempBM1=new CglCanvas(&MM_Temp,pDrawTempMemory1,MCUShowSize,MCUShowSize,pf15bit);
  
  xalign=xalign&~1;
  
  if(MultipleFix8==(1*0x100)){
    const u32 Strict=(ScreenWidth-MCUSize)*2;
    s32 ypos=-yalign;
    for(u32 yidx=0;yidx<ycnt;yidx++){
      s32 xpos=-xalign;
      ReadMCUBlock(MCUSize*(yofs+yidx+1));
      if(RequestInterruptBreak==true) break;
      for(u32 xidx=0;xidx<xcnt;xidx++){
        u8 *pMCUBuf=MCU_GetMCU(xofs+xidx,yofs+yidx);
        
        bool DirectDraw=false;
        
        if(MultipleFix8==(1*0x100)){
          if((0<=xpos)&&(xpos<=(ScreenWidth-MCUSize))){
            if((0<=ypos)&&(ypos<=(ScreenHeight-MCUSize))){
              DirectDraw=true;
            }
          }
        }
        
        if(DirectDraw==false){
          u16 *_ptmpbm0=pDrawTempBM0->GetVRAMBuf();
          u16 *_ptmpbm1=pDrawTempBM1->GetVRAMBuf();
          RedrawImage_ins_x100(pMCUBuf,_ptmpbm0,_ptmpbm1,0);
          pDrawTempBM0->BitBlt(pScreenMain->pBackCanvas,xpos,ypos,MCUSize,MCUSize,0,0,false);
          pDrawTempBM1->BitBlt(pScreenMain->pViewCanvas,xpos,ypos,MCUSize,MCUSize,0,0,false);
          }else{
          u16 *_ptmpbm0=&pScreenMain->pBackCanvas->GetScanLine(ypos)[xpos];
          u16 *_ptmpbm1=&pScreenMain->pViewCanvas->GetScanLine(ypos)[xpos];
          RedrawImage_ins_x100(pMCUBuf,_ptmpbm0,_ptmpbm1,Strict);
        }
        
        xpos+=MCUSize;
      }
      ypos+=MCUSize;
    }
    }else{
    s32 ypos=-yalign;
    for(u32 yidx=0;yidx<ycnt;yidx++){
      s32 xpos=-xalign;
      ReadMCUBlock(MCUSize*(yofs+yidx+1));
      if(RequestInterruptBreak==true) break;
      for(u32 xidx=0;xidx<xcnt;xidx++){
        u8 *pMCUBuf=MCU_GetMCU(xofs+xidx,yofs+yidx);
        u16 *_ptmpbm0=pDrawTempBM0->GetVRAMBuf();
        u16 *_ptmpbm1=pDrawTempBM1->GetVRAMBuf();
        
        switch(MultipleFix8){
          case (u32)(0.5*0x100): RedrawImage_ins_x50(pMCUBuf,_ptmpbm0,_ptmpbm1); break;
          case 1*0x100: break;
          case 2*0x100: RedrawImage_ins_x200(pMCUBuf,_ptmpbm0,_ptmpbm1); break;
        }
        
        pDrawTempBM0->BitBlt(pScreenMain->pBackCanvas,xpos,ypos,MCUShowSize,MCUShowSize,0,0,false);
        pDrawTempBM1->BitBlt(pScreenMain->pViewCanvas,xpos,ypos,MCUShowSize,MCUShowSize,0,0,false);
        
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
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}
