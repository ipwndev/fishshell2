
enum EScrollBar_CurMode {ESBCM_Idle,ESBCM_PressUpButton,ESBCM_PressDownButton,ESBCM_Drag};

typedef struct {
  s32 LastMouseX,LastMouseY;
  s32 LastSelectedIndex;
  s32 SelectedIndex;
  s32 SelectXOfs;
  s32 SlideWidth;
  s32 ItemHeight;
  s32 ClientSize;
  TRect DrawRect;
  s32 ClickMarginWidth;
  CglCanvas *pbm;
  s32 TopPos,ShowPos,MaxPos,LastPos;
  s32 ButtonWidth,ButtonHeight;
  s32 ButtonPressVSyncCount;
  s32 GripTop,GripHeight;
  s32 GripAlignY;
  bool UsePressImage;
  EScrollBar_CurMode CurMode;
} TScrollBar;

DATA_IN_IWRAM_FileList static TScrollBar ScrollBar;

static void ScrollBar_Free(TScrollBar *psb)
{
  if(psb->pbm!=NULL){
    delete psb->pbm; psb->pbm=NULL;
  }
  psb->CurMode=ESBCM_Idle;
}

static void ScrollBar_Init(TScrollBar *psb,s32 _ItemHeight,s32 _SlideWidth)
{
  psb->LastMouseX=0;
  psb->LastMouseY=0;
  psb->LastSelectedIndex=0;
  psb->SlideWidth=_SlideWidth;
  psb->SelectedIndex=0;
  psb->SelectXOfs=0;
  psb->ItemHeight=_ItemHeight;
  psb->ClientSize=ScreenHeight;
  psb->DrawRect=CreateRect(ScreenWidth-14,0,14,psb->ClientSize);
  psb->ClickMarginWidth=24;
  psb->pbm=new CglCanvas(&MM_Process,NULL,psb->DrawRect.w,psb->DrawRect.h,pf15bit);
  psb->TopPos=0;
  psb->ShowPos=0;
  psb->MaxPos=0;
  psb->LastPos=0;
  psb->ButtonWidth=14;
  psb->ButtonHeight=14;
  psb->GripTop=psb->ButtonHeight;
  psb->GripHeight=psb->DrawRect.h-(psb->GripTop*2);
  psb->GripAlignY=0;
  psb->CurMode=ESBCM_Idle;
  psb->UsePressImage=false;
}

static void ScrollBar_MoveUp(TScrollBar *psb)
{
  if((psb->TopPos%psb->ItemHeight)!=0){
    psb->TopPos=(psb->TopPos/psb->ItemHeight)*psb->ItemHeight;
    }else{
    psb->TopPos-=psb->ItemHeight;
  }
  
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_MoveDown(TScrollBar *psb)
{
  psb->TopPos+=psb->ClientSize;
  if((psb->TopPos%psb->ItemHeight)!=0){
    psb->TopPos=((psb->TopPos+(psb->ItemHeight-1))/psb->ItemHeight)*psb->ItemHeight;
    }else{
    psb->TopPos+=psb->ItemHeight;
  }
  psb->TopPos-=psb->ClientSize;
  
  if((psb->MaxPos-psb->ClientSize)<psb->TopPos) psb->TopPos=psb->MaxPos-psb->ClientSize;
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_MoveClientPos(TScrollBar *psb,s32 GripPosY)
{
  if(psb->MaxPos<=psb->ClientSize){
    psb->TopPos=0;
    return;
  }
  
  GripPosY-=psb->ButtonHeight;
  
  psb->TopPos=(GripPosY*psb->MaxPos)/psb->GripHeight;
  
  if((psb->MaxPos-psb->ClientSize)<psb->TopPos) psb->TopPos=psb->MaxPos-psb->ClientSize;
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_SetDirectTopPos(TScrollBar *psb,s32 _TopPos)
{
  psb->TopPos=_TopPos;
  
  if((psb->MaxPos-psb->ClientSize)<psb->TopPos) psb->TopPos=psb->MaxPos-psb->ClientSize;
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_SetSelectedIndex(TScrollBar *psb,s32 fileidx)
{
  s32 filecnt=psb->MaxPos/psb->ItemHeight;
  
  if(filecnt<=fileidx) fileidx=filecnt-1;
  if(fileidx<0) fileidx=0;
  
  psb->LastSelectedIndex=psb->SelectedIndex;
  psb->SelectedIndex=fileidx;
  psb->SelectXOfs=0;
  
  ReloadIPK(ProcState.FileList.CurrentPathUnicode,psb->SelectedIndex);
  
  ReloadDPGThumb(ProcState.FileList.CurrentPathUnicode,psb->SelectedIndex);
  
  s32 vec=(psb->SelectedIndex*psb->ItemHeight)-psb->TopPos;
  
  if(vec<-psb->ItemHeight){
    psb->TopPos=psb->SelectedIndex*psb->ItemHeight;
    }else{
    if(vec<0) psb->TopPos=psb->TopPos+vec;
  }
  
  if(psb->ClientSize<vec){
    psb->TopPos=(psb->SelectedIndex*psb->ItemHeight)-psb->ClientSize+psb->ItemHeight;
    }else{
    if((psb->ClientSize-psb->ItemHeight)<vec){
      psb->TopPos=psb->TopPos+(vec-(psb->ClientSize-psb->ItemHeight));
    }
  }
}

static bool ScrollBar_MouseDown(TScrollBar *psb,s32 x,s32 y)
{
  if(psb->CurMode!=ESBCM_Idle) StopFatalError(17201,"ScrollBar_MouseDown psb->CurMode(%d)!=ESBCM_Idle\n",psb->CurMode);
  
  s32 x1,y1,x2,y2;
  x1=psb->DrawRect.x;
  y1=psb->DrawRect.y;
  x2=x1+psb->DrawRect.w;
  y2=y1+psb->DrawRect.h;
  
  x1-=psb->ClickMarginWidth;
  if((x1<=x)&&(x<x2)&&(y1<=y)&&(y<y2)){
    }else{
    return(false);
  }
  x1+=psb->ClickMarginWidth;
  
  x-=x1;
  y-=y1;
  
  s32 bh=psb->ButtonHeight+2;
  
  if(y<bh){
    psb->CurMode=ESBCM_PressUpButton;
    }else{
    if((psb->DrawRect.h-bh)<=y){
      psb->CurMode=ESBCM_PressDownButton;
      }else{
      psb->CurMode=ESBCM_Drag;
    }
  }
  
  switch(psb->CurMode){
    case ESBCM_Idle: break;
    case ESBCM_PressUpButton: {
      ScrollBar_MoveUp(psb);
      psb->ButtonPressVSyncCount=0;
    } break;
    case ESBCM_PressDownButton: {
      ScrollBar_MoveDown(psb);
      psb->ButtonPressVSyncCount=0;
    } break;
    case ESBCM_Drag: {
      psb->LastPos=psb->TopPos;
      
      s32 bary,barh;
      
      if(psb->MaxPos<=psb->ClientSize){
        bary=0;
        barh=psb->GripHeight;
        }else{
        bary=(psb->TopPos*psb->GripHeight)/psb->MaxPos;
        barh=(psb->ClientSize*psb->GripHeight)/psb->MaxPos;
      }
      
      if(psb->GripHeight<bary) bary=psb->GripHeight;
      if(barh<8) barh=8;
      if(psb->GripHeight<barh) barh=psb->GripHeight;
      if(psb->GripHeight<(bary+barh)) bary=psb->GripHeight-barh;
      
      y-=psb->ButtonHeight;
      if((y<bary)||((bary+barh)<y)){
        psb->GripAlignY=-(barh/2);
        }else{
        psb->GripAlignY=-(y-bary);
      }
      y+=psb->ButtonHeight;
      
      ScrollBar_MoveClientPos(psb,psb->GripAlignY+y);
    } break;
  }
  
  return(true);
}

static bool ScrollBar_MouseMove(TScrollBar *psb,s32 x,s32 y)
{
  if(psb->CurMode==ESBCM_Idle) return(false);
  
  s32 x1=psb->DrawRect.x;
  s32 y1=psb->DrawRect.y;
  x-=x1;
  y-=y1;
  
  switch(psb->CurMode){
    case ESBCM_Idle: break;
    case ESBCM_PressUpButton: break;
    case ESBCM_PressDownButton: break;
    case ESBCM_Drag: {
      if(x<-psb->ClickMarginWidth){
        psb->TopPos=psb->LastPos;
        }else{
        ScrollBar_MoveClientPos(psb,psb->GripAlignY+y);
      }
    } break;
  }
  
  return(true);
}

static bool ScrollBar_MouseUp(TScrollBar *psb,s32 x,s32 y)
{
  if(psb->CurMode==ESBCM_Idle) return(false);
  
  psb->CurMode=ESBCM_Idle;
  
  ScreenRedrawFlag=true;
  
  return(true);
}

static void ScrollBar_MouseIdle(TScrollBar *psb,u32 VsyncCount)
{
  switch(psb->CurMode){
    case ESBCM_Idle: break;
    case ESBCM_PressUpButton: {
      psb->ButtonPressVSyncCount++;
      if(psb->ButtonPressVSyncCount<15){
        }else{
        if((psb->ButtonPressVSyncCount%5)==0){
          ScrollBar_MoveUp(psb);
          ScreenRedrawFlag=true;
        }
      }
    } break;
    case ESBCM_PressDownButton: {
      psb->ButtonPressVSyncCount++;
      if(psb->ButtonPressVSyncCount<15){
        }else{
        if((psb->ButtonPressVSyncCount%5)==0){
          ScrollBar_MoveDown(psb);
          ScreenRedrawFlag=true;
        }
      }
    } break;
    case ESBCM_Drag: break;
  }
  
  if(psb->TopPos!=psb->ShowPos){
    s32 vec=psb->TopPos-psb->ShowPos;
    if(vec<0){
      vec/=8;
      if(vec==0) vec=-1;
      }else{
      vec/=8;
      if(vec==0) vec=1;
    }
    psb->ShowPos+=vec;
    ScreenRedrawFlag=true;
  }
}

void ScrollBar_Draw_asm_DrawBG_Fixed75per(u16 *pbuf)
{
	asm volatile (
	"stmfd sp!,{r4,r5,r6,r7,lr} \n\t"
	" \n\t"
	"mov r1,#192 \n\t"
	"ldr r2,=0x7BDE7BDE @ RGB15(30,30,30)<<16 | RGB15(30,30,30) \n\t"
	"ldr r3,=0x739C739C @ RGB15(28,28,28)<<16 | RGB15(28,28,28) \n\t"
	"@  ldr r4,=0x18A718A7 @ RGB15(7,5,6)<<16 | RGB15(7,5,6) \n\t"
	"@  ldr r4,=0x18C618C6 @ RGB15(6,6,6)<<16 | RGB15(6,6,6) \n\t"
	"ldr r4,=0x1CE71CE7 @ RGB15(7,7,7)<<16 | RGB15(7,7,7) \n\t"
	"ldr r5,=(1<<15)|(1<<31) \n\t"
	" \n\t"
	"ScrollBar_Draw_asm_DrawBG_Fixed75per_loop: \n\t"
	" \n\t"
	"add r0,#(256-14)*2 \n\t"
	" \n\t"
	"ldmia r0,{r6,r7} \n\t"
	"and r6,r6,r2 \n\t"
	"orr lr,r5,r6,lsr #1 \n\t"
	"and r6,r6,r3 \n\t"
	"add r6,lr,r6,lsr #2 \n\t"
	"add r6,r6,r4 \n\t"
	"and r7,r7,r2 \n\t"
	"orr lr,r5,r7,lsr #1 \n\t"
	"and r7,r7,r3 \n\t"
	"add r7,lr,r7,lsr #2 \n\t"
	"add r7,r7,r4 \n\t"
	"stmia r0!,{r6,r7} \n\t"
	" \n\t"
	"ldmia r0,{r6,r7} \n\t"
	"and r6,r6,r2 \n\t"
	"orr lr,r5,r6,lsr #1 \n\t"
	"and r6,r6,r3 \n\t"
	"add r6,lr,r6,lsr #2 \n\t"
	"add r6,r6,r4 \n\t"
	"and r7,r7,r2 \n\t"
	"orr lr,r5,r7,lsr #1 \n\t"
	"and r7,r7,r3 \n\t"
	"add r7,lr,r7,lsr #2 \n\t"
	"add r7,r7,r4 \n\t"
	"stmia r0!,{r6,r7} \n\t"
	" \n\t"
	"ldmia r0,{r6,r7} \n\t"
	"and r6,r6,r2 \n\t"
	"orr lr,r5,r6,lsr #1 \n\t"
	"and r6,r6,r3 \n\t"
	"add r6,lr,r6,lsr #2 \n\t"
	"add r6,r6,r4 \n\t"
	"and r7,r7,r2 \n\t"
	"orr lr,r5,r7,lsr #1 \n\t"
	"and r7,r7,r3 \n\t"
	"add r7,lr,r7,lsr #2 \n\t"
	"add r7,r7,r4 \n\t"
	"stmia r0!,{r6,r7} \n\t"
	" \n\t"
	"ldr r6,[r0] \n\t"
	"and r6,r6,r2 \n\t"
	"orr lr,r5,r6,lsr #1 \n\t"
	"and r6,r6,r3 \n\t"
	"add r6,lr,r6,lsr #2 \n\t"
	"add r6,r6,r4 \n\t"
	"str r6,[r0],#4 \n\t"
	" \n\t"
	"subs r1,#1 \n\t"
	"bne ScrollBar_Draw_asm_DrawBG_Fixed75per_loop \n\t"
	" \n\t"
	"ldmfd sp!, {r4,r5,r6,r7,pc} \n\t"
	".ltorg \n\t"
	:::"memory"
	);
}

static void ScrollBar_Draw(TScrollBar *psb,CglCanvas *ptagbm)
{
  s32 dx,dy,dh;
  dx=psb->DrawRect.x;
  dy=psb->DrawRect.y;
  dh=psb->DrawRect.h;
  
//  DrawSkinAlpha(ScrollBarAlpha_GetSkin(ESBSA_BG),ptagbm,dx,dy);
  ScrollBar_Draw_asm_DrawBG_Fixed75per(ptagbm->GetVRAMBuf());
  
  CglTGF *pUpBtn;
  CglTGF *pDownBtn;
  
  if(psb->UsePressImage==false){
    pUpBtn=ScrollBarAlpha_GetSkin(ESBSA_UpBtn_Normal);
    pDownBtn=ScrollBarAlpha_GetSkin(ESBSA_DownBtn_Normal);
    switch(psb->CurMode){
      case ESBCM_Idle: break;
      case ESBCM_PressUpButton: {
        pUpBtn=ScrollBarAlpha_GetSkin(ESBSA_UpBtn_Press);
      } break;
      case ESBCM_PressDownButton: {
        pDownBtn=ScrollBarAlpha_GetSkin(ESBSA_DownBtn_Press);
      } break;
      case ESBCM_Drag: break;
    }
    }else{
    pUpBtn=ScrollBarAlpha_GetSkin(ESBSA_UpBtn_Press);
    pDownBtn=ScrollBarAlpha_GetSkin(ESBSA_DownBtn_Press);
  }
  
  DrawSkinAlpha(pUpBtn,ptagbm,dx,dy);
  DrawSkinAlpha(pDownBtn,ptagbm,dx,dy+dh-psb->ButtonHeight);
  
  if(psb->MaxPos==0) StopFatalError(17202,"ScrollBar_MouseIdle psb->MaxPos==0\n");
  
  CglTGF *pGripBG,*pGripTop,*pGripBottom;
  
  if((psb->CurMode!=ESBCM_Drag)&&(psb->UsePressImage==false)){
    pGripBG=ScrollBarAlpha_GetSkin(ESBSA_GripBG_Normal);
    pGripTop=ScrollBarAlpha_GetSkin(ESBSA_GripTop_Normal);
    pGripBottom=ScrollBarAlpha_GetSkin(ESBSA_GripBottom_Normal);
    }else{
    pGripBG=ScrollBarAlpha_GetSkin(ESBSA_GripBG_Press);
    pGripTop=ScrollBarAlpha_GetSkin(ESBSA_GripTop_Press);
    pGripBottom=ScrollBarAlpha_GetSkin(ESBSA_GripBottom_Press);
  }
  
  s32 bary,barh;
  
  if(psb->MaxPos<=psb->ClientSize){
    bary=0;
    barh=psb->GripHeight;
    }else{
    bary=(psb->ShowPos*psb->GripHeight)/psb->MaxPos;
    barh=(psb->ClientSize*psb->GripHeight)/psb->MaxPos;
  }
  
  if(psb->GripHeight<bary) bary=psb->GripHeight;
  if(barh<8) barh=8;
  if(psb->GripHeight<barh) barh=psb->GripHeight;
  if(psb->GripHeight<(bary+barh)) bary=psb->GripHeight-barh;
  
  bary+=psb->GripTop;
  
  DrawSkinAlpha(pGripTop,ptagbm,dx,bary);
  bary+=pGripTop->GetHeight();
  barh-=pGripTop->GetHeight();
  
  DrawSkinAlpha(pGripBottom,ptagbm,dx,bary+barh-pGripBottom->GetHeight());
  barh-=pGripBottom->GetHeight();
  
  s32 h=pGripBG->GetHeight();
  for(s32 y=0;y<(barh/h);y++){
    DrawSkinAlpha(pGripBG,ptagbm,dx,bary+(h*y));
  }
}

