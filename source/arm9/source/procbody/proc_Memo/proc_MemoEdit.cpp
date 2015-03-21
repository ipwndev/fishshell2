
#include <nds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "_console.h"
#include "_consoleWriteLog.h"
#include "maindef.h"
#include "memtool.h"
#include "_const.h"
#include "ErrorDialog.h"
#include "../../ipc6.h"

#include "procstate.h"
#include "datetime.h"

#include "glib.h"

#include "fat2.h"
#include "shell.h"
#include "strtool.h"

#include "skin.h"
#include "unicode.h"
#include "sndeff.h"
#include "lang.h"
#include "rect.h"
#include "playlist.h"
#include "msgwin.h"

#include "OverlayManager.h"

// -----------------------------------------------------

DATA_IN_IWRAM_MemoEdit static bool isNewFile;
DATA_IN_IWRAM_MemoEdit static bool isEditedFile;

DATA_IN_IWRAM_MemoEdit static bool isEraseMode;
DATA_IN_IWRAM_MemoEdit static bool isRevocationMode;

DATA_IN_IWRAM_MemoEdit static bool FirstBGBottomDrawed;

typedef struct {
  s32 x,y;
} TCurLine;

#define CurLinesMaxCount (256)
DATA_IN_IWRAM_MemoEdit static u32 CurLinesCount;
DATA_IN_IWRAM_MemoEdit static TCurLine *pCurLines;

DATA_IN_IWRAM_MemoEdit static bool *pLineDblImage;
DATA_IN_IWRAM_MemoEdit static bool *pMaskDblImage;

DATA_IN_IWRAM_MemoEdit static u32 *pCurrentImage;

DATA_IN_IWRAM_MemoEdit static u32 *pRevocationImage;

static void MemInit(void)
{
  CurLinesCount=0;
  pCurLines=(TCurLine*)safemalloc_chkmem(&MM_Process,sizeof(TCurLine)*CurLinesMaxCount);
  
  pLineDblImage=(bool*)safemalloc_chkmem(&MM_Process,ScreenWidth*2*ScreenHeight*2*1);
  pMaskDblImage=(bool*)safemalloc_chkmem(&MM_Process,ScreenWidth*2*ScreenHeight*2*1);
  
  pCurrentImage=(u32*)safemalloc_chkmem(&MM_Process,ScreenWidth*ScreenHeight*4);
  MemSet32CPU(0,pCurrentImage,ScreenWidth*ScreenHeight*4);
  
  pRevocationImage=(u32*)safemalloc_chkmem(&MM_Process,ScreenWidth*ScreenHeight*4);
  MemSet32CPU(0,pRevocationImage,ScreenWidth*ScreenHeight*4);
}

static void MemFree(void)
{
  if(pCurLines!=NULL){
    safefree(&MM_Process,pCurLines); pCurLines=NULL;
  }
  CurLinesCount=0;
  
  if(pLineDblImage!=NULL){
    safefree(&MM_Process,pLineDblImage); pLineDblImage=NULL;
  }
  if(pMaskDblImage!=NULL){
    safefree(&MM_Process,pMaskDblImage); pMaskDblImage=NULL;
  }
  
  if(pCurrentImage!=NULL){
    safefree(&MM_Process,pCurrentImage); pCurrentImage=NULL;
  }
  
  if(pRevocationImage!=NULL){
      safefree(&MM_Process,pRevocationImage); pRevocationImage=NULL;
  }
}

// -----------------------------------------------------

static UnicodeChar* GetNewFilenameUnicode(void)
{
  DateTime_ResetNow();
  TDateTime dt=DateTime_GetNow();
  
  char fn[MaxFilenameLength];
  snprintf(fn,MaxFilenameLength,"FishMemo_%04d-%02d-%02d_%02d-%02d-%02d.bmp",dt.Date.Year,dt.Date.Month,dt.Date.Day,dt.Time.Hour,dt.Time.Min,dt.Time.Sec);
  
  UnicodeChar fnw[MaxFilenameLength];
  StrConvert_Ank2Unicode(fn,fnw);

  return(Unicode_AllocateCopy(&MM_Temp,fnw));
}

// -----------------------------------------------------

#include "BMPReader.h"

static bool CurrentImageLoadFromFile(const UnicodeChar *pFilePath,const UnicodeChar *pFileName)
{
  const char *palias=ConvertFull_Unicode2Alias(pFilePath,pFileName);
  FAT_FILE *pf=FAT2_fopen_AliasForRead(palias);
  if(pf==NULL) StopFatalError(0,"Not found BMP file. [%s]",palias);
  
  if(BMPReader_Start(pf)==false){
    BMPReader_Free();
    return(false);
  }
  
  s32 width=BMPReader_GetWidth();
  s32 height=BMPReader_GetHeight();
  if((width<0)||(ScreenWidth<width)||(height<0)||(ScreenHeight<height)){
    _consolePrint("This BMP file is large size.\n");
    BMPReader_Free();
    return(false);
  }
  
  s32 ofsx=(ScreenWidth-width)/2;
  s32 ofsy=(ScreenHeight-height)/2;
  
  u32 *pdstbuf=pCurrentImage;
  pdstbuf+=ScreenWidth*ofsy;
  pdstbuf+=ofsx;
  
  for(u32 y=0;y<height;y++){
    BMPReader_GetBitmap32(y,pdstbuf);
    for(u32 x=0;x<width;x++){
      pdstbuf[x]=0x00ffffff-pdstbuf[x];
    }
    pdstbuf+=ScreenWidth;
  }
  
  BMPReader_Free();
  
  FAT2_fclose(pf);
  
  return(true);
}

#include "proc_MemoEdit_BMPWriter.h"

static bool CurrentImageSaveToFile(void)
{
  msgwin_Draw(Lang_GetUTF8("MemoEdit_SavingNow"),"",0,0);
  
  s32 ofsx,ofsy;
  s32 width,height;
  
  {
    u32 *pcurbuf=pCurrentImage;
    s32 minx=ScreenWidth-1,miny=ScreenHeight-1,maxx=0,maxy=0;
    for(u32 y=0;y<ScreenHeight;y++){
      for(u32 x=0;x<ScreenWidth;x++){
        u32 col32=*pcurbuf++;
        if(col32!=0){
          if(x<minx) minx=x;
          if(maxx<x) maxx=x;
          if(y<miny) miny=y;
          if(maxy<y) maxy=y;
        }
      }
    }
    
    minx-=2;
    if(minx<0) minx=0;
    miny-=2;
    if(miny<0) miny=0;
    maxx+=2+1;
    if(ScreenWidth<maxx) maxx=ScreenWidth;
    maxy+=2+1;
    if(ScreenHeight<maxy) maxy=ScreenHeight;
    
    ofsx=minx;
    ofsy=miny;
    width=maxx-minx;
    height=maxy-miny;
  }
  
  if((width<0)||(ScreenWidth<width)||(height<0)||(ScreenHeight<height)) return(false);
  
  if((isNewFile==true)||(Unicode_isEmpty(RelationalFileNameUnicode)==true)){
    UnicodeChar *pfnw=GetNewFilenameUnicode();
    Unicode_Copy(RelationalFileNameUnicode,pfnw);
    if(pfnw!=NULL){
      safefree(&MM_Temp,pfnw); pfnw=NULL;
    }
  }
  
  const UnicodeChar *pFilePath=RelationalFilePathUnicode;
  const UnicodeChar *pFileName=RelationalFileNameUnicode;
  
  if(FileExistsUnicode(pFilePath,pFileName)==false){
    if(Shell_CreateNewFileUnicode(pFilePath,pFileName)==NULL) StopFatalError(16805,"Can not create new file.\n");
  }
  
  const char *palias=ConvertFull_Unicode2Alias(pFilePath,pFileName);
  FAT_FILE *pf=FAT2_fopen_AliasForWrite(palias);
  if(pf==NULL) StopFatalError(0,"Can not open BMP file for write. [%s]",palias);
  
  BMPWriter_Create(pf,width,height);
  
  u32 *psrcbuf=pCurrentImage;
  psrcbuf+=ScreenWidth*ofsy;
  psrcbuf+=ofsx;
  
  for(s32 y=height-1;y>=0;y--){
    u32 *psrcbm=&psrcbuf[ScreenWidth*y];
    for(u32 x=0;x<width;x++){
      psrcbm[x]=0x00ffffff-psrcbm[x];
    }
    BMPWriter_Bitmap1Line(pf,width,psrcbm);
  }
  
  FAT2_fclose(pf);
  
  msgwin_Clear();
  
  return(true);
}
  
static void CurrentImageToScreen(void)
{
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  u16 *pcanbuf=pcan->GetVRAMBuf();
  
  u32 *pcurbuf=pCurrentImage;
  
  s32 minx=ScreenWidth-1,miny=ScreenHeight-1,maxx=0,maxy=0;
  
  for(u32 y=0;y<ScreenHeight;y++){
    for(u32 x=0;x<ScreenWidth;x++){
      u32 col32=*pcurbuf++;
      if(col32==0){
        *pcanbuf++=RGB15(31,31,31)|BIT15;
        }else{
        u32 b=(col32>>(0+3))&0x1f;
        u32 g=(col32>>(8+3))&0x1f;
        u32 r=(col32>>(16+3))&0x1f;
        *pcanbuf++=(0x7fff-RGB15(r,g,b))|BIT15;
        if(x<minx) minx=x;
        if(maxx<x) maxx=x;
        if(y<miny) miny=y;
        if(maxy<y) maxy=y;
      }
    }
  }
  
  minx-=2;
  if(minx<0) minx=0;
  miny-=2;
  if(miny<0) miny=0;
  maxx+=2+1;
  if(ScreenWidth<maxx) maxx=ScreenWidth;
  maxy+=2+1;
  if(ScreenHeight<maxy) maxy=ScreenHeight;
  
  s32 w=maxx-minx,h=maxy-miny;
  if((0<w)&&(0<h)){
    pcan->SetColor(RGB15(24,24,24)|BIT15);
    pcan->DrawBox(minx,miny,w,h);
  }
}
// -----------------------------------------------------

static __attribute__ ((noinline)) void DrawCurLines_ins_DrawLine(bool *pbuf,const int x1,const int y1,const int x2,const int y2)
{
  if((x1==x2)&&(y1==y2)) return;
  
  const u32 w=ScreenWidth*2;
#define SetPixel(x,y) pbuf[(x)+(y)*w]=true;
  
  if(x1==x2){
    int ys,ye;
    if(y1<y2){
      ys=y1;
      ye=y2-1;
      }else{
      ys=y2+1;
      ye=y1;
    }
    for(int py=ys;py<=ye;py++){
      SetPixel(x1,py);
    }
    return;
  }
  
  if(y1==y2){
    int xs,xe;
    if(x1<x2){
      xs=x1;
      xe=x2-1;
      }else{
      xs=x2+1;
      xe=x1;
    }
    for(int px=xs;px<=xe;px++){
      SetPixel(px,y1);
    }
    return;
  }
  
  if(abs(x2-x1)>abs(y2-y1)){
    int px=0;
    float py=0;
    int xe=x2-x1;
    float ye=y2-y1;
    int xv;
    float yv;
    
    if(0<xe){
      xv=1;
      }else{
      xv=-1;
    }
    yv=ye/abs(xe);
    
    while(px!=xe){
      SetPixel(x1+px,y1+(int)py);
      px+=xv;
      py+=yv;
    }
    return;
    
    }else{
    float px=0;
    int py=0;
    float xe=x2-x1;
    int ye=y2-y1;
    float xv;
    int yv;
    
    xv=xe/abs(ye);
    if(0<ye){
      yv=1;
      }else{
      yv=-1;
    }
    
    while(py!=ye){
      SetPixel(x1+(int)px,y1+py);
      px+=xv;
      py+=yv;
    }
    return;
  }

#undef SetPixel
}

static void DrawCurLines(void)
{
  MemCopy32CPU(pCurrentImage,pRevocationImage,ScreenWidth*ScreenHeight*4);
  MemSet32CPU(0,pLineDblImage,ScreenWidth*2*ScreenHeight*2*1);
  
  switch(CurLinesCount){
    case 0: break;
    case 1: {
      TCurLine *pcl=&pCurLines[0];
      DrawCurLines_ins_DrawLine(pLineDblImage,pcl->x*2,pcl->y*2,pcl->x*2+2,pcl->y*2);
    } break;
    case 2: {
      TCurLine *pcl0=&pCurLines[0];
      TCurLine *pcl1=&pCurLines[1];
      DrawCurLines_ins_DrawLine(pLineDblImage,pcl0->x*2,pcl0->y*2,pcl1->x*2,pcl1->y*2);
    } break;
    default: {
      for(u32 idx=0;idx<CurLinesCount-1;idx++){
        TCurLine *pcl0=&pCurLines[idx+0];
        TCurLine *pcl1=&pCurLines[idx+1];
        DrawCurLines_ins_DrawLine(pLineDblImage,pcl0->x*2,pcl0->y*2,pcl1->x*2,pcl1->y*2);
      }
    } break;
  }
  
  CurLinesCount=0;
  
  MemSet32CPU(0,pMaskDblImage,ScreenWidth*2*ScreenHeight*2*1);
  
  if(ProcState.MemoEdit.TickLine==false && isEraseMode==false){
    const u32 bmsize=2;
    const bool bmmap[bmsize*2*bmsize*2]={false, true , true , false,
                                         true , true , true , true ,
                                         true , true , true , true ,
                                         false, true , true , false};
    const bool *psrc=pLineDblImage;
    bool *pdst=pMaskDblImage;
    psrc+=bmsize*ScreenWidth*2;
    pdst+=bmsize*ScreenWidth*2;
    for(u32 y=bmsize;y<ScreenHeight*2-bmsize;y++){
      for(u32 x=bmsize;x<ScreenWidth*2-bmsize;x++){
        if(psrc[x]==true){
          bool *pdstalign=&pdst[x];
          pdstalign-=bmsize*ScreenWidth*2;
          pdstalign-=bmsize;
          const bool *pbmmap=bmmap;
          for(u32 py=0;py<bmsize*2;py++){
            for(u32 px=0;px<bmsize*2;px++){
              if(pbmmap[px]==true) pdstalign[px]=true;
            }
            pdstalign+=ScreenWidth*2;
            pbmmap+=bmsize*2;
          }
        }
      }
      psrc+=ScreenWidth*2;
      pdst+=ScreenWidth*2;
    }
    }else{
    const u32 bmsize=4;
    const bool bmmap[bmsize*2*bmsize*2]={false, false, true , true , true , true , false, false,
                                         false, true , true , true , true , true , true , false,
                                         true , true , true , true , true , true , true , true ,
                                         true , true , true , true , true , true , true , true ,
                                         true , true , true , true , true , true , true , true ,
                                         true , true , true , true , true , true , true , true ,
                                         false, true , true , true , true , true , true , false,
                                         false, false, true , true , true , true , false, false};
    const bool *psrc=pLineDblImage;
    bool *pdst=pMaskDblImage;
    psrc+=bmsize*ScreenWidth*2;
    pdst+=bmsize*ScreenWidth*2;
    for(u32 y=bmsize;y<ScreenHeight*2-bmsize;y++){
      for(u32 x=bmsize;x<ScreenWidth*2-bmsize;x++){
        if(psrc[x]==true){
          bool *pdstalign=&pdst[x];
          pdstalign-=bmsize*ScreenWidth*2;
          pdstalign-=bmsize;
          const bool *pbmmap=bmmap;
          for(u32 py=0;py<bmsize*2;py++){
            for(u32 px=0;px<bmsize*2;px++){
              if(pbmmap[px]==true) pdstalign[px]=true;
            }
            pdstalign+=ScreenWidth*2;
            pbmmap+=bmsize*2;
          }
        }
      }
      psrc+=ScreenWidth*2;
      pdst+=ScreenWidth*2;
    }
  }
  
  {
    u32 bradd=0;
    bool maskpr=false,maskpg=false,maskpb=false;
    bool maskmr=false,maskmg=false,maskmb=false;
    
    switch(ProcState.MemoEdit.PenColor){
      case EMEPC_Black: {
        if(ProcState.MemoEdit.TickLine==false){
          bradd=0x40;
          }else{
          bradd=0x30;
        }
        maskpr=true; maskpg=true; maskpb=true;
      } break;
      case EMEPC_Blue:  bradd=0x40; maskpr=true; maskpg=true; maskmb=true; break;
      case EMEPC_Green: bradd=0x40; maskpr=true; maskmg=true; maskpb=true; break;
      case EMEPC_Red:   bradd=0x40; maskmr=true; maskpg=true; maskpb=true; break;
    }

    const bool *psrc=pMaskDblImage;
    u32 *pdst=pCurrentImage;
    for(u32 y=0;y<ScreenHeight;y++){
      for(u32 x=0;x<ScreenWidth;x++){
        u32 br=0;
        if(psrc[0+(ScreenWidth*2*0)]==true) br+=bradd;
        if(psrc[1+(ScreenWidth*2*0)]==true) br+=bradd;
        if(psrc[0+(ScreenWidth*2*1)]==true) br+=bradd;
        if(psrc[1+(ScreenWidth*2*1)]==true) br+=bradd;
        psrc+=2;
        if(isEraseMode) {
            if(br!=0) *pdst=0;
        }else{
        if(br!=0){
          u32 col32=*pdst;
          s32 b=(col32>>0)&0xff;
          s32 g=(col32>>8)&0xff;
          s32 r=(col32>>16)&0xff;
          
          if(maskpr==true){
            r+=br;
            if(0xff<r) r=0xff;
          }
          if(maskpg==true){
            g+=br;
            if(0xff<g) g=0xff;
          }
          if(maskpb==true){
            b+=br;
            if(0xff<b) b=0xff;
          }
          
          br>>=1;
          if(maskmr==true){
            r-=br;
            if(r<0) r=0;
          }
          if(maskmg==true){
            g-=br;
            if(g<0) g=0;
          }
          if(maskmb==true){
            b-=br;
            if(b<0) b=0;
          }
          
          *pdst=(b<<0)|(g<<8)|(r<<16);
        }
      }
        pdst++;
      }
      psrc+=ScreenWidth*2;
    }
  }
  if(isEraseMode) Sound_Start(WAVFN_MemoErase);
  CurrentImageToScreen();
}

// -----------------------------------------------------

static void DrawOnlineHelp(void)
{
  if(Skin_OwnerDrawText.Custom_Top==true) return;
  
  CglB15 *pb15=MemoEdit_GetSkin(EME_BGTop);
  
  pb15->pCanvas->SetCglFont(pCglFontDefault);
  
  u32 x=8;
  u32 y=8;
  u32 h=glCanvasTextHeight+3;
  
  for(u32 idx=0;idx<10;idx++){
    const char *pmsg=NULL;
    switch(idx){
#define Prefix "ME_"
      case 0: pmsg=Lang_GetUTF8(Prefix "Help0"); break;
      case 1: pmsg=Lang_GetUTF8(Prefix "Help1"); break;
      case 2: pmsg=Lang_GetUTF8(Prefix "Help2"); break;
      case 3: pmsg=Lang_GetUTF8(Prefix "Help3"); break;
      case 4: pmsg=Lang_GetUTF8(Prefix "Help4"); break;
      case 5: pmsg=Lang_GetUTF8(Prefix "Help5"); break;
      case 6: pmsg=Lang_GetUTF8(Prefix "Help6"); break;
      case 7: pmsg=Lang_GetUTF8(Prefix "Help7"); break;
      case 8: pmsg=Lang_GetUTF8(Prefix "Help8"); break;
      case 9: pmsg=Lang_GetUTF8(Prefix "Help9"); break;
#undef Prefix
    }
    if(pmsg!=NULL){
      pb15->pCanvas->SetFontTextColor(ColorTable.MemoEdit.HelpText_Shadow);
      pb15->pCanvas->TextOutUTF8(x+1,y+1,pmsg);
      pb15->pCanvas->SetFontTextColor(ColorTable.MemoEdit.HelpText_Normal);
      pb15->pCanvas->TextOutUTF8(x+0,y+0,pmsg);
    }
    y+=h;
  }
}

static void DrawTopScreen(void)
{
  CglCanvas *pcan=new CglCanvas(&MM_Temp,NULL,ScreenWidth,ScreenHeight,pf15bit);
  pcan->SetCglFont(pCglFontDefault);
  
  {
    CglB15 *pb15=MemoEdit_GetSkin(EME_BGTop);
    pb15->pCanvas->BitBltFullBeta(pcan);
  }
  
  u32 fitop=ScreenHeight-32;
  
  {
    CglTGF *ptgf=MemoEditAlpha_GetSkin(EMEA_FileInfoBG);
    ptgf->BitBlt(pcan,0,fitop);
  }
  
  const char *pmsg0=NULL;
  UnicodeChar msg1[MaxFilenameLength];
  
  if(isNewFile==false){
    if(isEditedFile==false){
      pmsg0=Lang_GetUTF8("MemoEdit_FileInfo_Open");
      }else{
      pmsg0=Lang_GetUTF8("MemoEdit_FileInfo_Edited");
    }
    }else{
    pmsg0=Lang_GetUTF8("MemoEdit_FileInfo_Create");
  }
  
  if(isNewFile==false){
    Unicode_Copy(msg1,RelationalFileNameUnicode);
    }else{
    UnicodeChar *pfnw=GetNewFilenameUnicode();
    if(pfnw!=NULL){
      Unicode_Copy(msg1,pfnw);
      safefree(&MM_Temp,pfnw); pfnw=NULL;
    }
  }
  
  u32 x=8,y=fitop+5;
  u32 th=glCanvasTextHeight+1;
  
  pcan->SetFontTextColor(ColorTable.MemoEdit.FileInfo_Shadow);
  pcan->TextOutUTF8(x+1,y+(th*0)+1,pmsg0);
  pcan->TextOutW(x+1,y+(th*1)+1,msg1);
  pcan->SetFontTextColor(ColorTable.MemoEdit.FileInfo_Normal);
  pcan->TextOutUTF8(x+0,y+(th*0)+0,pmsg0);
  pcan->TextOutW(x+0,y+(th*1)+0,msg1);
  
  pcan->BitBltFullBeta(pScreenSub->pCanvas);
  
  if(pcan!=NULL){
    delete pcan; pcan=NULL;
  }
}

// -----------------------------------------------------

static void CB_KeyPress(u32 VsyncCount,u32 Keys)
{
  if(Keys!=0) isEraseMode=false;
  
  if((Keys&(KEY_DOWN|KEY_LEFT|KEY_UP|KEY_RIGHT))!=0){
    if(WaitKeyRelease) return;
    Sound_Start(WAVFN_Click);
    EMemoEditPenColor PenColor=EMEPC_Black;
    if((Keys&KEY_DOWN)!=0) PenColor=EMEPC_Black;
    if((Keys&KEY_LEFT)!=0) PenColor=EMEPC_Red;
    if((Keys&KEY_UP)!=0) PenColor=EMEPC_Green;
    if((Keys&KEY_RIGHT)!=0) PenColor=EMEPC_Blue;
    ProcState.MemoEdit.PenColor=PenColor;
    ProcState_RequestSave=true;
    ProcState_Save();
    WaitKeyRelease=true;
    return;
  }
    
  if((Keys&(KEY_X|KEY_Y))!=0){
    if(WaitKeyRelease) return;
    Sound_Start(WAVFN_Click);
    bool TickLine=false;
    if((Keys&KEY_X)!=0) TickLine=true;
    if((Keys&KEY_Y)!=0) TickLine=false;
    ProcState.MemoEdit.TickLine=TickLine;
    ProcState_RequestSave=true;
    ProcState_Save();
    WaitKeyRelease=true;
    return;
  }
  
  if((Keys&(KEY_SELECT))!=0){
    if(WaitKeyRelease) return;
    u32 *pTempImage=(u32*)safemalloc_chkmem(&MM_Temp,ScreenWidth*ScreenHeight*4);
    if(pTempImage!=NULL){
        MemCopy32CPU(pCurrentImage,pTempImage,ScreenWidth*ScreenHeight*4);
        MemCopy32CPU(pRevocationImage,pCurrentImage,ScreenWidth*ScreenHeight*4);
        MemCopy32CPU(pTempImage,pRevocationImage,ScreenWidth*ScreenHeight*4);
        safefree(&MM_Temp,pTempImage); pTempImage=NULL;
    }
    Sound_Start(WAVFN_MemoUndo);
    CurrentImageToScreen();
    WaitKeyRelease=true;
    return;
  }
  
  if((Keys&(KEY_L|KEY_R))!=0){
      isEraseMode=true;
      return;
  }
  
  if((Keys&(KEY_A|KEY_B))!=0){
    if(isEditedFile==true){
      if(CurrentImageSaveToFile()==false) isEditedFile=false;
    }
    RelationalFile_Clear();
    SetNextProc(ENP_FileList,EPFE_None);
    return;
  }
  
}

// -----------------------------------------------------

DATA_IN_IWRAM_MemoEdit static bool MouseDownFlag;

static void CB_MouseDown(s32 x,s32 y)
{
  Sound_Start(WAVFN_MemoPenDown);
  MouseDownFlag=true;
  
  if(FirstBGBottomDrawed==true){
    FirstBGBottomDrawed=false;
    CurrentImageToScreen();
  }
  
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  
  u16 pencol=RGB15(0,0,0)|BIT15;
  
  switch(ProcState.MemoEdit.PenColor){
    case EMEPC_Black: pencol=RGB15(0,0,0)|BIT15; break;
    case EMEPC_Blue:  pencol=RGB15(0,0,31)|BIT15; break;
    case EMEPC_Green: pencol=RGB15(0,31,0)|BIT15; break;
    case EMEPC_Red:   pencol=RGB15(31,0,0)|BIT15; break;
  }
  
  pcan->SetColor(pencol);
  pcan->MoveTo(x,y);
  
  CurLinesCount=0;
  
  TCurLine *pcl=&pCurLines[CurLinesCount];
  pcl->x=x;
  pcl->y=y;
  CurLinesCount++;
}

static void CB_MouseMove(s32 x,s32 y)
{
  if(MouseDownFlag==false) return;
  
  CglCanvas *pcan=pScreenMainOverlay->pCanvas;
  
  pcan->LineTo(x,y);
  
  if(CurLinesCount<CurLinesMaxCount){
    TCurLine *pcl=&pCurLines[CurLinesCount];
    pcl->x=x;
    pcl->y=y;
    CurLinesCount++;
  }
}

static void CB_MouseUp(s32 x,s32 y)
{
  if(MouseDownFlag==false) return;
  MouseDownFlag=false;
  
  DrawCurLines();
  CurLinesCount=0;
  
  isEditedFile=true;
}

// ------------------------------------------------------

static void CB_Start(void)
{
  MemInit();
  
  MouseDownFlag=false;
  
  DrawOnlineHelp();
  
  isNewFile=true;
  isEditedFile=false;
  
  isEraseMode=false;
//  StrConvert_Ank2Unicode("af.bmp",RelationalFileNameUnicode);
  
  Unicode_Copy(RelationalFilePathUnicode,Shell_GetMemoPathUnicode());
  
  const UnicodeChar *prelpath=RelationalFilePathUnicode;
  const UnicodeChar *prelname=RelationalFileNameUnicode;
  _consolePrintf("RelPath= %s.\n",StrConvert_Unicode2Ank_Test(prelpath));
  _consolePrintf("RelName= %s.\n",StrConvert_Unicode2Ank_Test(prelname));
  
  if((Unicode_isEmpty(prelpath)==false)&&(Unicode_isEmpty(prelname)==false)){
    if(FileExistsUnicode(prelpath,prelname)==true){
      if(CurrentImageLoadFromFile(prelpath,prelname)==true){
        isNewFile=false;
      }
    }
  }
  
  pScreenMainOverlay->pCanvas->FillFull(0);
  pScreenSub->pCanvas->FillFull(RGB15(31,31,31)|BIT15);
  pScreenMain->pBackCanvas->FillFull(RGB15(31,31,31)|BIT15);
  ScreenMain_Flip_ProcFadeEffect();
  
  DrawTopScreen();
  
  if(isNewFile==false){
    FirstBGBottomDrawed=false;
    CurrentImageToScreen();
    }else{
    FirstBGBottomDrawed=true;
    CglB15 *pb15=MemoEdit_GetSkin(EME_BGBottom);
    pb15->pCanvas->BitBltFullBeta(pScreenMainOverlay->pCanvas);
  }
}

static void CB_VsyncUpdate(u32 VsyncCount)
{
  MM_CheckOverRange();
  
  DATA_IN_IWRAM_MemoEdit static u32 a=0;
  a+=VsyncCount;
  
  if(MouseDownFlag==false){
    if(30<a){
      a=0;
      DrawTopScreen();
    }
  }
}

static void CB_End(void)
{
  MemFree();
}

// ------------------------------------------------------

static void CB_MWin_ProgressShow(const char *pTitleStr,s32 Max)
{
  msgwin_Draw(pTitleStr,"",0,0);
}

static void CB_MWin_ProgressSetPos(const char *pTitleStr,s32 pos, s32 max)
{
  char str[64];
  snprintf(str,64,"%d/%d %d%%",pos,max,pos*100/max);
  msgwin_Draw(pTitleStr,str,pos,max);
}

static void CB_MWin_ProgressDraw(const char *pstr0,const char *pstr1,s32 pos,s32 max)
{
  msgwin_Draw(pstr0,pstr1,pos,max);
}

static void CB_MWin_ProgressHide(void)
{
  msgwin_Clear();
}

// ------------------------------------------------------

void ProcMemoEdit_SetCallBack(TCallBack *pCallBack)
{
  pCallBack->Start=CB_Start;
  pCallBack->VsyncUpdate=CB_VsyncUpdate;
  pCallBack->End=CB_End;
  pCallBack->KeyPress=CB_KeyPress;
  pCallBack->MouseDown=CB_MouseDown;
  pCallBack->MouseMove=CB_MouseMove;
  pCallBack->MouseUp=CB_MouseUp;
  
  pCallBack->MWin_ProgressShow=CB_MWin_ProgressShow;
  pCallBack->MWin_ProgressSetPos=CB_MWin_ProgressSetPos;
  pCallBack->MWin_ProgressDraw=CB_MWin_ProgressDraw;
  pCallBack->MWin_ProgressHide=CB_MWin_ProgressHide;
}

