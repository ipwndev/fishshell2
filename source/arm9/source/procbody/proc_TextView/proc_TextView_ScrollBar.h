
enum EScrollBar_CurMode {ESBCM_Idle,ESBCM_Drag};

typedef struct {
  s32 LastMouseX,LastMouseY;
  s32 CurrentLineIndex;
  s32 LineHeight;
  s32 ClientSize;
  TRect DrawRect;
  s32 ClickMarginWidth;
  CglCanvas *pbm;
  s32 TopPos,ShowPos,MaxPos,LastPos;
  s32 GripHeight;
  s32 GripAlignY;
  EScrollBar_CurMode CurMode;
} TScrollBar;

DATA_IN_IWRAM_TextView static TScrollBar ScrollBar;

static void ScrollBar_Free(TScrollBar *psb)
{
  if(psb->pbm!=NULL){
    delete psb->pbm; psb->pbm=NULL;
  }
  psb->CurMode=ESBCM_Idle;
}

static void ScrollBar_Init(TScrollBar *psb,s32 _LineHeight)
{
  psb->LastMouseX=0;
  psb->LastMouseY=0;
  psb->CurrentLineIndex=0;
  psb->LineHeight=_LineHeight;
  psb->ClientSize=ShowLineCount*psb->LineHeight;
  psb->DrawRect=CreateRect(ScreenWidth-12,0,12,ScreenHeight);
  psb->ClickMarginWidth=0;
  psb->pbm=new CglCanvas(&MM_Process,NULL,psb->DrawRect.w,psb->DrawRect.h,pf15bit);
  psb->TopPos=0;
  psb->ShowPos=0;
  psb->MaxPos=0;
  psb->LastPos=0;
  psb->GripHeight=psb->DrawRect.h;
  psb->GripAlignY=0;
  psb->CurMode=ESBCM_Idle;
}

static void ScrollBar_MoveUp(TScrollBar *psb)
{
  if((psb->TopPos%psb->LineHeight)!=0){
    psb->TopPos=(psb->TopPos/psb->LineHeight)*psb->LineHeight;
    }else{
    psb->TopPos-=psb->LineHeight;
  }
  
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_MoveDown(TScrollBar *psb)
{
  psb->TopPos+=psb->ClientSize;
  if((psb->TopPos%psb->LineHeight)!=0){
    psb->TopPos=((psb->TopPos+(psb->LineHeight-1))/psb->LineHeight)*psb->LineHeight;
    }else{
    psb->TopPos+=psb->LineHeight;
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
  
  psb->TopPos=(GripPosY*psb->MaxPos)/psb->GripHeight;
  
  if((psb->MaxPos-psb->ClientSize)<psb->TopPos) psb->TopPos=psb->MaxPos-psb->ClientSize;
  if(psb->TopPos<0) psb->TopPos=0;
}

static void ScrollBar_SetDirectTopPos(TScrollBar *psb,s32 _TopPos)
{
  psb->TopPos=_TopPos;
    
  if((psb->MaxPos-psb->ClientSize)<psb->TopPos) psb->TopPos=psb->MaxPos-psb->ClientSize;
  if(psb->TopPos<0) psb->TopPos=0;
  
  s32 lineidx=psb->TopPos/psb->LineHeight;
    
  psb->CurrentLineIndex=lineidx;
}

static void ScrollBar_SetCurrentLineIndex(TScrollBar *psb,s32 lineidx)
{
    
  s32 linecnt=TextLinesCount-ShowLineCount;
  
  if(linecnt<=lineidx) lineidx=linecnt;
  if(lineidx<0) lineidx=0;
  
  psb->CurrentLineIndex=lineidx;
  
  s32 vec=(psb->CurrentLineIndex*psb->LineHeight)-psb->TopPos;
  
  if(vec<-psb->LineHeight){
      psb->TopPos=psb->CurrentLineIndex*psb->LineHeight;
  }else{
      psb->TopPos=psb->TopPos+vec;
  }
}

static bool ScrollBar_MouseDown(TScrollBar *psb,s32 x,s32 y)
{
  if(!EncodeConvertAllEnd) return(false);
    
  u32 Keys=(~REG_KEYINPUT)&0x3ff;
  
  if((ProcState.Text.LockScrollBar)&&((Keys&(KEY_L|KEY_R))==0)) return(false);
  
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
  
  psb->CurMode=ESBCM_Drag;
  
  switch(psb->CurMode){
    case ESBCM_Idle: break;
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
      
      if((y<bary)||((bary+barh)<y)){
        psb->GripAlignY=-(barh/2);
        }else{
        psb->GripAlignY=-(y-bary);
      }
      
      ScrollBar_MoveClientPos(psb,psb->GripAlignY+y);
    } break;
  }
  
  s32 lineidx=psb->TopPos/psb->LineHeight;
  ScrollBar_SetCurrentLineIndex(psb,lineidx);
    
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
    case ESBCM_Drag: {
      if(x<-psb->ClickMarginWidth){
        psb->TopPos=psb->LastPos;
        }else{
        ScrollBar_MoveClientPos(psb,psb->GripAlignY+y);
      }
    } break;
  }
  
  s32 lineidx=psb->TopPos/psb->LineHeight;
  ScrollBar_SetCurrentLineIndex(psb,lineidx);
  
  return(true);
}

static bool ScrollBar_MouseUp(TScrollBar *psb,s32 x,s32 y)
{
  if(psb->CurMode==ESBCM_Idle) return(false);
  
  psb->CurMode=ESBCM_Idle;
  
  ScreenRedrawFlag=true;
  
  return(true);
}

static void ScrollBar_MouseIdle(TScrollBar *psb)
{
  switch(psb->CurMode){
    case ESBCM_Idle: break;
    case ESBCM_Drag: break;
  }
  
  if(psb->TopPos!=psb->ShowPos){
    if(!ProcState.Text.UseSmoothScroll) {
        psb->ShowPos=psb->TopPos;
        ScreenRedrawFlag=true;
        return;
    }
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

static void ScrollBar_Draw(TScrollBar *psb,CglCanvas *ptagbm)
{
  s32 x,w;
  x=psb->DrawRect.x;
  w=psb->DrawRect.w;
  
  if(!EncodeConvertAllEnd) {
      u32 pos=FAT2_ftell(FileHandle),max=FileSize;
      s32 y=0;        
      s32 h=(pos*ScreenHeight)/max;
                
      if(ScreenHeight<h) y=ScreenHeight;
      
      ptagbm->SetColor(RGB15(8,30,8)|BIT15);
      ptagbm->DrawBox(x,y,w,h);
              
      x+=1; y+=1; w-=2; h-=2;
      ptagbm->SetColor(RGB15(20,30,20)|BIT15);
      ptagbm->FillBox(x,y,w,h);  
      return;
  }
  
  if(psb->MaxPos==0) return;//StopFatalError(17202,"ScrollBar_MouseIdle psb->MaxPos==0\n");
  
  s32 y,h;
  
  if(psb->MaxPos<=psb->ClientSize){
    y=0;
    h=psb->GripHeight;
    }else{
    y=(psb->ShowPos*psb->GripHeight)/psb->MaxPos;
    h=(psb->ClientSize*psb->GripHeight)/psb->MaxPos;
  }
  
  if(psb->GripHeight<y) y=psb->GripHeight;
  if(h<8) h=8;
  if(psb->GripHeight<h) h=psb->GripHeight;
  if(psb->GripHeight<(y+h)) y=psb->GripHeight-h;
  
  ptagbm->SetColor(ColorTable.TextView.ScrollBar_Frame);
  ptagbm->DrawBox(x,y,w,h);
      
  x+=1; y+=1; w-=2; h-=2;
  u16 *pbuf=ptagbm->GetScanLine(y);
  pbuf+=x;
  u32 bufsize=ptagbm->GetWidth();
    
  u16 BGColor=ColorTable.TextView.ScrollBar_Inside;
  for(s32 py=0;py<h;py++){
      for(s32 px=0;px<w;px++){
          u16 col=pbuf[px];
          u16 mask=RGB15(30,30,30);
          col=(col&mask)>>1;
          pbuf[px]=(BGColor+col)|BIT15;
      }
      pbuf+=bufsize;
  }
}

