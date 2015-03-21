
#pragma Ospace

#include <nds.h>

#include "_const.h"
#include "_console.h"

#include "maindef.h"
#include "memtool.h"
#include "shell.h"
#include "lang.h"

#include "component.h"

#include "skin.h"
#include "strtool.h"

// ---------------------

DATA_IN_AfterSystem static UnicodeChar ucs[1]={0};
DATA_IN_AfterSystem static CglFont *pDefaultFont;
DATA_IN_AfterSystem static bool isBalloonShow=false;

void Component_SetFont(CglFont *pFont)
{
  pDefaultFont=pFont;
}

void Component_BalloonShow(const char *pMsgUTF8,const UnicodeChar *pMsgUnicode,s32 mx,s32 my)
{
	/*CglCanvas *pcan=pScreenMainOverlay->pCanvas;
	pcan->SetColor(RGB15(0,0,0)|BIT(15));
	pcan->MoveTo(mx,my);
	pcan->LineTo(mx-6,my+15);
	pcan->MoveTo(mx,my);
	pcan->LineTo(mx+6,my+15);
	pcan->SetColor(RGB15(31,31,31)|BIT(15));
	for(s32 sx=mx-5;sx<mx+5;sx++){
		pcan->MoveTo(mx,my);
		pcan->LineTo(sx,my+15);
	}*/
	isBalloonShow=true;
}
// ---------------------

void ComponentLabel_Init(TComponentLabel *pComponentLabel,CglCanvas *pCanvas)
{
  TComponentLabel *pcl=pComponentLabel;
  
  pcl->CallBack_Click=NULL;
  pcl->pCanvas=pCanvas;
  pcl->Rect=CreateRect(0,0,0,0);
  pcl->Visible=true;
  pcl->Center=false;
  pcl->isTitle=false;
  pcl->TextColor=0;
  pcl->pMsgUTF8="";
  pcl->pMsgUnicode=ucs;
  pcl->pInfoUTF8="";
  pcl->pInfoUnicode=ucs;
}

void ComponentLabel_Draw(TComponentLabel *pComponentLabel)
{
  TComponentLabel *pcl=pComponentLabel;
  
  if(pcl->Visible==false) return;
  
  pcl->pCanvas->SetCglFont(pDefaultFont);
  if(pcl->TextColor!=0){
    pcl->pCanvas->SetFontTextColor(pcl->TextColor);
    }else{
    if(pcl->isTitle==false){
      pcl->pCanvas->SetFontTextColor(ColorTable.Component.Label_Text);
      }else{
      pcl->pCanvas->SetFontTextColor(ColorTable.Component.TitleLabel_Text);
    }
  }
  
  const char *pmsgUTF8=pcl->pMsgUTF8;
  const UnicodeChar *pmsgUnicode=pcl->pMsgUnicode;
  
  u32 TextWidth=(pmsgUTF8[0]!=0) ? pcl->pCanvas->GetTextWidthUTF8(pmsgUTF8) : pcl->pCanvas->GetTextWidthW(pmsgUnicode);
  u32 TextHeight=glCanvasTextHeight;
  
  if(pcl->Rect.w==0) pcl->Rect.w=TextWidth;
  if(pcl->Rect.h==0) pcl->Rect.h=TextHeight;
  
  TRect r=pcl->Rect;
  
//  pcl->pCanvas->SetColor(pcl->BGColor);
//  pcl->pCanvas->FillBox(r.x,r.y,r.w,r.h);
  
  if(pmsgUTF8[0]!=0){
    if(pcl->Center==false){
      pcl->pCanvas->TextOutUTF8(r.x,r.y,pmsgUTF8);
      }else{
      pcl->pCanvas->TextOutUTF8(r.x+((r.w-TextWidth)/2),r.y,pmsgUTF8);
    }
  }else if(pmsgUnicode[0]!=0){
	  if(pcl->Center==false){
		  pcl->pCanvas->TextOutW(r.x,r.y,pmsgUnicode);
	  }else{
		  pcl->pCanvas->TextOutW(r.x+((r.w-TextWidth)/2),r.y,pmsgUnicode);
	  }
  }
  
  const char *pinfoUTF8=pcl->pInfoUTF8;
  const UnicodeChar *pinfoUnicode=pcl->pInfoUnicode;
  if(pinfoUTF8[0]!=0 || pinfoUnicode[0]!=0) {
	  TRect r=pcl->Rect;
	  CglTGF *pInfoTGF=ComponentAlpha_GetSkin(ECSA_Info);
	  s32 w=pInfoTGF->GetWidth();
	  s32 h=pInfoTGF->GetHeight();
	  s32 x=r.x+(TextWidth-w/2-1);
	  s32 y=r.y-w/2+1;
	  
	  if(x<0) x=0;
	  if(y<0) y=0;
	  if(x+w>pcl->pCanvas->GetWidth()) x=pcl->pCanvas->GetWidth()-w-1;
	  if(y+h>pcl->pCanvas->GetHeight()) x=pcl->pCanvas->GetHeight()-h-1;
	  pInfoTGF->BitBlt(pcl->pCanvas,x,y);
  }
}

s32 ComponentLabel_GetWidth(TComponentLabel *pComponentLabel)
{
  TComponentLabel *pcl=pComponentLabel;
  
  if(pcl->Visible==false) return(0);
  
  pcl->pCanvas->SetCglFont(pDefaultFont);
  pcl->pCanvas->SetFontTextColor(ColorTable.Component.Label_Text);
  
  const char *pmsgUTF8=pcl->pMsgUTF8;
  const UnicodeChar *pmsgUnicode=pcl->pMsgUnicode;
  
  u32 TextWidth=(pmsgUTF8[0]!=0) ? pcl->pCanvas->GetTextWidthUTF8(pmsgUTF8) : pcl->pCanvas->GetTextWidthW(pmsgUnicode);

  if(pcl->Rect.w==0) pcl->Rect.w=TextWidth;
  
  TRect r=pcl->Rect;
  
  s32 w=0;
  
  if(pmsgUTF8[0]!=0){
    if(pcl->Center==false){
      w=pcl->pCanvas->GetTextWidthUTF8(pmsgUTF8);
      }else{
      w=r.w;
    }
  }else if(pmsgUnicode[0]!=0){
	  if(pcl->Center==false){
		  w=pcl->pCanvas->GetTextWidthW(pmsgUnicode);
	  }else{
		  w=r.w;
	  }
  }
  
  return(w+2);
}

s32 ComponentLabel_GetIndexFromPos(TComponentLabel *pComponentLabel,s32 mx,s32 my)
{
  TComponentLabel *pcl=pComponentLabel;
  TRect r=pcl->Rect;
  
  if(pcl->Visible==false) return(-1);
  
  mx-=r.x;
  my-=r.y;
  
  if((0<=mx)&&(mx<r.w)){
    if((0<=my)&&(my<r.h)){
      return(0);
    }
  }
  
  return(-1);
}

bool ComponentLabel_MouseUp(TComponentLabel *pComponentLabel,s32 mx,s32 my)
{
  TComponentLabel *pcl=pComponentLabel;
  
  if(pcl->Visible==false) return(false);
  
  if(ComponentLabel_GetIndexFromPos(pcl,mx,my)==-1) return(false);
  
  if(pcl->CallBack_Click!=NULL) pcl->CallBack_Click(pcl);
  
  return(true);
}

// ------------------

void ComponentCheck_Init(TComponentCheck *pComponentCheck,CglCanvas *pCanvas)
{
  TComponentCheck *pcc=pComponentCheck;
  
  pcc->CallBack_Click=NULL;
  pcc->UserData=0;
  pcc->pCanvas=pCanvas;
  pcc->pOnIcon=NULL;
  pcc->pOffIcon=NULL;
  pcc->Checked=false;
  pcc->Rect=CreateRect(0,0,0,0);
  pcc->Visible=true;
  pcc->pMsgUTF8="";
  pcc->pMsgUnicode=ucs;
  pcc->pInfoUTF8="";
  pcc->pInfoUnicode=ucs;
  pcc->TextColor=ColorTable.Component.Check_Text;
}

void ComponentCheck_Draw(TComponentCheck *pComponentCheck)
{
  TComponentCheck *pcc=pComponentCheck;
  
  if(pcc->Visible==false) return;
  
  if(pcc->pOnIcon!=NULL){
    if(pcc->Rect.w==0) pcc->Rect.w=pcc->pOnIcon->GetWidth();
    if(pcc->Rect.h==0) pcc->Rect.h=pcc->pOnIcon->GetHeight();
  }
  
  TRect r=pcc->Rect;
  
  CglTGF *pSrcTGF=NULL;
  if(pcc->Checked==true){
    pSrcTGF=pcc->pOnIcon;
    }else{
    pSrcTGF=pcc->pOffIcon;
  }
  if(pSrcTGF!=NULL){
    s32 x=r.x;
    s32 y=r.y;
    s32 w=pSrcTGF->GetWidth();
    s32 h=pSrcTGF->GetHeight();
    x+=(r.w-w+0)/2;
    y+=(r.h-h+1)/2;
    pSrcTGF->BitBlt(pcc->pCanvas,x,y);
  }
  
  pcc->pCanvas->SetCglFont(pDefaultFont);
  pcc->pCanvas->SetFontTextColor(pcc->TextColor);
  
  const char *pmsgUTF8=pcc->pMsgUTF8;
  const UnicodeChar *pmsgUnicode=pcc->pMsgUnicode;
  const char *pinfoUTF8=pcc->pInfoUTF8;
  const UnicodeChar *pinfoUnicode=pcc->pInfoUnicode;
  
  u32 TextX=r.x+r.w+2;
  u32 TextY=r.y;
  u32 TextWidth=(pmsgUTF8[0]!=0) ? pcc->pCanvas->GetTextWidthUTF8(pmsgUTF8) : pcc->pCanvas->GetTextWidthW(pmsgUnicode);
  u32 TextHeight=glCanvasTextHeight;
  TextY+=(r.h-TextHeight+1)/2;
  
//  pcc->pCanvas->SetColor(RGB15(0,0,0)|BIT15);
//  pcc->pCanvas->FillBox(TextX,TextY,64,TextHeight);
  
  if(pmsgUTF8[0]!=0){
	  pcc->pCanvas->TextOutUTF8(TextX,TextY,pmsgUTF8);
  }else if(pmsgUnicode[0]!=0){
	  pcc->pCanvas->TextOutW(TextX,TextY,pmsgUnicode);
  }else if(pinfoUTF8[0]!=0){
	  pcc->pCanvas->TextOutUTF8(TextX,TextY,pinfoUTF8);
  }else if(pinfoUnicode[0]!=0){
	  pcc->pCanvas->TextOutW(TextX,TextY,pinfoUnicode);
  }
  
  if(pinfoUTF8[0]!=0 || pinfoUnicode[0]!=0) {
  	  CglTGF *pInfoTGF=ComponentAlpha_GetSkin(ECSA_Info);
  	  s32 w=pInfoTGF->GetWidth();
  	  s32 h=pInfoTGF->GetHeight();
  	  s32 x=TextX+(TextWidth-w/2-1);
  	  s32 y=TextY-w/2+1;
  	  
  	  if(x<0) x=0;
  	  if(y<0) y=0;
  	  if(x+w>pcc->pCanvas->GetWidth()) x=pcc->pCanvas->GetWidth()-w-1;
  	  if(y+h>pcc->pCanvas->GetHeight()) x=pcc->pCanvas->GetHeight()-h-1;
  	  pInfoTGF->BitBlt(pcc->pCanvas,x,y);
  }
}

s32 ComponentCheck_GetWidth(TComponentCheck *pComponentCheck)
{
  TComponentCheck *pcc=pComponentCheck;
  
  if(pcc->Visible==false) return(0);
  
  if(pcc->pOnIcon!=NULL){
    if(pcc->Rect.w==0) pcc->Rect.w=pcc->pOnIcon->GetWidth();
  }
  
  TRect r=pcc->Rect;
  
  pcc->pCanvas->SetCglFont(pDefaultFont);
  
  const char *pmsgUTF8=pcc->pMsgUTF8;
  const UnicodeChar *pmsgUnicode=pcc->pMsgUnicode;

  if(pmsgUTF8[0]!=0){
	  return(r.w+2+pcc->pCanvas->GetTextWidthUTF8(pmsgUTF8)+2);
  }else if(pmsgUnicode[0]!=0){
	  return(r.w+2+pcc->pCanvas->GetTextWidthW(pmsgUnicode)+2);
  }else{
	  return(r.w+2+2);
  }
}

static bool ComponentCheck_GetIndexFromPos(TComponentCheck *pComponentCheck,s32 mx,s32 my)
{
  TComponentCheck *pcc=pComponentCheck;
  TRect r=pcc->Rect;
  
  if(pcc->Visible==false) return(false);
  
  if(pcc->Rect.w==0 && pcc->Rect.h==0) return(false);
  
  pcc->pCanvas->SetCglFont(pDefaultFont);
  const char *pmsgUTF8=pcc->pMsgUTF8; 
  const UnicodeChar *pmsgUnicode=pcc->pMsgUnicode;
  const char *pinfoUTF8=pcc->pInfoUTF8;
  const UnicodeChar *pinfoUnicode=pcc->pInfoUnicode;
  
  s32 TextWidth=0;
  if(str_isEmpty(pcc->pMsgUTF8)==true && pmsgUnicode[0]==0){
    TextWidth=ScreenWidth;
    }else{
    	if(pmsgUTF8[0]!=0){
    		TextWidth=r.w+2+pcc->pCanvas->GetTextWidthUTF8(pmsgUTF8);
    	}else if(pmsgUnicode[0]!=0){
    		TextWidth=r.w+2+pcc->pCanvas->GetTextWidthW(pmsgUnicode);
    	}
  }
  
  mx-=r.x;
  my-=r.y;
  
  if(pinfoUTF8[0]!=0 || pinfoUnicode[0]!=0) {
	  CglTGF *pInfoTGF=ComponentAlpha_GetSkin(ECSA_Info);
	  s32 w=pInfoTGF->GetWidth();
	  s32 h=pInfoTGF->GetHeight();
	  TextWidth+=w/2+1;
  }
  
  if((0<=mx)&&(mx<TextWidth)){
    if((0<=my)&&(my<r.h)){
      return(true);
    }
  }
  
  return(false);
}

bool ComponentCheck_MouseDown(TComponentCheck *pComponentCheck,s32 mx,s32 my)
{
  TComponentCheck *pcc=pComponentCheck;
  TRect r=pcc->Rect;
  
  if(pcc->Visible==false) return(false);
  
  if(ComponentCheck_GetIndexFromPos(pcc,mx,my)==false) return(false);
  
  pcc->pCanvas->SetCglFont(pDefaultFont);
  const char *pmsgUTF8=pcc->pMsgUTF8; 
  const UnicodeChar *pmsgUnicode=pcc->pMsgUnicode;
  const char *pinfoUTF8=pcc->pInfoUTF8;
  const UnicodeChar *pinfoUnicode=pcc->pInfoUnicode;
  
  if(pinfoUTF8[0]==0 && pinfoUnicode[0]==0) return(false);
	  
  s32 TextWidth=0;
  if(str_isEmpty(pcc->pMsgUTF8)==true && pmsgUnicode[0]==0){
	  TextWidth=ScreenWidth;
  }else{
	  if(pmsgUTF8[0]!=0){
		  TextWidth=r.w+2+pcc->pCanvas->GetTextWidthUTF8(pmsgUTF8);
	  }else if(pmsgUnicode[0]!=0){
		  TextWidth=r.w+2+pcc->pCanvas->GetTextWidthW(pmsgUnicode);
	  }
  }
  
  CglTGF *pInfoTGF=ComponentAlpha_GetSkin(ECSA_Info);
  s32 w=pInfoTGF->GetWidth();
  s32 h=pInfoTGF->GetHeight();
    
  if((mx-r.x)<(TextWidth-w/2-1) || (my-r.y)>(r.h-h/2+1)) return(false);
  
  Component_BalloonShow(pinfoUTF8,pinfoUnicode,mx,my);

  s32 x0=4,y0=10;
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  pcan->SetFontTextColor(RGB15(31,31,31)|BIT15);
  pcan->TextOutW(x0+1,y0+1,pinfoUnicode);
    
  pcan->SetFontTextColor(RGB15(0,0,0)|BIT15);
  pcan->TextOutW(x0,y0,pinfoUnicode);
  
  return(true);
}

bool ComponentCheck_MouseUp(TComponentCheck *pComponentCheck,s32 mx,s32 my)
{
	TComponentCheck *pcc=pComponentCheck;
	TRect r=pcc->Rect;
  
	if(isBalloonShow){
		pScreenMainOverlay->pCanvas->SetColor(0);
		pScreenMainOverlay->pCanvas->FillFull(0);
		isBalloonShow=false;
	}
	
	if(pcc->Visible==false) return(false);
  
	if(ComponentCheck_GetIndexFromPos(pcc,mx,my)==false) return(false);
  
	pcc->pCanvas->SetCglFont(pDefaultFont);
	const char *pmsgUTF8=pcc->pMsgUTF8; 
	const UnicodeChar *pmsgUnicode=pcc->pMsgUnicode;
	const char *pinfoUTF8=pcc->pInfoUTF8;
	const UnicodeChar *pinfoUnicode=pcc->pInfoUnicode;
	  
	if(pinfoUTF8[0]!=0 || pinfoUnicode[0]!=0) {
		s32 TextWidth=0;
		if(str_isEmpty(pcc->pMsgUTF8)==true && pmsgUnicode[0]==0){
			TextWidth=ScreenWidth;
		}else{
			if(pmsgUTF8[0]!=0){
				TextWidth=r.w+2+pcc->pCanvas->GetTextWidthUTF8(pmsgUTF8);
			}else if(pmsgUnicode[0]!=0){
				TextWidth=r.w+2+pcc->pCanvas->GetTextWidthW(pmsgUnicode);
			}
		}
		   
		CglTGF *pInfoTGF=ComponentAlpha_GetSkin(ECSA_Info);
		s32 w=pInfoTGF->GetWidth();
		s32 h=pInfoTGF->GetHeight();
		  
		mx-=r.x;
		my-=r.y;
		    
		if(mx>TextWidth-w/2-1 && my<r.h-h/2+1){
			//pScreenMainOverlay->pCanvas->SetColor(0);
			//pScreenMainOverlay->pCanvas->FillFull(0);
			return(false);
		}
	}
	
	if(pcc->CallBack_Click!=NULL) pcc->CallBack_Click(pcc);
  
	return(true);
}

// ------------------

void ComponentButton_Init(TComponentButton *pComponentButton,CglCanvas *pCanvas)
{
  TComponentButton *pcb=pComponentButton;
  
  pcb->CallBack_Click=NULL;
  pcb->pCanvas=pCanvas;
  pcb->pIcon=NULL;
  pcb->DrawFrame=true;
  pcb->Pressing=false;
  pcb->Rect=CreateRect(0,0,0,0);
  pcb->pMsgUTF8="";
  pcb->pMsgUnicode=ucs;
  pcb->Visible=true;
  pcb->NormalTextColor=ColorTable.Component.Button_NormalText;
  pcb->PressTextColor=ColorTable.Component.Button_PressText;
}

void ComponentButton_Draw(TComponentButton *pComponentButton)
{
  TComponentButton *pcb=pComponentButton;
  
  if(pcb->Visible==false) return;
  
  if(pcb->DrawFrame==true){
    if(pcb->Pressing==false){
      TRect r=pcb->Rect;
      pcb->pCanvas->SetColor(ColorTable.Component.Button_NormalHighlight);
      pcb->pCanvas->DrawBox(r.x-1,r.y-1,r.w+2,r.h+2);
      pcb->pCanvas->SetColor(ColorTable.Component.Button_NormalShadow);
      pcb->pCanvas->DrawBox(r.x,r.y,r.w+1,r.h+1);
      pcb->pCanvas->SetColor(ColorTable.Component.Button_NormalBG);
      pcb->pCanvas->FillBox(r.x,r.y,r.w,r.h);
      }else{
      TRect r=pcb->Rect;
      pcb->pCanvas->SetColor(ColorTable.Component.Button_PressHighlight);
      pcb->pCanvas->DrawBox(r.x-1,r.y-1,r.w+2,r.h+2);
      pcb->pCanvas->SetColor(ColorTable.Component.Button_PressShadow);
      pcb->pCanvas->DrawBox(r.x,r.y,r.w+1,r.h+1);
      pcb->pCanvas->SetColor(ColorTable.Component.Button_PressBG);
      pcb->pCanvas->FillBox(r.x,r.y,r.w,r.h);
    }
  }
  
  pcb->pCanvas->SetCglFont(pDefaultFont);
  if(pcb->Pressing==false){
    pcb->pCanvas->SetFontTextColor(pcb->NormalTextColor);
    }else{
    pcb->pCanvas->SetFontTextColor(pcb->PressTextColor);
  }
  
  const char *pmsgUTF8=pcb->pMsgUTF8;
  const UnicodeChar *pmsgUnicode=pcb->pMsgUnicode;

  TRect r=pcb->Rect;
  
  s32 IconWidth;
  s32 IconHeight;
  if(pcb->pIcon==NULL){
    IconWidth=-8;
    IconHeight=0;
    }else{
    IconWidth=pcb->pIcon->GetWidth();
    IconHeight=pcb->pIcon->GetHeight();
  }
  
   
  u32 TextWidth=(pmsgUTF8[0]!=0) ? pcb->pCanvas->GetTextWidthUTF8(pmsgUTF8) : pcb->pCanvas->GetTextWidthW(pmsgUnicode);
  s32 TextHeight=glCanvasTextHeight;
  
  s32 BodyWidth=IconWidth;
  if(TextWidth!=0) BodyWidth+=8+TextWidth;
  s32 BodyHeight=IconHeight;
  if(IconHeight<TextHeight) BodyHeight=TextHeight;
  s32 BodyX=r.x+((r.w-BodyWidth)/2);
  s32 BodyY=r.y+((r.h-BodyHeight)/2);
  
  if(pcb->pIcon!=NULL){
    s32 x=BodyX;
    s32 y=BodyY+((BodyHeight-IconHeight)/2);
    pcb->pIcon->BitBlt(pcb->pCanvas,x,y);
  }
  
  if(pmsgUTF8[0]!=0){
	  pcb->pCanvas->TextOutUTF8(BodyX+8+IconWidth,BodyY+((BodyHeight-TextHeight)/2),pmsgUTF8);
  }else if(pmsgUnicode[0]!=0){
	  pcb->pCanvas->TextOutW(BodyX+8+IconWidth,BodyY+((BodyHeight-TextHeight)/2),pmsgUnicode);
  }
}

s32 ComponentButton_GetIndexFromPos(TComponentButton *pComponentButton,s32 mx,s32 my)
{
  TComponentButton *pcb=pComponentButton;
  TRect r=pcb->Rect;
  
  if(pcb->Visible==false) return(-1);
  
  mx-=r.x;
  my-=r.y;
  
  if((0<=mx)&&(mx<r.w)){
    if((0<=my)&&(my<r.h)){
      return(0);
    }
  }
  
  return(-1);
}

bool ComponentButton_MouseUp(TComponentButton *pComponentButton,s32 mx,s32 my)
{
  TComponentButton *pcb=pComponentButton;
  
  if(pcb->Visible==false) return(false);
  
  if(ComponentButton_GetIndexFromPos(pcb,mx,my)==-1) return(false);
  
  if(pcb->CallBack_Click!=NULL) pcb->CallBack_Click(pcb);
  
  return(true);
}

