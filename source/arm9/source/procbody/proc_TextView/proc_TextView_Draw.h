

extern "C" {
void VRAMWriteCache_Enable(void);
void VRAMWriteCache_Disable(void);
}

static void DrawCurrentText_BookMark_FillLine(s32 LineNum,CglCanvas *pcan,u32 y)
{
    if(LineNum<0) return;
  
    u32 x=2;
    u32 w=ScreenWidth-ScrollBarWidth-2-x;
    u32 h=ShowLineHeight-ShowLineMargin+1;
  
    for(u32 idx=0;idx<BookmarkItemCount;idx++){
        TBookmarkItem *pbmi=Bookmark_Load(idx);
        if(LineNum==(pbmi->LineNum-1)){
            const u16 col=ColorTable.TextView.Bookmark_FillBox[idx];
            if(col!=(RGB15(0,0,0)|BIT15)){
                pcan->SetColor(col);
                pcan->FillBox(x,y,w,h);
            }
        }
    }
}

static void TextView_MainDrawText(TScrollBar *psb)
{
    if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
  
  CglCanvas *pTmpBM=pScreenMain->pBackCanvas;
  
  pExtFont->Color=ColorTable.TextView.MainText;
    
  pDrawLineBM->SetColor(ColorTable.TextView.Line);
  
// ------------------------------
  TextView_GetSkin(ETV_PageBG)->pCanvas->BitBltFullBeta(pTmpBM);
  
  {
    CglCanvas *pBGBM=TextView_GetSkin(ETV_PageBG)->pCanvas;
      
    const u32 cnt=TextLinesCount;
    for(s32 idx=0;idx<cnt;idx++){
      s32 DrawHeight=psb->LineHeight;
      s32 DrawTop=(idx*DrawHeight)-psb->ShowPos+TopMargin;
     if((-psb->LineHeight<DrawTop)&&(DrawTop<psb->ClientSize+TopMargin)){
        if((psb->ClientSize+TopMargin)<(DrawTop+DrawHeight)) DrawHeight=psb->ClientSize+TopMargin-DrawTop;
        
        UnicodeChar strw[128+1];
        if(libconv_GetTextLine(idx,strw)==true){
            pBGBM->BitBlt(pDrawLineBM,0,0,ScreenWidth,DrawHeight,0,DrawTop,false);
            u32 x=LeftMargin;
            u32 y=0;
            DrawCurrentText_BookMark_FillLine(idx,pDrawLineBM,y);
            y++;
            ExtFont_TextOutW(pDrawLineBM,x,y,strw);
            
            y+=ShowLineHeight-ShowLineMargin;
            pDrawLineBM->DrawLine(2,y,ScreenWidth-ScrollBarWidth-2,y);
        
            pDrawLineBM->BitBlt(pTmpBM,0,DrawTop,ScreenWidth,DrawHeight,0,0,false);
        }
      }
    }
    
    pBGBM->BitBlt(pTmpBM,0,0,ScreenWidth,TopMargin,0,0,false);
    
    ScrollBar_Draw(psb,pTmpBM);
    
  }
  
// ------------------------------
  
  ScreenMain_Flip_ProcFadeEffect();
  
  if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

#include "proc_TextView_DrawID3Tag.h"
#include "proc_TextView_DrawLyric.h"

static void TextView_DrawInfo(CglCanvas *pDstBM)
{
    if(ShowID3TagTimeOut!=0 && TextView_DrawID3Tag(pDstBM)) return;
    
    u32 LinesCount=4;
  
    u32 x=8,y=8,h=glCanvasTextHeight+2;
  
    {
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
  
    pDstBM->SetFontTextColor(ColorTable.FileList.ID3TagText);
  
    char str[128];
  
    pDstBM->TextOutUTF8(x,y,Lang_GetUTF8("TV_Help1"));
    y+=h;
    pDstBM->TextOutUTF8(x,y,Lang_GetUTF8("TV_Help2"));
    y+=h;
    snprintf(str,128,Lang_GetUTF8("TV_Help3"),pEncodeID);
    pDstBM->TextOutUTF8(x,y,str);
    y+=h;
    snprintf(str,128,Lang_GetUTF8("TV_Info"),1+ScrollBar.CurrentLineIndex,TextLinesCount,(float)ScrollBar.CurrentLineIndex*100/TextLinesCount);
    pDstBM->TextOutUTF8(x,y,str);
    y+=h;
}

// -----------------------------

#include "proc_TextView_Clock.h"

static void TextView_SubDrawText(TScrollBar *psb)
{
    
    switch(ProcState.Text.TopScrMode){
        case ETTSM_LightOff: {
            pScreenSub->pCanvas->FillFull(RGB15(0,0,0)|BIT15);
            IPC6->LCDPowerControl=LCDPC_ON_BOTTOM;
            if(ProcState.Text.isSwapDisp) IPC6->LCDPowerControl=LCDPC_ON_TOP;
        } return;
        case ETTSM_Text: {
        } break;
        case ETTSM_Clock: {
            Clock_Standby_Draw(pSubTempBM);
            TextView_DrawInfo(pSubTempBM);
            //TextView_DrawLyric(pSubTempBM);
            pSubTempBM->BitBltFullBeta(pScreenSub->pCanvas);
            IPC6->LCDPowerControl=LCDPC_ON_BOTH;
            if(ProcState.Text.isSwapDisp && !ProcState.Text.BacklightFlag) IPC6->LCDPowerControl=LCDPC_ON_TOP;
        } return;
    }
    
    pSubTempBM->SetCglFont(pCglFontDefault);
  
    pDrawLineBM->SetCglFont(pCglFontDefault);
  
    pExtFont->Color=ColorTable.TextView.MainText;
    
    pDrawLineBM->SetColor(ColorTable.TextView.Line);
  
// ------------------------------
    TextView_GetSkin(ETV_PageBG)->pCanvas->BitBltFullBeta(pSubTempBM);

    {
        CglCanvas *pBGBM=TextView_GetSkin(ETV_PageBG)->pCanvas;
      
        const u32 cnt=TextLinesCount;
        for(s32 idx=0;idx<cnt;idx++){
            s32 DrawHeight=psb->LineHeight;
            s32 DrawTop=(idx*DrawHeight)+psb->ClientSize-psb->ShowPos+TopMargin;
            if((-psb->LineHeight<DrawTop)&&(DrawTop<psb->ClientSize+TopMargin)){
                if((psb->ClientSize+TopMargin)<(DrawTop+DrawHeight)) DrawHeight=psb->ClientSize+TopMargin-DrawTop;
            
                UnicodeChar strw[128+1];
                if(libconv_GetTextLine(idx,strw)==true){
                    pBGBM->BitBlt(pDrawLineBM,0,0,ScreenWidth,DrawHeight,0,DrawTop,false);
                    u32 x=LeftMargin;
                    u32 y=0;
                    DrawCurrentText_BookMark_FillLine(idx,pDrawLineBM,y);
                    y++;
                    ExtFont_TextOutW(pDrawLineBM,x,y,strw);
                
                    y+=ShowLineHeight-ShowLineMargin;
                    pDrawLineBM->DrawLine(2,y,ScreenWidth-ScrollBarWidth-2,y);
                    
                    pDrawLineBM->BitBlt(pSubTempBM,0,DrawTop,ScreenWidth,DrawHeight,0,0,false);
                }
            }
        }
    
        pBGBM->BitBlt(pSubTempBM,0,0,ScreenWidth,TopMargin+ShowLineHeight,0,0,false);
        
        {
            u32 x=LeftMargin;
            u32 y=TopMargin+1;
            char str[128];
            snprintf(str,128,Lang_GetUTF8("TV_Info"),1+ScrollBar.CurrentLineIndex,TextLinesCount,(float)ScrollBar.CurrentLineIndex*100/TextLinesCount);
            pSubTempBM->SetFontTextColor(ColorTable.TextView.MainText);
            pSubTempBM->TextOutUTF8(x,y,str);
        }
    }
  // ------------------------------
  
    if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Enable();
    pSubTempBM->BitBltFullBeta(pScreenSub->pCanvas);
    if(GlobalINI.System.VRAMCacheEnabled) VRAMWriteCache_Disable();
}

DATA_IN_IWRAM_TextView static bool mb;
DATA_IN_IWRAM_TextView static s32 msx,msy;
DATA_IN_IWRAM_TextView static s32 mx,my;
DATA_IN_IWRAM_TextView static s32 movespeed;

static bool TextView_MouseDown(TScrollBar *psb,s32 x,s32 y)
{
	if(x>(ScreenWidth-psb->DrawRect.w-psb->ClickMarginWidth)) return(false);
		
  mb=true;
  msx=x;
  msy=y;
  mx=x;
  my=y;
  movespeed=0;
  
  return(true);
}

static bool TextView_MouseMove(TScrollBar *psb,s32 x,s32 y)
{
  if(mb==false) return(false);
  
  if(y!=my){
    s32 vec=my-y;
    ScrollBar_SetDirectTopPos(psb,psb->TopPos+vec);
    
    if(movespeed<vec){
      movespeed=vec;
      }else{
      movespeed=(movespeed+(vec*15))/16;
    }
  }
  
  mx=x;
  my=y;
  
  return(true);
}

static bool TextView_MouseUp(TScrollBar *psb,s32 x,s32 y)
{
  if(mb==false) return(false);
  
  mb=false;
  
  if(16<movespeed) movespeed=16;
  
  if((-16<(x-msx))&&((x-msx)<16)){
    }else{
        if(VerboseDebugLog==true) _consolePrintf("TextView_MouseUp: It was judged a horizontal scroll. (%dpixels)\n",x-msx);
    //return(true);
  }
  
  if((-16<(y-msy))&&((y-msy)<16)){
    }else{
        if(VerboseDebugLog==true) _consolePrintf("TextView_MouseUp: It was judged a vertical scroll. (%dpixels)\n",y-msy);
    //return(true);
  }
  
  
  ScrollBar_SetCurrentLineIndex(psb,psb->CurrentLineIndex);
  return(true);
}

static void TextView_MouseIdle(TScrollBar *psb)
{
  if(mb==true){
    movespeed=(movespeed*7)/8;
    return;
  }
  
  if(movespeed!=0){
    ScrollBar_SetDirectTopPos(psb,psb->TopPos+movespeed);
    ScrollBar_SetCurrentLineIndex(psb,psb->CurrentLineIndex);
    movespeed=(movespeed*31)/32;
    ScreenRedrawFlag=true;
  }
}

