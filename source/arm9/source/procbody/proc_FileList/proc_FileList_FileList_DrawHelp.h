
static void FileList_DrawBG_DrawHelp(CglCanvas *pDstBM)
{
  if((Process_DrawHelpLeft==false)&&(Process_DrawHelpRight==false)) return;
  
  bool ErrorMsg=false;
  u32 LinesCount=0;
  
  if(Process_DrawHelpLeft==true) LinesCount=atoi(Lang_GetUTF8("FL_HelpLeft_Count"));
  if(Process_DrawHelpRight==true) LinesCount=atoi(Lang_GetUTF8("FL_HelpRight_Count"));
  
  if(LinesCount==0){
    ErrorMsg=true;
    LinesCount=1;
  }
  
  if(12<LinesCount) LinesCount=12;
  
  const u32 h=glCanvasTextHeight+2;
  u32 x=8,y=(ScreenHeight-(h*LinesCount))/2;
  
  { // Draw frame.
    const u32 DstBMWidth=ScreenWidth; // pDstBM->GetWidth();
    
    u16 *pb=pDstBM->GetVRAMBuf();
    u32 size;
    
    size=1*DstBMWidth;
    pb+=(y-4)*size;
    
    size=2*DstBMWidth;
    pb=FileList_DrawID3Tag_asm_Fill50per(pb,size);
    
    size=(2+(h*LinesCount)+2)*DstBMWidth;
    pb=FileList_DrawID3Tag_asm_Fill25per(pb,size);
    
    size=2*DstBMWidth;
    pb=FileList_DrawID3Tag_asm_Fill50per(pb,size);
  }
  
  for(u32 idx=0;idx<LinesCount;idx++){
    const char *pstr=NULL;
    
    if(ErrorMsg==true){
      pstr="Not found language item. Old version?";
      }else{
      if(Process_DrawHelpLeft==true){
        switch(idx){
          case 0: pstr=Lang_GetUTF8("FL_HelpLeft_Line0"); break;
          case 1: pstr=Lang_GetUTF8("FL_HelpLeft_Line1"); break;
          case 2: pstr=Lang_GetUTF8("FL_HelpLeft_Line2"); break;
          case 3: pstr=Lang_GetUTF8("FL_HelpLeft_Line3"); break;
          case 4: pstr=Lang_GetUTF8("FL_HelpLeft_Line4"); break;
          case 5: pstr=Lang_GetUTF8("FL_HelpLeft_Line5"); break;
          case 6: pstr=Lang_GetUTF8("FL_HelpLeft_Line6"); break;
          case 7: pstr=Lang_GetUTF8("FL_HelpLeft_Line7"); break;
          case 8: pstr=Lang_GetUTF8("FL_HelpLeft_Line8"); break;
          case 9: pstr=Lang_GetUTF8("FL_HelpLeft_Line9"); break;
          case 10: pstr=Lang_GetUTF8("FL_HelpLeft_Line10"); break;
          case 11: pstr=Lang_GetUTF8("FL_HelpLeft_Line11"); break;
        }
      }
      if(Process_DrawHelpRight==true){
        switch(idx){
          case 0: pstr=Lang_GetUTF8("FL_HelpRight_Line0"); break;
          case 1: pstr=Lang_GetUTF8("FL_HelpRight_Line1"); break;
          case 2: pstr=Lang_GetUTF8("FL_HelpRight_Line2"); break;
          case 3: pstr=Lang_GetUTF8("FL_HelpRight_Line3"); break;
          case 4: pstr=Lang_GetUTF8("FL_HelpRight_Line4"); break;
          case 5: pstr=Lang_GetUTF8("FL_HelpRight_Line5"); break;
          case 6: pstr=Lang_GetUTF8("FL_HelpRight_Line6"); break;
          case 7: pstr=Lang_GetUTF8("FL_HelpRight_Line7"); break;
          case 8: pstr=Lang_GetUTF8("FL_HelpRight_Line8"); break;
          case 9: pstr=Lang_GetUTF8("FL_HelpRight_Line9"); break;
          case 10: pstr=Lang_GetUTF8("FL_HelpRight_Line10"); break;
          case 11: pstr=Lang_GetUTF8("FL_HelpRight_Line11"); break;
        }
      }
    }
    
    if(pstr!=NULL){
      pDstBM->SetFontTextColor(RGB15(0,0,0)|BIT15);
      pDstBM->TextOutUTF8(x+1,y+1+1,pstr);
      pDstBM->SetFontTextColor(RGB15(31,31,31)|BIT15);
      pDstBM->TextOutUTF8(x,y+1,pstr);
    }
    y+=h;
  }
}

